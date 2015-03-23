

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** bueq : this is the built in 'eq' function. Given two S-expressions  **
 ** returns 't' if they are the same object. This just means they have  **
 ** the same address. However, to make PC-LISP compatible with Franz,   **
 ** we return t when (eq fixnum fixnum) is run. This is a kludge but it **
 ** makes less changes necessary to Franz code.                         **
 *************************************************************************/
struct conscell *bueq(form)
struct conscell *form;
{      struct conscell *o1,*o2;
       if ((form != NULL)&&(form->cdrp != NULL))
       {    o1 = form->carp;
            form = form->cdrp;
            if ((form != NULL)&&(form->cdrp == NULL))
            {    o2 = form->carp;
                 if (eq(o1, o2)) return(LIST(thold));
                 return(NULL);
            };
       };
       ierror("eq");
}
