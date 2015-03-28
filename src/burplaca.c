

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (rplaca list exp) Will actually alter the car pointer of list to exp**
 ** and then return list which will have been destructively altered.    **
 *************************************************************************/
struct conscell *burplaca(form)
struct conscell *form;
{      struct conscell *list,*exp;
       if ((form!=NULL)&&(form->cdrp != NULL))
       {   list = form->carp;
           form = form->cdrp;
           if ((form!=NULL)&&(form->cdrp == NULL))
           {   exp = form->carp;
               if ((list != NULL)&&(list->celltype == CONSCELL))
               {    list->carp = exp;
                    return(list);
               };
           };
       };
       ierror("rplaca");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
