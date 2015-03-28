

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bunth() Will return the nth element of a list. Just follow the cdr's**
 ** till I get to n. Then return the car of this cell.                  **
 *************************************************************************/
struct conscell *bunth(form)
struct conscell *form;
{      long int n;
       if (form!=NULL)
       {   if (form->carp != NULL)
           {    if (!GetFix(form->carp,&n)) goto ERR;
                form = form->cdrp;
                if (form->carp == NULL) return(NULL);
                if (form->carp->celltype == CONSCELL)
                {    if (form->cdrp == NULL)
                     {   form = form->carp;
                         if (n >= 0)
                         {   while(n-- > 0)
                             {     if (form != NULL)
                                       form = form->cdrp;
                                   else
                                       return(NULL);
                             };
                             if (form == NULL) return(NULL);
                             return(form->carp);
                         };
                     };
                };
           };
       };
ERR:   ierror("nth");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
