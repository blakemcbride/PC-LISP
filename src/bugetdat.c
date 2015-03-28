

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (getdata array) Returns the hunk structure of array 'array'.        **
 *************************************************************************/
struct conscell *bugetdata(form)
struct conscell *form;
{      struct arraycell *temp;
       if ((form != NULL)&&(form->cdrp == NULL))
       {   if (ExtractArray(form->carp,&temp))
              return(LIST(temp->base));
       };
       ierror("getdata");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}

