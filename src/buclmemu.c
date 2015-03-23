/* */

/*
 | PC-LISP (C) 1991-1992 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include "lisp.h"

/*
 | Given an S-expression, compute the memory used in cons_bytes, alpha_bytes and heap bytes
 | and add to the given counters. This is an approximate calculation and does not count hunk or
 | array objects because they do not usually occur as clisp literals.
 */
static compute_memusage(l, cons_bytes, alpha_bytes, heap_bytes)
       struct conscell *l; long *cons_bytes, *alpha_bytes, *heap_bytes;
{
       if (l == NULL) return;
       switch(l->celltype) {
              case CONSCELL  : for( ; l && (l->celltype == CONSCELL); l = l->cdrp ) {
                                  *cons_bytes += sizeof(struct conscell);
                                  if ( l->carp ) compute_memusage(l->carp, cons_bytes, alpha_bytes, heap_bytes);
                               }
                               break;
              case ALPHAATOM : *alpha_bytes += sizeof(struct alphacell);
                               *heap_bytes  += strlen(ALPHA(l)->atom) + 1;
                               break;
              case STRINGATOM: *cons_bytes += sizeof(struct conscell);
                               *heap_bytes += strlen(STRING(l)->atom) + 1;
                               break;
              default :        *cons_bytes += sizeof(struct conscell);
                               break;
       }
}

/*
 | Top level LISP function (clisp-memusage clisp) returns the number of code, literal, cons, alpha
 | & heap bytes required by the clisp and all its used objects. This is called after compiling
 | a file so that the (memory-expand) call can be added to the head of the object file to speed
 | up loading by anticipating memory requirements.
 */
struct conscell *buclmemusage(form)
       struct conscell *form;
{
       register struct conscell **l;
       long cons_bytes, alpha_bytes, heap_bytes;
       register long nl, code_bytes, literal_bytes;
       register struct clispcell *clisp;
       register struct conscell *n;
       struct conscell c1, *r;

      /*
       | Validate the input parameters expect a single clisp argument with an optional
       | port.
       */
       if (form == NULL) goto er;
       clisp = CLISP(form->carp);
       form = form->cdrp;
       if (form || !clisp || (clisp->celltype != CLISPCELL)) goto er;

      /*
       | The number of literal bytes is stored one word before the head of the literal list.
       */
       literal_bytes = *((int *)clisp->literal - 1);
       code_bytes    = *((int *)clisp->code - 1);

      /*
       | Zero out the running byte counters which we will accumulate as we go through the
       | literals list.
       */
       cons_bytes = alpha_bytes = heap_bytes = 0;

      /*
       | Loop through all literals and for each compute how much space it occupies in the 4 areas.
       */
       c1.celltype = CONSCELL; c1.cdrp = NULL;
       nl = literal_bytes / sizeof(struct conscell *) - 1;
       for(l  = clisp->literal + 1; nl-- > 0; l++ ) {
           if (!(*l)) continue;
           if ((*l)->celltype != CLISPCELL) {
               compute_memusage(*l, &cons_bytes, &alpha_bytes, &heap_bytes);
           } else {
               c1.carp = *l;
               r = buclmemusage(&c1);
               code_bytes    += FIX(r->carp)->atom; r = r->cdrp;
               literal_bytes += FIX(r->carp)->atom; r = r->cdrp;
               cons_bytes    += FIX(r->carp)->atom; r = r->cdrp;
               alpha_bytes   += FIX(r->carp)->atom; r = r->cdrp;
               heap_bytes    += FIX(r->carp)->atom; r = r->cdrp; if (r) goto er;
           }
       }

      /*
       | Add in the counts for the headers stored at the front of the malloced areas.
       */
       literal_bytes += sizeof(struct conscell *);
       code_bytes    += sizeof(int);

      /*
       | Construct the return parameter of (code_bytes literal_bytes cons_bytes alpha_bytes heap_bytes)
       */
       push(r);
       r = new(CONSCELL); r->carp = newintop(heap_bytes); r->cdrp = NULL;
       n = new(CONSCELL); n->cdrp = r; r = n; r->carp = newintop(alpha_bytes);
       n = new(CONSCELL); n->cdrp = r; r = n; r->carp = newintop(cons_bytes);
       n = new(CONSCELL); n->cdrp = r; r = n; r->carp = newintop(literal_bytes);
       n = new(CONSCELL); n->cdrp = r; r = n; r->carp = newintop(code_bytes);
       xpop(1);
       return(r);

      /*
       | Throw usual error.
       */
er:    ierror("clisp-memusage");
}
