

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (getdata array) Returns the hunk structure of array 'array'.        **
 *************************************************************************/
struct conscell * bugetdata(struct conscell *form)
{      struct conscell *temp;
       if ((form != NULL)&&(form->cdrp == NULL))
       {   if (ExtractArray(form->carp,&temp))
              return(LIST(ARRAY(temp)->base));
       };
       ierror("getdata");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}

