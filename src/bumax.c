/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:42) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** (max -numbers-) Find the largest in a list of FIXATOMS or REALATOMS **
 ** and return this element.                                            **
 *************************************************************************/
struct conscell *bumax(form)
struct conscell *form;
{      struct conscell *maxsofar,*op; int first = 1, result;
       if (form == NULL) return(newintop(0L));
       while (form != NULL)
       {      op = (struct conscell *) form->carp;
              if (first)
              {   if ((op!=NULL)&&(op->celltype==FIXATOM)||(op->celltype==REALATOM))
                      maxsofar = op;
                  else
                      ierror("max");
              }
              else
              {   if ((result = MixedTypeCompare(op,maxsofar)) == MT_GREATER)
                      maxsofar = op;
                  else
                      if (result == MT_ERROR)
                          ierror("max");
              };
              form = form->cdrp;
              first = 0;
       };
       return(maxsofar);
}
