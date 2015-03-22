/* EDITION AB02, APFUN PAS.685 (91/02/13 17:17:00) -- CLOSED */                 
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/***************************************************************************
 ** bugpname(get_pname symbol) Return a string whose print name is symbol **
 ***************************************************************************/
struct conscell *bugpname(form)
struct conscell *form;
{      char *s; char work[MAXATOMSIZE];
       if ((form != NULL)&&(form->cdrp == NULL))
       {   if (GetString(form->carp, &s)) {
               strcpy(work,s);                   /* copy to avoid relocation of 's' */
               return(LIST(insertstring(work))); /* when we insert */
           }
       };
       ierror("get_pname");
}
