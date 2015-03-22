/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:54) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (remob atom) Will take that atom off the oblist. We do this by just **
 ** setting its interned bit to NOT_INTERNED. Thus all property etc,    **
 ** comes off the oblist as well. This remob/intern pair should be o.k. **
 ** as the Franz intention is not to create new atoms. This can be seen **
 ** by looking at the beginlocal/endlocal definitions in LISPcraft(243) **
 ** as interned/remob'ed atoms take their property/values/func with them**
 *************************************************************************/
struct conscell *buremob(form)
struct conscell *form;
{      struct alphacell *at;
       if ((form != NULL)&&(form->carp != NULL)&&(form->cdrp == NULL))
       {   at = ALPHA(form->carp);
           if (at->celltype == ALPHAATOM)
           {   at->interned = NOT_INTERNED;
               return(LIST(at));
           };
       };
       ierror("remob");
}
