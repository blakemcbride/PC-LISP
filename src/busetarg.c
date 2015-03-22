/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:58) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (setarg fixnum exp) Will set the fixnum'th argument to the current  **
 ** enclosing lexpr to be exp. We first get the current enclosing lexpr **
 ** bodies arg list by reading it from the blexprhold binding stack,    **
 ** then extracting the length of the arg list which is stored in the   **
 ** car of this list. Then we just check the range of the parameter,    **
 ** loop to get to that elements cons cell, then substitute it's car.   **
 *************************************************************************/
struct conscell *busetarg(form)
struct conscell *form;
{      long int n,len;
       struct conscell *alist;
       if (blexprhold->valstack != NULL)
       {   alist = blexprhold->valstack->carp;
           len = FIX(alist->carp)->atom;
           if (form != NULL)
           {   if (GetFix(form->carp,&n) && (form->cdrp != NULL))
               {   form = form->cdrp;
                   if ((n > 0L) && (n <= len)&&(form->cdrp == NULL))
                   {    while(n--) alist = alist->cdrp;
                        alist->carp = form->carp;
                        return(alist->carp);
                   };
               };
           };
       };
       ierror("setarg");
}
