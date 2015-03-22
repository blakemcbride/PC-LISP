/* EDITION AB01, APFUN MR.68 (90/04/18 09:24:08) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** buuntrace: unset the trace bit on all passed atoms. If no args are  **
 ** given just called GetTraced to get a list of all traced atoms and   **
 ** unset each of these.                                                **
 *************************************************************************/
struct  conscell *buuntrace(form)
struct  conscell *form;
{       struct alphacell *at; struct conscell *hold;
        if (form == NULL) form = GetTraced();
        hold = form;
        while(form != NULL)
        {     if (form->carp != NULL)
              {  at = ALPHA(form->carp);
                 if (at->celltype == ALPHAATOM)
                 {   at->tracebit = TRACE_OFF;
                     form = form->cdrp;
                     continue;
                 };
              };
              ierror("untrace");
        };
        return(hold);
}
