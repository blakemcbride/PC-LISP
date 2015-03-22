/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:24) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** buboundp: Given an atom parameter it will determine if the atom is  **
 ** bound to anything. We can figure this out by simply checking the    **
 ** shallow binding stack for non null. We then must return(nil.value)  **
 ** as per the Franz manual.                                            **
 *************************************************************************/
struct conscell *buboundp(form)
struct conscell *form;
{      struct alphacell *r; struct conscell *t;
       if ((form != NULL)&&(form->cdrp == NULL))
       {   if (form->carp->celltype == ALPHAATOM)
           {   r = ALPHA(form->carp);
               if (r->valstack == NULL)
                   return(NULL);
               t = new(CONSCELL);
               t->cdrp = r->valstack->carp;
               return(t);
           };
       };
       ierror("boundp");
}
