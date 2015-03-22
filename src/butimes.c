/* EDITION AB01, APFUN MR.68 (90/04/18 09:24:06) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** butimes: Multiply the list of reals and return result.   (0 defualt)**
 *************************************************************************/
struct conscell *butimes(form)
struct conscell *form;
{      struct fixcell *op; long int prod = 1L;
       xpush(form);
       while (form != NULL)
       {      op = (struct fixcell *) form->carp;
              if ((op != NULL)&&(op->celltype == FIXATOM))
                   prod *= op->atom;
              else
                   ierror("*");
              form = form->cdrp;
       };
       xret(newintop(prod),1);
}
