/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:40) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bumakunb: Given an atom parameter this routine will cause the global**
 ** binding to be removed. This means that the atom can be a candidate  **
 ** for garbage collection if its value and function pointers are NIL.  **
 ** If the atom has no global binding this function return NIL, else it **
 ** unlinks the global value and returns T.                             **
 *************************************************************************/
struct conscell *bumakunb(form)
struct conscell *form;
{      struct alphacell *r; struct conscell *t, **l;
       if ((form != NULL)&&(form->cdrp == NULL))
       {   if (form->carp->celltype == ALPHAATOM)
           {   r = ALPHA(form->carp);
               l = &(r->valstack);
               if ((*l != NULL) && (r->botvaris == GLOBALVAR)) {
                  for(t = *l; t->cdrp != NULL; t = *(l = &(t->cdrp)));
                  *l = NULL;
                  r->botvaris = LOCALVAR;
                  return(LIST(thold));
               }
               return(NULL);
           }
       }
       ierror("makunbound");
}
