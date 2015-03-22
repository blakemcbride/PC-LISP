/* EDITION AB02, APFUN PAS.710 (91/04/08 12:05:14) -- CLOSED */                 
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** intexpt(x,y) compute x^y where x & y are both integers. We use a    **
 ** a very simplistic doubling algorithm. If asked for x^0 return 1,    **
 ** if asked for x^y where y < 0 return 0 because 1/(y^x) is in range   **
 ** 0..1. If asked for x^y where y > 0 we loop checking if y is odd, if **
 ** it is we form the product z by just multiplying it by x once and    **
 ** decrementing the exponent, however if the exponent y is even, it is **
 ** safe to double x and divide y by 2 and maintain the same result, by **
 ** doubling and halving we make the algorithm O(lg(n)) intead of O(n). **
 *************************************************************************/
long intexpt(x,y)
long x,y;
{    long z;
     if (y < 0) return(0L);                        /* neg exponent ans=0 */
     z = 1L;                                       /* x^0 = 1 = z */
     while(y > 0)
         if (y&1){z*=x;y--;} else {y>>= 1;x*=x;};  /* accumulate z */
     return(z);
}

/*************************************************************************
 ** buexpt: (expt x y) Will return the result x^y. If the numbers are   **
 ** both fixnums we call intexpt to compute the result using a quick    **
 ** doubling algorithm. If however one of the args is a double we call  **
 ** on the IEE floating point power function 'pow' to do the work for us**
 ** It probably uses x^y = e^(y*log(x)).                                **
 *************************************************************************/
struct conscell *buexpt(form)
struct conscell *form;
{      struct conscell *p1,*p2; double fop1,fop2;
       if (form != NULL) {
           p1 = form->carp;
           form = form->cdrp;
           if ((form != NULL)&&(form->cdrp == NULL)) {
              p2 = form->carp;
              if ((p1!=NULL)&&(p2!=NULL)) {
                   if ( (p1->celltype == FIXATOM) && (p2->celltype == FIXATOM) )
                        return( newintop( intexpt( FIX(p1)->atom, FIX(p2)->atom ) ) );
                   if ( GetFloat(p1, &fop1) && GetFloat(p2, &fop2) )
                        return( newrealop( pow( fop1, fop2 ) ) );
              }
           }
       }
       ierror("expt");
}
