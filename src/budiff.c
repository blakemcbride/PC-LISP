

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include "lisp.h"

/*************************************************************************
 ** budiff (diff -numbers-) Subtract the numbers as floating points, if **
 ** all were fixnums then return a fixnum otherwise return a flonum.    **
 *************************************************************************/
struct conscell *budiff(form)
struct conscell *form;
{      double diff,op; int isflt;
       if (form == NULL) return(newintop(0L));
       if (!GetFloat(form->carp,&diff)) goto ERR;
       isflt = (form->carp->celltype == REALATOM);
       form = form->cdrp;
       while(form != NULL) {
	    if (!GetFloat(form->carp,&op)) goto ERR;
	    isflt = isflt || (form->carp->celltype == REALATOM);
	    diff -= op;
	    form = form->cdrp;
       }
       if (isflt) return(newrealop( diff ));
       return(newintop( (long) diff ));
ERR:   ierror("diff|difference");
}

