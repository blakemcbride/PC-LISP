/* EDITION AB01, APFUN MR.68 (90/04/18 09:24:02) -- CLOSED */                   
/* */
/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include "lisp.h"

/*
 |  fixnum <- (strlen string)
 |  ~~~~~~~~~~~~~~~~~~~~~~~~~
 |  Obviously this function returns its length of its argument string.
 */
struct conscell *bustrlen(form)
   struct conscell *form;
{
   char *s;
   if ((form != NULL)&&(form->cdrp == NULL))
      if (GetString(form->carp, &s))
          return(LIST(newintop((long)strlen(s))));
   ierror("strlen");
}
