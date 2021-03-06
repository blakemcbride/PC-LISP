

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** buacos:Return the arc cosine of the single double parameter.        **
 *************************************************************************/
struct conscell *buacos(form)
struct conscell *form;
{      double f;
       if ((form != NULL)&&(form->cdrp == NULL))
          if (GetFloat(form->carp,&f))
              return(newrealop(acos(f)));
       ierror("acos");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
