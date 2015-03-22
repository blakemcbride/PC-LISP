/* EDITION AB02, APFUN PAS.677 (91/01/24 10:35:28) -- CLOSED */                 
/* */
/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <ctype.h>
#include "lisp.h"

#if !defined(tolower)
#   define tolower(c) ((c)-'A'+'a')
#endif

/*
 |  string <- (tolower string)
 |  ~~~~~~~~~~~~~~~~~~~~~~~~~~
 |  Obviously this function returns its argument string with all upper case
 |  characters converted to lower case.
 */
struct conscell *butolower(form)
   struct conscell *form;
{
   char *s, *w, work[MAXATOMSIZE];
   if ((form != NULL)&&(form->cdrp == NULL)) {
       if (GetString(form->carp, &s)) {
           for(w = &work[0]; *s != '\0'; s++, w++)
               *w = isupper(*s) ? tolower(*s) : *s;
           *w = '\0';
           return(LIST(insertstring(work)));
       }
   }
   ierror("tolower");
}
