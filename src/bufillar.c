

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (fillarray array list) Will fill the array 'array' using elements   **
 ** from the list 'list'. If the array is bigger than the list then the **
 ** last element in the list is used repeatedly to fill the rest of the **
 ** elements in the array.                                              **
 *************************************************************************/
struct conscell *bufillarray(form)
struct conscell *form;
{      struct conscell **e,*n,*l;
       struct arraycell *array;
       long int size,i;
       if (form != NULL)
       {   if (ExtractArray(form->carp,&array))
           {  form = form->cdrp;
              if ((form == NULL)||(form->cdrp != NULL)) goto ERR;
              n = form->carp;
              l = NULL;
              size = FIX(array->info->carp)->atom;
              for(i=0L;i<size;i++)
              {    e = GetArrayIndex(array->base,i,size);
                   if (n != NULL)
                   {  if (n->celltype != CONSCELL) goto ERR;
                      l = n->carp;
                      n = n->cdrp;
                   };
                  *e = l;
              };
              return(NULL);
           };
       };
  ERR: ierror("fillarray");
}
