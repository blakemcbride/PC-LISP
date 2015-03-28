

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (listify fixnum) Will return the arguments to the current encolsing **
 ** lexpr as a list starting with the fixnum'th arg. If fixnum is neg-  **
 ** ative then we return the arg list starting fixnum from the end. If  **
 ** the specified location is outside the bounds of the list of parms we**
 ** return nil. This is not treated as an error because it simplifies   **
 ** the handling of &rest arguments they can be simply (listify N) and  **
 ** do not require any testing of (arg) for legal range.                **
 *************************************************************************/
struct conscell *bulistify(form)
struct conscell *form;
{      struct conscell *alist; long int n,pos,len;
       if ((form != NULL)&&(form->cdrp == NULL))
       {   if ((blexprhold->valstack != NULL)&&(GetFix(form->carp,&n)))
           {   alist = blexprhold->valstack->carp;
               len = FIX(alist->carp)->atom;
               pos = n > 0L? n : len + n + 1L;
               if ((pos > 0)&&(pos <= len))
               {   while(pos--) alist = alist->cdrp;
                   return(alist);
               };
               return(NULL);
           };
       };
       ierror("listify");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
