

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bureturn : is the return  function which is called from within a    **
 ** prog: the value of the function is just the list (return . arg).    **
 ** This will be used only by prog to detect when and what to return.   **
 ** If no arg is given we return (return . nil) i.e. (return).          **
 *************************************************************************/
struct conscell *bureturn(form)
struct conscell *form;
{      register struct conscell *f;
       if (form != NULL) {
           if (form->cdrp) ierror("return");
           form = form->carp;
       }
       f = new(CONSCELL);
       f->carp = (struct conscell *) returnhold;
       f->cdrp = form;
       return(f);
}
