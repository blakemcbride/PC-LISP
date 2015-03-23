

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** burplacx: (rplacx index Hunk element) Replace Hunk[index] with the  **
 ** new 'element'. If index is outside the range of Hunk it is an error.**
 ** Returns the hunk Hunk but it will have been destructively altered.  **
 *************************************************************************/
struct conscell *burplacx(form)
struct conscell *form;
{      long int index; struct hunkcell *h; struct conscell **e;
       if (form != NULL)
       {    if (GetFix(form->carp,&index))
            {   form = form->cdrp;
                if ((form!=NULL)&&(form->carp!=NULL))
                {    h = HUNK(form->carp);
                     form = form->cdrp;
                     if (h->celltype == HUNKATOM)
                     {   if ((index >= 0L)&&(index < HUNK(h)->size))
                         {  e = GetHunkIndex(h,(int) index);
                            if ((form!=NULL)&&(form->cdrp==NULL))
                            {  *e = form->carp;
                                return(LIST(h));
                            };
                         };
                     };
                };
            };
       };
       ierror("rplacx");
}
