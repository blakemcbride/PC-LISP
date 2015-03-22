/* EDITION AC01, APFUN PAS.765 (91/12/10 16:57:44) -- CLOSED */                 
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (mod i1 i2) return i1 % i2 (Remainder after i1 divided by i2).      **
 *************************************************************************/
struct conscell *bumod(form)
struct conscell *form;
{      struct fixcell *op1,*op2;
       xpush(form);
       if (form != NULL) {
           op1 = FIX(form->carp);
           form = form->cdrp;
           if ((op1->celltype == FIXATOM)&&(form != NULL)) {
                op2 = FIX(form->carp);
                if (op2->atom != 0) {
                    form = form->cdrp;
                    if ((form == NULL)&&(op2->celltype == FIXATOM))
                       xret(newintop((long)(op1->atom % op2->atom)),1);
                }
           }
       }
       ierror("mod");
}
