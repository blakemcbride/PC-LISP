

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buasin:Returns the arc sine of the single double parameter.         **
 *************************************************************************/
struct conscell *buasin(form)
struct conscell *form;
{      double f;
       if ((form != NULL)&&(form->cdrp == NULL))
          if (GetFloat(form->carp,&f))
              return(newrealop(asin(f)));
       ierror("asin");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
