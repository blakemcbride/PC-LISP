

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (copy exp) returns a new object (equal) to exp with new cons cells  **
 *************************************************************************/
struct conscell *bucopy(form)
struct conscell *form;
{      if ((form != NULL)&&(form->cdrp == NULL))
          return(copy(form->carp));
       ierror("copy");
}
