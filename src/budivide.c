

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** budivide:Divide the first from the rest of list, ret res.(0 defualt)**
 *************************************************************************/
struct conscell *budivide(form)
struct conscell *form;
{      struct fixcell *op; long int quot = 1L; int first = 1;
       xpush(form);
       while (form != NULL)
       {      op = (struct fixcell *) form->carp;
              if ((op != NULL)&&(op->celltype == FIXATOM))
              {    if (first)
                       quot = op->atom;
                   else
                       if (op->atom == 0L)
                           gerror("divide by zero");
                       else
                           quot /= op->atom;
              }
              else
                   ierror("/");
              form = form->cdrp;
              first = 0;
       };
       xret(newintop(quot),1);
}
