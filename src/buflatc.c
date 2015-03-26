

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buflatc: call printlist with DELIM_OFF to compute the flat print    **
 ** size of the list. This makes use of the *counter last argument to   **
 ** the print function. Note that printlist will stop when counter is   **
 ** greater or equal to zero. So for normal use we set counter to the   **
 ** largest NEGATIVE integer that system will handle. If the user gives **
 ** a limit we set the counter to that value.                           **
 *************************************************************************/
struct  conscell *buflatc(form)
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
            printlist(NULL,expr,DELIM_OFF,NULL,&counter);
            return(newintop((long)(counter - limit)));
        };
ERR:    ierror("flatc");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
