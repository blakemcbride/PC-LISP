/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:26) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buequal() Will return 't' if the two parameters are 'equal' we call **
 ** the above routine to figure out if this is the case.                **
 *************************************************************************/
struct conscell *buequal(form)
struct conscell *form;
{      struct conscell *e1, *e2;
       if (form != NULL)
       {   e1 = form->carp;
           form = form->cdrp;
           if (form != NULL)
           {   e2 = form->carp;
               if (form->cdrp == NULL)
               {   if (equal(e1,e2))
                       return(LIST(thold));
                   else
                       return(NULL);
               };
           };
       };
       ierror("equal");
}
