

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** buputprop: The putprop    function and will extract the three parms **
 ** for putprop from the list of parameters 'form'. A call is made to   **
 ** putprop and the result is returned. SEE putprop for a descirption of**
 ** a property list. EG (putprop 'lisp 'mcarthy 'author)    will place  **
 ** (author . mcarthy) on the property list for atom 'lisp'.            **
 *************************************************************************/
struct conscell *buput(form)
struct conscell *form;
{      struct conscell *atm,*indic,*prop;
       xpush(form);
       if ((form != NULL)&&(form->carp->celltype == ALPHAATOM))
       {    atm = form->carp;
            form = form->cdrp;
            if (form != NULL)
            {    prop  = form->carp;
                 form = form->cdrp;
                 if ((form != NULL)&&(form->cdrp == NULL))
                 {    indic = form->carp;
                      xret(putprop(atm,indic,prop),1);
                 };
            };
       };
       ierror("putprop");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
