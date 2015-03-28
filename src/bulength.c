

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** bulength() : Will count the length of the list at the top level and **
 ** return a new real cell containing this value.                       **
 *************************************************************************/
struct conscell *bulength(form)
struct conscell *form;
{
       if (form!=NULL)
       {   if (form->carp == NULL)
               return(newintop(0L));
           if (form->carp->celltype == CONSCELL)
               if (form->cdrp == NULL)
                   return(newintop((long)liulength(form->carp)));
       }
       ierror("length");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
