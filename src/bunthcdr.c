

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bunthcdr: (nthcdr number list) Perform cdr 'number' times down the  **
 ** list parameter remembering that (cdr nil) is nil. If number is < 0  **
 ** we return (cons nil list) {Don't ask me why but Franz does this? }  **
 *************************************************************************/
struct conscell *bunthcdr(form)
struct conscell *form;
{      long int n; struct conscell *temp;
       if (form != NULL)
       {   if (GetFix(form->carp,&n))
           {   form = form->cdrp;
               if (form->cdrp == NULL)
               {   form = form->carp;
                   if ((form==NULL)||(form->celltype==CONSCELL))
                   {  if (n < 0L)
                      {  temp = new(CONSCELL);
                         temp->cdrp = form;
                         return(temp);
                      };
                      if (form == NULL) return(NULL);
                      for(;;form = form->cdrp,n--)
                      {   if (n == 0L)
                             return(form);
                          if ((form->celltype!=CONSCELL)||(form->cdrp==NULL))
                             return(NULL);
                      };
                   };
               };
           };
       };
       ierror("nthcdr");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
