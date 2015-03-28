/* */
/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <ctype.h>
#include "lisp.h"

/*
 |  string <- (strtrim string)
 |  ~~~~~~~~~~~~~~~~~~~~~~~~~~
 |  This function returns its argument string with all trailing white
 |  space removed, ie tabs, new lines etc.
 */
struct conscell *bustrtrim(form)
   struct conscell *form;
{
   char *s,*t,work[MAXATOMSIZE]; int n;
   if ((form != NULL)&&(form->cdrp == NULL)) {
      if (GetString(form->carp, &s)) {
          for(t = s + strlen(s) - 1; isspace(*t) && (t >= s); t--);
          n = (t >= s) ? (t - s) + 1 : 0;
          memcpy(work, s, n);
          work[n] = '\0';
          return(LIST(insertstring(work)));
      }
   }
   ierror("strtrim");  /*  doesn't return  */
   return NULL;   /*  keep compiler happy  */
}



