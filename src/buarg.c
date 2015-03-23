

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (arg [fixnum]) Will return the fixnum'th argument to the current    **
 ** enclosing lexpr, or if fixnum is not given will return the number   **
 ** of args that this lexpr has. The arg list for the current enclosing **
 ** lexpr is stored on the value stack for the special holding atom     **
 ** blexprhold. The first element of this list is the number or args in **
 ** the list, not including the added length element. If the number is  **
 ** is bigger than the number of actual args given we ierror().         **
 *************************************************************************/
struct conscell *buarg(form)
struct conscell *form;
{      long int n,len;
       struct conscell *alist;
       if (blexprhold->valstack != NULL)
       {   alist = blexprhold->valstack->carp;
           if (form == NULL) return(alist->carp);
           len = FIX(alist->carp)->atom;
           if (GetFix(form->carp,&n) && (form->cdrp == NULL))
           {   if ((n > 0L) && (n <= len))
               {  while(n--) alist = alist->cdrp;
                  return(alist->carp);
               };
           };
       };
       ierror("arg");
}
