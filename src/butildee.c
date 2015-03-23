/* */
/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#include "lisp.h"

/*
 |  string <- (tilde-expand string)
 |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 |  This function expands the ~fred part of the path name string. If no tilde starts the
 |  string the input string is returned.
 */
struct conscell *butildeexpand(form)
   struct conscell *form;
{
   char *s, *t, head[MAXATOMSIZE], work[MAXATOMSIZE];
   struct passwd *pwent;

   if ((form != NULL)&&(form->cdrp == NULL)) {
      if (GetString(form->carp, &s)) {

         /*
          | If argument does not start with a tilde then just return input argument string.
          */
          if (*s++ != '~')
              return(form->carp);

         /*
          | If argument starts with ~/ then the path to be expanded must start with the
          | user home directory so get the password entry for the current user, otherwise
          | extract the name from work up to the next '/' and look up this as the pass word
          | entry to use. Also "~" is treated as equivalent to "~/".
          */
          if ((*s != '/' )&& (*s != '\0')) {
              t = &head[0];
              while((*s != '/') && (*t++ = *s++));
              if (*s == '/') { s++; *t = '\0'; } else s--;
              pwent = getpwnam(head);
          } else {
              if (*s == '/') s++;
              pwent = getpwuid(getuid());
          }

         /*
          | If there is no password entry for this file then just return NIL.
          */
          if (pwent == NULL) return(NULL);

         /*
          | Now make sure the home directory plus the divider plus the rest of the path
          | will fit in the output string and create the <home>/<rest> path and return
          | it.
          */
          if (strlen(pwent->pw_dir) + strlen(s) + 1 >= sizeof(work)) goto er;
          if (*s != '\0')
              sprintf(work,"%s/%s", pwent->pw_dir, s);
          else
              strcpy(work, pwent->pw_dir);
          return(LIST(insertstring(work)));
      }
   }
er:ierror("tilde-expand");
}



