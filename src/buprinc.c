

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buprinc: Extract the one list or atom parameter and call 'printlist'**
 ** in the main.c module to print the result. Print without delimiters. **
 ** Identical to patom except that we return r instead of the list.     **
 *************************************************************************/
struct conscell *buprinc(form)
struct conscell *form;
{      struct conscell *l; struct filecell *p;
       if (form != NULL) {
          l = form->carp;
          form = form->cdrp;
          if (form == NULL) {
              printlist(stdout,l,DELIM_OFF,NULL,NULL);
              return(LIST(thold));
          } else {
              if ((form->carp != NULL) && (form->cdrp == NULL)) {
                 if (form->carp->celltype == FILECELL) {
                     p = PORT(form->carp);
                     if (p->atom == NULL) goto IOERR;
                     if (p->issocket && p->state == 1) rewind(p->atom);     /* if was reading socket rewind before writing */
                     p->state = 2;                                          /* set new state to writing */
                     printlist(p->atom,l,DELIM_OFF,NULL,NULL);
                     if (ferror(p->atom)) goto IOERR;
                     return(LIST(thold));
                 }
              }
          }
       }
       ierror("princ");
IOERR: ioerror(p);  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
