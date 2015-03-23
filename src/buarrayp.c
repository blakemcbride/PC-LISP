

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (arrayp Sexp) Returns t if Sexp is an array, otherwise returns nil  **
 *************************************************************************/
struct conscell *buarrayp(form)
struct conscell *form;
{      struct arraycell *temp;
       if ((form != NULL)&&(form->cdrp == NULL))
       {   if (ExtractArray(form->carp,&temp))
              return(LIST(thold));
           return(NULL);
       };
       ierror("arrayp");
}
