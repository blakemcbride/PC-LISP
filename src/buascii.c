/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:22) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buascii : given a realatom parameter it will return an alpha atom of**
 ** one character corresponding to the ascii value of the number. It    **
 ** will not check the range except that it must be 0..255.             **
 *************************************************************************/
struct conscell *buascii(form)
struct conscell *form;
{      char work[2]; long int v; work[1] = '\0';
       if ((form != NULL)&&(form->cdrp == NULL))
       {   if (GetFix(form->carp,&v))
           {   if ((v >= 0L) && (v < 256L))
               {  *work = (char) v;
                  return(LIST(CreateInternedAtom(work)));
               };
           };
       };
       ierror("ascii");
}
