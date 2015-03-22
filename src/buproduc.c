/* EDITION AB02, APFUN MR.68 (90/04/18 09:23:52) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include "lisp.h"

/*************************************************************************
 ** buprod (product -numbers-) Multiply numbers as floating points, if  **
 ** all are fixnums then return a fixnum otherwise return a flonum.     **
 *************************************************************************/
struct conscell *buproduct(form)
struct conscell *form;
{      double prod = 1.0, op; int isflt = 0;
       while(form!=NULL) {
	   if (!GetFloat(form->carp,&op)) ierror("times|product");
	   isflt = isflt || (form->carp->celltype == REALATOM);
	   prod *= op;
	   form = form->cdrp;
       }
       if (isflt) return(newrealop(prod));
       return(newintop((long) prod));
}
