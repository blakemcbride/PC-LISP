/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:34) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buhsize: (hunksize H) Will return the size of the hunk H. We just   **
 ** read the size from the hunk cell field 'size' and turn it into a    **
 ** fixnum cell, then return it.                                        **
 *************************************************************************/
struct conscell *buhsize(form)
struct conscell *form;
{      if ((form != NULL)&&(form->carp != NULL)&&(form->cdrp == NULL))
       {    if (form->carp->celltype == HUNKATOM)
                return(newintop((long)HUNK(form->carp)->size ));
       };
       ierror("hunksize");
}
