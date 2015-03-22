/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:50) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** buplist: Returns the property list associated with atom which is its**
 ** parameter. NIL if the list is empty. We return a top level copy of  **
 ** this list because the top level is not stable, it may be destroyed  **
 ** by (remprop).                                                       **
 *************************************************************************/
struct conscell *buplist(form)
struct conscell *form;
{      if (form != NULL)
       {  if (form->cdrp == NULL)
          {  form = form->carp;
             if (form != NULL)
                if (form->celltype == ALPHAATOM)
                   return(topcopy(ALPHA(form)->proplist));
          };
       };
       ierror("plist");
}
