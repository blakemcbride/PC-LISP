/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:28) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** bufact:Returns the factorial of the single double parameter.        **
 *************************************************************************/
struct conscell *bufact(form)
struct conscell *form;
{      long int val = 1;long int pump;
       if ((form != NULL)&&(form->cdrp == NULL))
          if (GetFix(form->carp,&pump))
          {   while(pump > 1)
              {     val *= pump;
                    pump--;
              };
              return(newintop(val));
          };
       ierror("fact");
}
