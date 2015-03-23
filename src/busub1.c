

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** busub1: (sub1 number)  subtract 1 from number and return result     **
 *************************************************************************/
struct conscell *busub1(form)
struct conscell *form;
{      struct conscell *temp;
       if ((form != NULL)&&(form->cdrp == NULL))
       {  if ((temp = form->carp) != NULL)
          {  if (temp->celltype == FIXATOM)
                return(newintop((long)FIX(temp)->atom - 1L));
             if (temp->celltype == REALATOM)
                return(newrealop(REAL(temp)->atom - 1.0));
          };
       };
       ierror("sub1");
}
