/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:22) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buadd1: (add1 number)  Add number and 1 and return the result.      **
 *************************************************************************/
struct conscell *buadd1(form)
struct conscell *form;
{      struct conscell *temp;
       if ((form != NULL)&&(form->cdrp == NULL))
       {  if ((temp = form->carp) != NULL)
          {  if (temp->celltype == FIXATOM)
                return(newintop((long)FIX(temp)->atom + 1L));
             if (temp->celltype == REALATOM)
                return(newrealop(REAL(temp)->atom + 1.0));
          };
       };
       ierror("add1");
}
