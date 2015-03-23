

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bufix: (fix number) Return a fix representation of number.          **
 *************************************************************************/
struct conscell *bufix(form)
struct conscell *form;
{      struct conscell *temp;
       if ((form != NULL)&&(form->cdrp == NULL))
       {  if ((temp = form->carp) != NULL)
          {  if (temp->celltype == FIXATOM)
                return(temp);
             if (temp->celltype == REALATOM)
                return(newintop((long)REAL(temp)->atom));
          };
       };
       ierror("fix");
}
