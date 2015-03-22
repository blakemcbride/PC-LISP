/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:58) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** buset : Set the value of a variable. We will see if it there is an  **
 ** n element on the valuestack. It there is we will alter the value of **
 ** this element stored in its car field. If the value stack is empty   **
 ** the reference will be made global by setting the botvaris field to  **
 ** be GLOBALVAR.                                                       **
 *************************************************************************/
struct conscell *buset(form)
struct conscell *form;
{      struct conscell *var,*val;
       push(val);
       if ((form != NULL)&&(form->carp->celltype == ALPHAATOM))
       {    var = form->carp;
            form = form->cdrp;
            if ((form != NULL)&&(form->cdrp == NULL))
            {    val = form->carp;
                 if (((struct alphacell *)var)->valstack == NULL)
                 {   ((struct alphacell *)var)->botvaris = GLOBALVAR;
                     ((struct alphacell *)var)->valstack = new(CONSCELL);
                 };
                ((struct alphacell *)var)->valstack->carp = val;
                fret(val,1);
            };
       };
       ierror("set");
}
