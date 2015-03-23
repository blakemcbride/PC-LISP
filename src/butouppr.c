/* */
/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <ctype.h>
#include "lisp.h"

#if !defined(toupper)
#   define toupper(c) ((c)-'a'+'A')   /* not defined on SUN workstation ctype.h */
#endif

/*
 |  string <- (toupper string)
 |  ~~~~~~~~~~~~~~~~~~~~~~~~~~
 |  Obviously this function returns its argument string with all lower case
 |  characters converted to upper case.
 */
struct conscell *butoupper(form)
   struct conscell *form;
{
   char *s, *w, work[MAXATOMSIZE];
   if ((form != NULL)&&(form->cdrp == NULL)) {
       if (GetString(form->carp, &s)) {
           for(w = &work[0]; *s != '\0'; s++, w++)
               *w = islower(*s) ? toupper(*s) : *s;
           *w = '\0';
           return(LIST(insertstring(work)));
       }
   }
   ierror("toupper");
}

