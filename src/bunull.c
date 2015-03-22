/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:46) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** bunull : the built in null and not functions. They must if          **
 ** given a null list as its single paramter return the 't' atom. If not**
 ** it will return the 'nil' atom. NOTE that in lisp the atom 'nil' is  **
 ** equivalent to the NULL list, so we take that into consideration in  **
 ** the testing of this predicate: EG (null nil) is 't'.                **
 *************************************************************************/
struct conscell *bunull(form)
struct conscell *form;
{      if ((form != NULL)&&(form->cdrp == NULL))
       {  if (form->carp == NULL)
               return((struct conscell *)thold);
          else
               return(NULL);
       };
       ierror("null");
}
