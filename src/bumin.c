/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:44) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** (min -numbers-) Find the smallest in a list of FIXATOMS or REALATOMS**
 ** and return this element.                                            **
 *************************************************************************/
struct conscell *bumin(form)
struct conscell *form;
{      struct conscell *minsofar,*op; int first = 1, result;
       if (form == NULL) return(newintop(0L));
       while (form != NULL)
       {      op = (struct conscell *) form->carp;
              if (first)
              {   if ((op!=NULL)&&(op->celltype==FIXATOM)||(op->celltype==REALATOM))
                      minsofar = op;
                  else
                      ierror("min");
              }
              else
              {   if ((result = MixedTypeCompare(op,minsofar)) == MT_LESS)
                      minsofar = op;
                  else
                      if (result == MT_ERROR)
                          ierror("min");
              };
              form = form->cdrp;
              first = 0;
       };
       return(minsofar);
}
