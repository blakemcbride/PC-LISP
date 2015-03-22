/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:30) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buflatsize: just like buflatc but with DELIM_ON  on the print call. **
 *************************************************************************/
struct  conscell *buflatsize(form)
struct  conscell *form;
{       int counter = MAXNEGINT, limit = MAXNEGINT;
        struct conscell *expr;
        if (form != NULL)
        {   expr = form->carp;
            form = form->cdrp;
            if (form != NULL)
            {   if ((form->cdrp != NULL)||(form->carp == NULL)||
                   (form->carp->celltype != FIXATOM)) goto ERR;
                counter = limit = -(int)FIX(form->carp)->atom;
            };
            printlist(NULL,expr,DELIM_ON,NULL,&counter);
            return(newintop((long)(counter - limit)));
        };
ERR:    ierror("flatsize");
}
