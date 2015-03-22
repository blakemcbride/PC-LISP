/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:26) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bucons : this function is the built in `cons' function, and will if **
 ** given two parameters the second of which must be a list, return the **
 ** result of constructing a new list, the head of which is the first   **
 ** parameter and the tail of which is the second parameter. For example**
 ** (cons a (b c d)) is just (a b c d)                                  **
 *************************************************************************/
struct conscell *bucons(form)
struct conscell *form;
{      struct conscell *temp1,*temp2;
       xpush(form);
       if (form != NULL)
       {   temp1 = form->carp;
           form = form->cdrp;
           if ((form != NULL)&&(form->cdrp == NULL))
           {   temp2 = new(CONSCELL);
               temp2->carp=temp1;
               temp2->cdrp=form->carp;
               fret(temp2,1);
           };
       };
       ierror("cons");
}
