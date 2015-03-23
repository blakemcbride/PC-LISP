

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** buget : this function is the built in 'get' function and will take  **
 ** the parameters 'amt' and 'indic' from the list of parameters 'form' **
 ** and return the result of a call to the getprop function. For example**
 ** (get 'lisp 'author     ) would return mcarthy          if the put   **
 ** above was executed prior to this get.                               **
 *************************************************************************/
struct conscell *buget(form)
struct conscell *form;
{      struct conscell *atm,*indic;
       if ((form != NULL) && (form->carp != NULL)) {
            atm = form->carp;
            form = form->cdrp;
            if ((form != NULL) && (form->cdrp == NULL) && (atm->celltype == ALPHAATOM)) {
                 indic = form->carp;
                 return(getprop(atm,indic));
            }
       }
       ierror("get");
}
