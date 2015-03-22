/* EDITION AC01, APFUN PAS.751 (91/09/05 17:41:16) -- CLOSED */                 
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bucond: The lisp 'cond' function. It takes a list of statements,    **
 ** each statement consists of a pair of expressions. The first express **
 ** ion , called the guard, is evaluated, the first guard to evaluate non*
 ** nil causes the value of the cond expression to be the result of eval**
 ** int the corresponding expression. We evaluate the list of expressions*
 ** associated with the none nil guard and return the last evaluation.  **
 ** Note that if a ($..return..$ <expr>) is returned by the evaluation  **
 ** of any expression in the statments then we stop the statments evalu-**
 ** ation and return the return expression which will cause the the prog**
 ** etc. running above to process it.                                   **
 *************************************************************************/
struct conscell *bucond(form)
struct conscell *form;
{      register struct conscell *w, *s, *r, *t;
       xpush(form);
       while (form != NULL) {
          if (((w = form->carp) != NULL) && (( s = eval(w->carp)) != NULL)) {
              if ((w = w->cdrp) == NULL)
                 fret(s,1);                             /* no exp? return guard */
              do {
                   if ((r = eval(w->carp)) && (r->celltype == CONSCELL))
                      if (((t = r->carp) == LIST(returnhold)) || (t == LIST(gohold)))
                         break;
                   w = w->cdrp;
              } while(w != NULL);
              fret(r,1);                                /* return last expression evaled */
          }
          form = form->cdrp;
       }
       fret(NULL, 1);                                   /* no true guards */
}
