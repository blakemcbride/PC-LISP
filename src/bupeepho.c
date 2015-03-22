/* EDITION AA02, APFUN PAS.801 (92/03/25 15:48:44) -- CLOSED */                 
/* --- */

/*
 | PC-LISP (C) 1990-1992 Peter J.Ashwood-Smith
 | ---------------------------------------------
 */
#include <stdio.h>
#include "lisp.h"

/*
 | Top level LISP function (ph-optimize S-expr) expects an expression output from
 | (compile <expr>) which it will then traverse and destrictively modify certain
 | instructions to become nil's which the assembler will ignore hence removing the
 | code completely.
 |
 | The following peep hole type reductions are performed to remove redundant code phrases.
 |
 | TYPE#1    PUSHT   o POP                    => nil nil       { get this after declare/defun }
 | TYPE#2    PUSHNIL o POP                    => nil nil       { also after declare/defun }
 | TYPE#3    PUSHLV  o POP                    => nil nil
 | TYPE#4    PUSHL   o POP                    => nil nil
 | TYPE#5    SETQ <N> o POP o PUSHLV <N>      => SETQ <n>      { (prog() (setq x 10) (fred x) .. }
 */
struct conscell *buphoptimize(form)
       struct conscell *form;
{
       register struct conscell *clisp, *literals, *instructions,*l;
       register int n_1234, n_5;

#      define EQ(at, str) (strcmp(at->atom, str) == 0)                    /* a useful macro */

      /*
       | Initialize statistics gathering code, we print out results later.
       */
       n_1234 = n_5 = 0;

      /*
       | Initially extract the liberals and instructions lists from the passed $$clisp$$ structure..
       */
       if ((form == NULL)||(form->cdrp != NULL)) goto er;
       clisp = form->carp;
       if (!(form = clisp)) goto er;
       if (form->celltype == CLISPCELL) return(form);                   /* already assembled? nothing to do */
       if (form->celltype != CONSCELL) goto er;
       if (!form->carp || form->carp->celltype != ALPHAATOM) goto er;
       if (strcmp(ALPHA(form->carp)->atom, "$$clisp$$") != 0) goto er;
       if (!(form = form->cdrp)) goto er;
       if (form->carp) goto er;                                          /* cannot optimize if errors */
       if (!(form = form->cdrp)) goto er;
       literals = form->carp;
       form = form->cdrp;
       if (!form) goto er;
       instructions = form->carp;
       if (form->cdrp) goto er;
       if (!instructions) goto ok;
       if (instructions->celltype != CONSCELL) goto er;
       if (literals && (literals->celltype != CONSCELL)) goto er;

      /*
       | First recursively peep-hole optimize any literal $$clisp$$'s used by this $$clisp$$ and add
       | their peep hole optimization counts to our current ones.
       */
       for( ; literals != NULL; literals = literals->cdrp) {
            register struct conscell *litpair, *lit, *first;
            TEST_BREAK();
            if (!(litpair = literals->carp)) goto er;
            if ((lit = litpair->carp) && (lit->celltype == CONSCELL)) {
                if ((first = lit->carp) && (first->celltype == ALPHAATOM)) {
                    if (strcmp(ALPHA(first)->atom, "$$clisp$$") == 0) {
                        struct conscell c1,*res;
                        c1.celltype = CONSCELL;
                        c1.cdrp = NULL;
                        c1.carp = lit;
                        res = buphoptimize(&c1);
                        if (res == NULL) return(NULL);
                        n_1234 += FIX(res->cdrp->carp)->atom;
                        n_5    += FIX(res->cdrp->cdrp->carp)->atom;
                        litpair->carp = res->carp;
                    }
                }
            }
       }

      /*
       | PASS#1 remove redundant PUSH/POP's and reduce SETQ/POP/PUSHLV sequences. Also accumulate information for
       | the second pass.
       */
       for(l = instructions; l != NULL; l = l->cdrp) {
            register struct conscell *inst; register struct alphacell *op;
            TEST_BREAK();
            inst = l->carp;
            if ((inst == NULL) || (inst->celltype != CONSCELL)) continue;
            op = ALPHA(inst->carp);
            if ((op == NULL) || (op->celltype != ALPHAATOM)) continue;

           /*
            | Reduction numbers 1,2,3 & 4. If get a push, remember by setting l1 to it. If get a pop and l1 is
            | set, immediately reduce both instructions to NULL.
            */
            if ( EQ(op,"PUSHT") || EQ(op,"PUSHNIL") || EQ(op,"PUSHLV") || EQ(op,"PUSHL") ) {
                 register struct conscell *n = l->cdrp;
                 if (n && (n->celltype == CONSCELL) && (n = n->carp))
                     if ((n->celltype == CONSCELL) && n->carp && (n->carp->celltype == ALPHAATOM))
                         if EQ( ALPHA(n->carp), "POP" ) {
                            l->carp = l->cdrp->carp = NULL;
                            n_1234 += 1;
                         }
            }

           /*
            | Reduction number 4. Replace SETQ <lit> o POP o PUSHL <lit> with SETQ <lit> o nil o nil
            */
            if ( EQ(op, "SETQ") ) {
                 register struct conscell *n, *m = l->cdrp;
                 if (m && (m->celltype == CONSCELL) && (n = m->carp))
                     if ((n->celltype == CONSCELL) && n->carp && (n->carp->celltype == ALPHAATOM))
                        if EQ( ALPHA(n->carp), "POP" ) {
                           register struct conscell *k, *s = m->cdrp;
                           if (s && (s->celltype == CONSCELL) && (k = s->carp))
                              if ((k->celltype == CONSCELL) && k->carp && (k->carp->celltype == ALPHAATOM))
                                  if EQ( ALPHA(k->carp), "PUSHLV" )
                                     if (equal(k->cdrp, inst->cdrp)) {   /* same literal? */
                                         m->carp = s->carp = NULL;
                                         n_5 += 1;
                                     }
                        }
            }
       }

ok:
      /*
       | All done, return a list containing the $$clisp$$ and a count of the number of each optimization
       | type performed.  ( ($$clisp$$) 10 5 .. )
       */
       { struct conscell *a, *b = new(CONSCELL);
         xpush(b);
         b->carp = clisp;
         a = b->cdrp = new(CONSCELL);
         a->carp = newintop((long) n_1234);
         a = a->cdrp = new(CONSCELL);
         a->carp = newintop((long) n_5);
         xpop(1);
         return(b);
       }

      /*
       | Throw error assembling instructions.
       */
er:    ierror("ph-optimize");
}
