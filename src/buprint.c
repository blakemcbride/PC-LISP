/* EDITION AD01, APFUN PAS.765 (91/12/10 16:57:44) -- CLOSED */                 
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buprint: Extract the one list or atom parameter and call 'printlist'**
 ** in the main.c module to print the result. We print with delimiters  **
 *************************************************************************/
struct conscell *buprint(form)
struct conscell *form;
{      struct conscell *l; struct filecell *p;
       if (form != NULL) {
          l = form->carp;
          form = form->cdrp;
          if (form == NULL) {
              printlist(stdout,l,DELIM_ON,NULL,NULL);
              return(l);
          } else {
              if ((form->carp != NULL) && (form->cdrp == NULL)) {
                 if (form->carp->celltype == FILECELL) {
                     p = PORT(form->carp);
                     if (p->atom == NULL) goto IOERR;
                     if (p->issocket && p->state == 1) rewind(p->atom);     /* if was reading socket rewind before writing */
                     p->state = 2;                                          /* set new state to writing */
                     printlist(p->atom,l,DELIM_ON,NULL,NULL);
                     if (ferror(p->atom)) goto IOERR;
                     return(l);
                 }
              }
          }
       }
       ierror("print");
IOERR: ioerror(p);
}
