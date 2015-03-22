/* EDITION AB02, APFUN MR.68 (90/04/18 09:24:00) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** busqrt:Returns the square root of the single double parameter.      **
 *************************************************************************/
struct conscell *busqrt(form)
struct conscell *form;
{      double f;
       if ((form != NULL)&&(form->cdrp == NULL))
          if (GetFloat(form->carp,&f)) {
              if (f < 0.0) goto er;
              return(newrealop(sqrt(f)));
          }
 er:   ierror("sqrt");
}
