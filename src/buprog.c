
/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/**************************************************************************
 ** LISP progs: A simple prog example is as follows:                     **
 **                                                                      **
 **                                                                      **
 **                                                                      **
 **      (prog  (n)                                                      **
 **                 (set  'n  1)                                         **
 **          top    (set  'n  (+ n n))                                   **
 **                 (cond ( (= n 512) (return  'finished ))              **
 **                       ( t (go top) )                                 **
 **      ]                                                               **
 **                                                                      **
 **                                                                      **
 **                                                                      **
 ** Where the prog has a local variable 'n'. If we execute this prog we  **
 ** would get the silly result 'finished' after a second of run time.    **
 ** the LISP prog function allows sequential exection of statements.     **
 ** PROGS are closely related to the GO and RETURN functions. These two  **
 ** functions although available from outside of a PROG have little use. **
 ** Return and Go simply evaluate their argument and return the list     **
 ** (go ..) or (return .. ) accordingly. The prog function on the next   **
 ** page will spot these special lists and act accordingly. Set is also  **
 ** a function with very little meaning outside of a prog. Set can be    **
 ** used to alter the value of a bound variable. The side effect of the  **
 ** call is this changed value. The value of SET is also this new value. **
 ** Labels are considered to be bound to the lists that follow them. So  **
 ** changing a label with a set command would have a disasterous effect. **
 ** By making labels part of the alist it is possible to jump out of a   **
 ** prog to an outer prog, but it is not possible to jump from the outer **
 ** into the inner. Thus we have a scope rule. The same is true of access**
 ** to the local 'prog' variables, they are only bound withing the prog  **
 ** function itself, much like the lambda expression bound variables.    **
 **************************************************************************/


/*************************************************************************
 ** buprog : takes a LISP prog and evaluates it. First we assocaite all **
 ** the local parameters with nil by a call to pairlis. Then we associate*
 ** all the labels with the point in the list that follows. We must scan**
 ** the prog once to do this. Finally we set the program pointer 'pp' to**
 ** the head of the program and evaluate the statements on by one. If   **
 ** we get (go ...) we change the program pointer accordingly. If we get**
 ** (return ....) we stop and return the appropriate value. If we ever  **
 ** hit a null statement we give an error message.                      **
 *************************************************************************/
struct conscell *buprog(form)
struct conscell *form;
{      register struct conscell *value,*pp,*locals,*labels;
       int has_labels = 0;
       lillev += 1;                                           /* increment lexical level */
       xpush(form);
       value = locals = labels = NULL;
       if (form != NULL) {
           bindtonil(locals = form->carp);                    /* bind locals*/
           form = form->cdrp;
           labels = pp = value = form;
           while (value != NULL) {                            /* bind labels*/
              if ((value->carp != NULL) && (value->carp->celltype == ALPHAATOM)) {    /* point in */
                  bindlabel(value->carp,value->cdrp);         /* prog */
                  has_labels = 1;
              }
              value = value->cdrp;
           }
           value = NULL;
           for (;;) {                                         /* run a prog */
              if (pp != NULL) {
                  if ((pp->carp != NULL)&&(pp->carp->celltype == ALPHAATOM))
                      pp = pp->cdrp;                          /* skip a label */
                  else {
                      value = eval(pp->carp);
                      if (value != NULL) {
                          if (value->carp == (struct conscell *)returnhold) {
                              value = value->cdrp;
                              goto done;
                          } else
                              if (value->carp == (struct conscell *)gohold)
                                  pp = value->cdrp;
                              else
                                  pp = pp->cdrp;              /* do next SEX*/
                      } else                                  /* null return*/
                          pp = pp->cdrp;                      /* do next list*/
                  }
              } else                                          /* ran off end */
                  goto done;                                  /* return value */
           }
       }
       ierror("prog");
done:  popvariables(locals);                                  /* done, so pop locals and labels if any */
       if (has_labels) {
          while (labels != NULL) {
             if ((labels->carp != NULL) && (labels->carp->celltype == ALPHAATOM))
                 unbindvar(labels->carp);
             labels = labels->cdrp;
          }
       }
       lillev -= 1;                                           /* decrement lexical level */
       fret(value,1);
}
