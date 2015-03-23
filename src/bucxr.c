

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bucxr: (cxr index Hunk) will return the element number index in the **
 ** hunk Hunk. If index is outside the size of Hunk it is an error. We  **
 ** index from 0. But not the h->size field is the absolute size of h.  **
 *************************************************************************/
struct conscell *bucxr(form)
struct conscell *form;
{      long int index; struct hunkcell *h;
       if (form != NULL)
       {    if (GetFix(form->carp,&index))
            {   form = form->cdrp;
                if ((form!=NULL)&&(form->carp!=NULL)&&(form->cdrp==NULL))
                {    h = HUNK(form->carp);
                     if (h->celltype == HUNKATOM)
                     {   if ((index >= 0L)&&(index < HUNK(h)->size))
                            return(*GetHunkIndex(h,(int) index));
                     };
                };
            };
       };
       ierror("cxr");
}
