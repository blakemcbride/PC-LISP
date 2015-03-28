#include        <stdio.h>
#include        <ctype.h>
#include        "lisp.h"

/*
 | (repeat <expr1> <expr2> <expr3> ..... <exprN>)
 |
 | This primitive will evaluate <expr1> which should be a positive fixnum. It
 | evaluate the list <expr2> .. <exprN> that many times. Ie, it repeats the
 | evaluation N times. It handles labels and jumps/returns in the same manner
 | as prog/while/foreach etc.
 |
 |     (repeat 14
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
struct conscell *burepeat(form)
struct conscell *form;
{      struct conscell *count, *pp, *lastval;
       int has_labels = 0;
       long n;

      /*
       | Basic sanity. Make sure we have some arguments, extract the first
       | which is the count expression, evaluate the count and extract the
       | resulting fixnum. If none throw an error. If the range is negative
       | or zero then get out right now and return NULL. If range is positive
       | then extract the head of the list of bodies to be repeated and mark
       | the lastval to be returned. Also, if there is no body to repeat we
       | just exit immediately without doing anything (ie a very fast loop!).
       */
       if (form == NULL) goto er;
       count = eval(form->carp);
       if (!GetFix(count, &n)) goto er;
       form = form->cdrp;
       if ((n <= 0) || (form == NULL)) return(NULL);
       push(lastval);

       lillev += 1;                                  /* increment lexical level */

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
       | Run the counted loop. We simply run the program pointer through the
       | bodies one by one. If we ever run off the end of the list of bodies
       | we decrement the loop counter 'N' and if negative or zero we exit
       | and return the last evaluated body. If non zero we reset the program
       | pointer 'pp' to the top of the loop and keep going. Note how we check
       | for an expression of the form (go ...) or (return ..) and use cadr of
       | them appropriately to do a jump or a return.
       */
       for(pp = form;;) {
           if (pp == NULL) {
               if (--n <= 0) goto end;
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

      /*
       | All done, return 'lastval' and pop the pointer to it from the mark
       | stack for garbage collection. Decrement the lexical level first.
       */
       lillev -= 1;
       fret(lastval,1);

      /*
       | Problem with the arguments, so throw the error 'while'.
       */
er:    ierror("repeat");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
