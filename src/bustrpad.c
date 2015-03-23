/* */
/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include "lisp.h"

/*
 |  string <- (strpad string n)
 |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 |  This function returns its argument string padded to n characters
 |  long.
 */
struct conscell *bustrpad(form)
   struct conscell *form;
{
   char *s, *w, work[MAXATOMSIZE];
   long int n; int m;
   if (form != NULL) {
       if (GetString(form->carp, &s)) {
          form = form->cdrp;
          if ((form != NULL)&&(form->cdrp == NULL)) {
             if (GetFix(form->carp, &n)) {
                 if ((n >= 0) && (n < MAXATOMSIZE)) {
                     m = strlen(s);
                     memcpy(work, s, m);
                     if (n > m) memset(work + m,' ',n - m);
                     work[n] = '\0';
                     return(LIST(insertstring(work)));
                 }
             }
          }
       }
   }
   ierror("strpad");
}

