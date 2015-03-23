

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (listarray array [number]) takes 'number' elements from the array   **
 ** and returns as a list. Returns a list of entire array if no number. **
 *************************************************************************/
struct conscell *bulistarray(form)
struct conscell *form;
{      struct conscell *n,*l;
       struct arraycell *array;
       long int size,number;
       if (form != NULL)
       {   if (ExtractArray(form->carp,&array))
           {  form = form->cdrp;
              number = size = FIX(array->info->carp)->atom;
              if (form != NULL)
              {   if ((form->cdrp != NULL)||(!GetFix(form->carp,&number)))
                      goto ERR;
              };
              push(l);
              l = n = NULL;
              while(number--)
              {    n = new(CONSCELL);
                   n->carp = *GetArrayIndex(array->base,number,size);
                   n->cdrp = l;
                   l = n;
              };
              xpop(1);
              return(l);
           };
       };
  ERR: ierror("listarray");
}
