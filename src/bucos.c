/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:26) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bucos :Returns the cosine  of the single double parameter.          **
 *************************************************************************/
struct conscell *bucos(form)
struct conscell *form;
{      double f;
       if ((form != NULL)&&(form->cdrp == NULL))
          if (GetFloat(form->carp,&f))
              return(newrealop(cos(f)));
       ierror("cos");
}
