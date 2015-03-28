

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buppform:  The basic pretty printer. Will pretty print the first    **
 ** expression to standard output, or to given port. Indenet will be 0  **
 ** or given indent.                                                    **
 *************************************************************************/
struct  conscell *buppform(form)
struct  conscell *form;
{       struct conscell *expr; FILE *port; struct filecell *p; int offset;
        port = stdout; offset = 0;
        if (form != NULL) {
            expr = form->carp;
            form = form->cdrp;
            if (form != NULL) {
                if ((form->carp == NULL)||(form->carp->celltype != FILECELL)) goto ERR;
                p = PORT(form->carp);
                if (p->issocket && p->state == 1) rewind(p->atom);     /* if was reading socket rewind before writing */
                p->state = 2;                                          /* set new state to writing */
                port = p->atom;
                form = form->cdrp;
                if (form != NULL) {
                    if ((form->carp == NULL)||(form->carp->celltype != FIXATOM)) goto ERR;
                    offset = (int) FIX(form->carp)->atom;
                    if (form->cdrp != NULL) goto ERR;
                }
            }
            prettyprint(expr,offset,0,port);    /* indent so far = 0 */
            fprintf(port,"\n");
            return(LIST(thold));
        }
ERR:    ierror("pp-form");  /*  doesn't return  */
        return NULL;   /*  keep compiler happy  */
}
