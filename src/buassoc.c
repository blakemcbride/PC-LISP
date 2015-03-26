

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** buassoc: The assoc function, takes from its list of arguments 'form'**
 ** the parameters 'var' and 'alist' to pass to the function assoc:     **
 ** EG (assoc 'IBM '( (GE.appliances) (IBM.computers))                  **
 ** is (IBM.computers)                                                  **
 *************************************************************************/
struct conscell *buassoc(form)
struct conscell *form;
{      struct conscell *var,*alist;
       xpush(form);
       if (form != NULL)
       {    var = form->carp;
            form = form->cdrp;
            if ((form != NULL)&&(form->cdrp == NULL))
            {    alist = form->carp;
                 if ((alist == NULL)||(alist->celltype == CONSCELL))
                     xret(assoc(var,alist),1);
            };
       };
       ierror("assoc");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
