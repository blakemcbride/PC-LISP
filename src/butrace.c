/* EDITION AB01, APFUN MR.68 (90/04/18 09:24:06) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** butrace: Set the trace bit on all passed atoms make sure that they  **
 ** are all user defined functions or built in functions. If no         **
 ** args are given just return the list of atoms that are currently set.**
 *************************************************************************/
struct  conscell *butrace(form)
struct  conscell *form;
{       struct alphacell *at; struct conscell *hold;
        if ((hold = form) == NULL) return(GetTraced());
        while(form != NULL)
        {     if (form->carp != NULL)
              {  at = ALPHA(form->carp);
                 if (at->celltype == ALPHAATOM)
                 {   at->tracebit = TRACE_ON;
                     form = form->cdrp;
                     continue;
                 };
              };
              ierror("trace");
        };
        return(hold);
}
