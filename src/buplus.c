

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** (+ i1 i2 .... in) Add a list of fixnums and return the result.      **
 *************************************************************************/
struct conscell *buplus(form)
struct conscell *form;
{      struct fixcell *op; long int sum = 0L;
       xpush(form);
       while (form != NULL)
       {      op = (struct fixcell *) form->carp;
              if ((op != NULL)&&(op->celltype == FIXATOM))
                   sum += op->atom;
              else
                   ierror("+");
              form = form->cdrp;
       };
       xret(newintop(sum),1);
}
