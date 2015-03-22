/* EDITION AB02, APFUN PAS.665 (90/11/23 09:34:20) -- CLOSED */                 
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bulist  : built in list function will take all of its arguments and **
 ** turn them into a list. This is really easy because we have already  **
 ** got a list of arguments mainly 'form'. We just return it.           **
 *************************************************************************/
struct conscell *bulist(form)
struct conscell *form;
{      return(topcopy(form));
}
