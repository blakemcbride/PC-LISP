/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:58) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** busetq: Set the value of a variable. We will see if it there is an  **
 ** n element on the valuestack. It there is we will alter the value of **
 ** this element stored in its car field. If the value stack is empty   **
 ** the reference will be made global by setting the botvaris field to  **
 ** be GLOBALVAR. Same as set except that first arg is not evaluated.   **
 *************************************************************************/
struct conscell *busetq(form)
struct conscell *form;
{      struct conscell *var,*val;
       push(val);
       while(form != NULL)
       {     var = form->carp;
             form = form->cdrp;
             if ((var != NULL)&&(var->celltype == ALPHAATOM)&&(form != NULL))
             {   val = eval(form->carp);
                 if (ALPHA(var)->valstack == NULL)
                 {   ALPHA(var)->botvaris = GLOBALVAR;
                    (ALPHA(var)->valstack = new(CONSCELL))->carp = val;
                 }
                 else
                     ALPHA(var)->valstack->carp = val;
             }
             else
                 ierror("setq");
             form = form->cdrp;
       };
       fret(val,1);
}
