

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include "lisp.h"

/*************************************************************************
 ** buquotient (quotient -numbers-) Divide the numbers as flonums, if   **
 ** all are fixnums then return as a fixnum otherwise return a flonum.  **
 *************************************************************************/
struct conscell *buquotient(form)
struct conscell *form;
{      double quot,op; int isflt;
       if (form == NULL) return(newintop(1L));
       if (!GetFloat(form->carp,&quot)) goto ERR;
       isflt = (form->carp->celltype == REALATOM);
       form = form->cdrp;
       while(form!=NULL) {
	    if (!GetFloat(form->carp,&op)) goto ERR;
	    isflt = isflt || (form->carp->celltype == REALATOM);
	    if (op == 0.0) gerror("divide by zero");
	    quot /= op;
	    form = form->cdrp;
       }
       if (isflt) return(newrealop(quot));
       return(newintop((long) quot));
ERR:   ierror("quotient");
}
