/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:48) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buoddp: (oddp fixnum) Return t if fixnum is an odd number.          **
 *************************************************************************/
struct conscell *buoddp(form)
struct conscell *form;
{      struct conscell *temp;
       if ((form != NULL)&&(form->cdrp == NULL))
       {  if ((temp = form->carp) != NULL)
          {  if (temp->celltype == FIXATOM)
             {  if (FIX(temp)->atom % 2)
                   return(LIST(thold));
                else
                   return(NULL);
             };
          };
       };
       ierror("oddp");
}
