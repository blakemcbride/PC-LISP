/* EDITION AB02, APFUN MR.68 (90/04/18 09:24:04) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include "lisp.h"

/*************************************************************************
 ** busum: (sum -numbers-) Add up numbers as floating points, then if   **
 ** all numbers were fixnums return a fixnum otherwise return flonum.   **
 *************************************************************************/
struct conscell *busum(form)
struct conscell *form;
{      double sum = 0.0, op; int isflt = 0;
       while(form!=NULL) {
	   if (!GetFloat(form->carp,&op)) ierror("sum|plus|add");
	   isflt = isflt || (form->carp->celltype == REALATOM);
	   sum += op;
	   form = form->cdrp;
       }
       if (isflt) return(newrealop(sum));
       return(newintop( (long) sum ));
}


