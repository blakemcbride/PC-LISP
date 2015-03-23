

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (time-string [fixnum]) Will return a string representing the time   **
 ** fixnum seconds after creation. If fixnum is not provided then the   **
 ** current time string is returned.                                    **
 *************************************************************************/
struct conscell *butimestring(form)
struct conscell *form;
{
       long int fix; int len;
       char *s, *ctime();
       fix = time(NULL);
       if (form != NULL)
          if ((form->cdrp != NULL)||(!GetFix(form->carp,&fix))) goto er;
       s = ctime(&fix);
       if (s == NULL) goto er;
       len = strlen(s);
       if (len <= 0) goto er;
       len -= 1;
       if (s[len] == '\n') s[len] = '\0';    /* ctime may put a new line, get rid of it */
       return(LIST(insertstring(s)));
er:    ierror("time-string");
 }
