/* EDITION AD02, APFUN PAS.821 (92/04/23 15:06:42) -- CLOSED */                 
#include        <stdio.h>
#include        <ctype.h>
#include        "lisp.h"

/*
 | (while <expr1> <expr2> <expr3> ..... <exprN>)
 |
 | This primitive will evaluate <expr1> and if the result is non nil will then
 | evaluate each of expr2 .. exprN as per (prog). It then loops back and tests
 | the condition <expr1> again etc. until the expression is nil. The result is
 | the value of the last evaluation of <exprN>. For Example:
 |
 |     (while (< i n)
 |       l:   (setq i (+1 i))
 |              :
 |            (cond ... (go l)
 |              :
 |              :       (return '(xyz))
 |            )
 |     )
 |
 | IMPORTANT: this built in function alters bindings so the throw/catch and
 | errset mechanisms must know about it. The file throw.c knows how to undo
 | the bindings introduced by this code if a 'throw' or 'errset' is called
 | below or above us respectively.
 */
struct conscell *buwhile(form)
struct conscell *form;
{      register struct conscell *cond, *pp;
       struct conscell *lastval;
       int has_labels = 0;

      /*
       | Basic sanity. Make sure we have some arguments, extract the first
       | which is the condition expression, and push a mark pointer to the
       | last value pointer. The last value pointer holds a pointer to the
       | last evaluated S-expression in the body. Pushing it on the mark
       | stack also sets it to NULL.
       */
       if (form == NULL) goto er;
       cond = form->carp;
       form = form->cdrp;
       if (cond == NULL) return(NULL);

      /*
       | Handle the special case (while (condition)), just keep evaluating the
       | condition as long as it is non NULL. then exit and return NULL.
       */
       if (form == NULL) {
           while( eval(cond) != NULL );
           return(NULL);
       }

       lillev += 1;                          /* increment lexical level */

      /*
       | It has a condition and at least one body so away we go with the more complex
       | form.
       */
       push(lastval);

      /*
       | Loop through the bodies list looking for labels which are just atoms.
       | If  we  find  one,  bind  the label to the next position in the list.
       | If  any dotted pairs are  encountered then just abort and jump to the
       | label unbinding code which will do the same operation only in reverse.
       */
       for(pp = form; pp != NULL; pp = pp->cdrp) {
           if (pp->celltype != CONSCELL) goto end;
           if ((pp->carp != NULL)&&(pp->carp->celltype == ALPHAATOM)) {
                bindlabel(pp->carp,pp->cdrp);
                has_labels = 1;
           }
       }

      /*
       | Run the while loop. We simply run the program pointer through the
       | bodies one by one. Before each we evaluate the condition expression
       | and if it evaluates to non NIL we keep going otherwise we stop the
       | loop. If during the loop we get to the end of the list of bodies
       | we reset the program pointer to start at the beginning again. Note
       | how we check for an expression of the form (go ...) or (return ..)
       | and use cadr of them appropriately to do a jump or a return.
       */
       for(pp = NULL; ;) {
           if (pp == NULL) {
               if (eval(cond) == NULL) goto end;
               pp = form;
           }
           if (pp->celltype != CONSCELL) goto er;
           if ((pp->carp != NULL)&&(pp->carp->celltype != ALPHAATOM)) {
               lastval = eval(pp->carp);
               if ((lastval != NULL)&&(lastval->celltype == CONSCELL)) {
                  if (lastval->carp == LIST(returnhold)) {
                     lastval = lastval->cdrp;
                     goto end;
                  }
                  if (lastval->carp == LIST(gohold)) {
                     pp = lastval->cdrp;
                     continue;
                  }
               }
           }
           pp = pp->cdrp;
       }

      /*
       | We are done looping around the while loop. Either the loop condition
       | was not met, a (return [<expr>]) was executed, or something was wrong
       | with the list of bodies (ie a dotted pair). So, we loop through the
       | bodies, unbinding the labels.
       */
end:   if (has_labels) {
          for(pp = form; pp != NULL; pp = pp->cdrp) {
              if (pp->celltype != CONSCELL) goto er;
              if ((pp->carp != NULL)&&(pp->carp->celltype == ALPHAATOM))
                   unbindvar(pp->carp);
          }
       }
       lillev -= 1;       /* decrement lexical level */

      /*
       | All done, return 'lastval' and pop the pointer to it from the mark
       | stack for garbage collection.
       */
       fret(lastval,1);

      /*
       | Problem with the arguments, so throw the error 'while'.
       */
er:    ierror("while");
}
