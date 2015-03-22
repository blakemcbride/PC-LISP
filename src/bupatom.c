/* EDITION AD01, APFUN PAS.765 (91/12/10 16:57:44) -- CLOSED */                 
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bupatom: Extract the one list or atom parameter and call 'printlist'**
 ** in the main.c module to print the result. Print without delimiters. **
 *************************************************************************/
struct conscell *bupatom(form)
struct conscell *form;
{      struct conscell *l; struct filecell *p;
       if (form != NULL) {
          l = form->carp;
          form = form->cdrp;
          if (form == NULL) {
              printlist(stdout,l,DELIM_OFF,NULL,NULL);
              return(l);
          } else {
              if ((form->carp != NULL) && (form->cdrp == NULL)) {
                 if (form->carp->celltype == FILECELL) {
                     p = PORT(form->carp);
                     if (p->atom == NULL) goto IOERR;
                     if (p->issocket && p->state == 1) rewind(p->atom);   /* if was reading socket rewind it */
                     p->state = 2;                                        /* new state is writing */
                     printlist(p->atom,l,DELIM_OFF,NULL,NULL);
                     if (ferror(p->atom)) goto IOERR;
                     return(l);
                 }
              }
          }
       }
       ierror("patom");
IOERR: ioerror(p);
}
