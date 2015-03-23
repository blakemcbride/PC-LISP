

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buoneplus: (1+ fixnum) Add number and 1 and return the result.      **
 *************************************************************************/
struct conscell *buoneplus(form)
struct conscell *form;
{      struct conscell *temp;
       if ((form != NULL)&&(form->cdrp == NULL))
          if ((temp = form->carp) != NULL)
             if (temp->celltype == FIXATOM)
                return(newintop((long)FIX(temp)->atom + 1L));
       ierror("1+");
}
