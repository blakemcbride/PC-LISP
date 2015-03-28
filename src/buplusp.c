

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buplusp:(plusp number) Return t if number is positive.              **
 *************************************************************************/
struct conscell *buplusp(form)
struct conscell *form;
{      struct conscell *temp;
       if ((form != NULL)&&(form->cdrp == NULL))
       {  if ((temp = form->carp) != NULL)
          {  if (temp->celltype == FIXATOM)
             {  if (FIX(temp)->atom > 0L)
                   return(LIST(thold));
                else
                   return(NULL);
             };
             if (temp->celltype == REALATOM)
             {  if (REAL(temp)->atom > 0.0)
                   return(LIST(thold));
                else
                   return(NULL);
             };
          };
       };
       ierror("plusp");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
