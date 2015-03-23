/* */
/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <ctype.h>
#include "lisp.h"

/*
 |  string <- (strcomp string)
 |  ~~~~~~~~~~~~~~~~~~~~~~~~~~
 |  This function returns its argument string with all white spaced removed,
 |  ie tabs, new lines etc.
 */
struct conscell *bustrcomp(form)
   struct conscell *form;
{
   char *s,*t,work[MAXATOMSIZE];
   if ((form != NULL)&&(form->cdrp == NULL)) {
      if (GetString(form->carp, &s)) {
          for(t = &work[0]; *s != '\0'; s++)
              if (!isspace(*s))
                 *t++ = *s;
          *t = '\0';
          return(LIST(insertstring(work)));
      }
   }
   ierror("strcomp");
}



