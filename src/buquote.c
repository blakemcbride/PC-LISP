/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:52) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buquote : built in quote function given a list of parameters 'form' **
 ** will check that its parameter list is not empty and that it only    **
 ** contains one parameter. If it does then this parameter is returned  **
 ** un changed. Ie (quote (a b c)) is (a b c). Note that there is one   **
 ** exception if the form is (quote nil) we return NULL. This is because**
 ** 'nil == nil == () == '().                                           **
 *************************************************************************/
struct conscell *buquote(form)
struct conscell *form;
{      if ((form != NULL)&&(form->cdrp == NULL))
            return(form->carp);
       ierror("quote");
}

