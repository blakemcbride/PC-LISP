

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (arg? fix exp) Will return the fixnum'th argument to the current    **
 ** enclosing lexpr, or if fixnum is outide the range of provided parms **
 ** will return 'exp'. This is a NON FRANZ function that is used to help**
 ** implement effecient &optional parameters. See the lexprify() code.  **
 *************************************************************************/
struct conscell *buargq(form)
struct conscell *form;
{      long int n,len;
       struct conscell *alist;
       if (blexprhold->valstack != NULL)
       {   alist = blexprhold->valstack->carp;
           len = FIX(alist->carp)->atom;
           if ((form!=NULL)&&(form->cdrp!=NULL)&&(GetFix(form->carp,&n)))
           {   form = form->cdrp;
               if ((form != NULL)&&(form->cdrp == NULL))
               {  if ((n > 0L) && (n <= len))
                  {  while(n--) alist = alist->cdrp;
                     return(alist->carp);
                  };
                  return(form->carp);
               };
           };
       };
       ierror("arg?");
}
