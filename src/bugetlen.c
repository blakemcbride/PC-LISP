/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:32) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (getlength array) Returns the length of the array 'array'           **
 *************************************************************************/
struct conscell *bugetlength(form)
struct conscell *form;
{      struct arraycell *temp;
       if ((form != NULL)&&(form->cdrp == NULL))
       {   if (ExtractArray(form->carp,&temp))
              return(temp->info->carp);
       };
       ierror("getlength");
}
