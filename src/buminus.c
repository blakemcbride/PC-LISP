

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** (- i1 i2 .... in) Subtract a list of fixnums and return result.     **
 *************************************************************************/
struct conscell *buminus(form)
struct conscell *form;
{      struct fixcell *op; long int sum = 0L; int first = 1;
       xpush(form);
       while (form != NULL)
       {      op = (struct fixcell *) form->carp;
              if ((op != NULL)&&(op->celltype == FIXATOM))
                   sum =  first ? (op->atom) : (sum - op->atom);
              else
                   ierror("-");
              form = form->cdrp;
              first = 0;
       };
       xret(newintop(sum),1);
}
