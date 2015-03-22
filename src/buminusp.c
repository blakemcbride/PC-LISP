/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:44) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buminusp:(minusp number) Return t if number is less than zero.      **
 *************************************************************************/
struct conscell *buminusp(form)
struct conscell *form;
{      struct conscell *temp;
       if ((form != NULL)&&(form->cdrp == NULL))
       {  if ((temp = form->carp) != NULL)
          {  if (temp->celltype == FIXATOM)
             {  if (FIX(temp)->atom < 0L)
                   return(LIST(thold));
                else
                   return(NULL);
             };
             if (temp->celltype == REALATOM)
             {  if (REAL(temp)->atom < 0.0)
                   return(LIST(thold));
                else
                   return(NULL);
             };
          };
       };
       ierror("minusp");
}
