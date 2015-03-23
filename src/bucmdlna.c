

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/***************************************************************************
 ** (command-line-args) Will return a list of the command line arguments. **
 ***************************************************************************/
struct conscell *bucmdlna(form)
struct conscell *form;
{      struct conscell *head, *n; int i;
       push(head);
       if (form != NULL) ierror("command-line-args");
       if ((liargv == NULL) || (liargc == 0)) return(NULL);
       for(i = liargc - 1; i >= 0; i--) {
           n = new(CONSCELL);
           n->cdrp = head;
           head = n;
           if (strlen(liargv[i]) >= MAXATOMSIZE) return(NULL);
           head->carp = LIST(insertstring(liargv[i]));
       }
       xpop(1);
       return(head);
}
