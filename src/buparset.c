/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:50) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buparsetq: Like setq but this is a parallel assignment. The value of**
 ** a previous assignment will not effect the rest of the assignments.  **
 ** The results are uneffected by the order of the atoms and values in  **
 ** the expression unlike setq which does its work left to right and    **
 ** can be effected by side effects. This function is used by the (do)  **
 ** Macro. I added because it is faster to do it it C than in LISP.     **
 ** This operates by first building a list of the evaluated values to be**
 ** assigned. These are then pushed destructively onto the valstacks of **
 ** the atoms in the PAR-setq parameter list.                           **
 *************************************************************************/
struct conscell *buparsetq(form)
struct conscell *form;
{      struct conscell *save,*retval,*temp,*vlist,*last;
       push(vlist); push(temp);
       save = form; retval = NULL;
       while(form != NULL)
       {     form = form->cdrp;
             if (form == NULL) goto ERR;
             {   temp = new(CONSCELL);
                 temp->carp = eval(form->carp);
                 if (vlist == NULL)
                     last = vlist = temp;
                 else
                 {   last->cdrp = temp;
                     last = temp;
                 };
             };
             form = form->cdrp;
       };
       while(save != NULL)
       {     if ((temp = save->carp) == NULL) goto ERR;
             if (temp->celltype != ALPHAATOM) goto ERR;
             if (ALPHA(temp)->valstack == NULL)
             {   ALPHA(temp)->botvaris = GLOBALVAR;
                 retval=(last=ALPHA(temp)->valstack=vlist)->carp;
                 vlist = vlist->cdrp;
                 last->cdrp = NULL;
             }
             else
             {   retval = (ALPHA(temp)->valstack->carp = vlist->carp);
                 vlist = vlist->cdrp;
             };
             save = save->cdrp->cdrp;
       };
       xpop(2);
       return(retval);
  ERR: ierror("PAR-setq");
}
