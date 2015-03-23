

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buforeach: Similar to prog but its syntax is as follows:            **
 **                                                                     **
 **     (foreach atom list e1 e2 e3 .... eN)                            **
 **                                                                     **
 ** Foreach atom list, will iterate the binding of atom through each of **
 ** the elements in list and will for each binding evaluat each of e1.. **
 ** eN. It allows labels and go/return similar to prog.                 **
 *************************************************************************/
struct conscell *buforeach(form)
struct conscell *form;
{      register struct conscell *temp,*value,*pp,*local;
       struct conscell *vallis;
       int has_labels = 0;
       lillev += 1;                                             /* up lexical scope */
       if (form == NULL) goto er;
       xpush(form); push(vallis);
       value = local = NULL;
       local = form->carp;                                      /* find atom local var */
       if ((local == NULL)||(local->celltype != ALPHAATOM))     /* verify it is atom */
             goto er;
       bindvar(local,NULL);                                     /* initialize atom to NIL */
       form = form->cdrp;                                       /* advance to (list e1... */
       if (form == NULL) goto er;                               /* if no list parm error */
       vallis = eval(form->carp);                               /* vallis is the list of values */
       form = form->cdrp;                                       /* now on (e1 e2 ... eN) */
       if (vallis == NULL) goto done;                           /* if no values to iterate we are done */
       temp = form;                                             /* set program pointer (pp) etc. */
       while(temp != NULL) {                                    /* loop and look for labels each */
             if (temp->carp->celltype == ALPHAATOM) {           /* is bound to its point in the body */
                 bindlabel(temp->carp,temp->cdrp);              /* of (e1 .... eN) */
                 has_labels = 1;
             }
             temp = temp->cdrp;
       }
       while(vallis != NULL) {                                  /* iterate local through vallis */
             if (vallis->celltype != CONSCELL) goto er;         /* if list parm is not a list : error */
             ALPHA(local)->valstack->carp = vallis->carp;       /* bind local to next elem in vallis */
             vallis = vallis->cdrp;                             /* wind in vallis for next iteration */
             pp = form;
             while(pp != NULL) {
                 if ((pp->carp != NULL)&&(pp->carp->celltype == ALPHAATOM))
                     pp = pp->cdrp;                             /* skip a label */
                 else {
                     value = temp = eval(pp->carp);
                     if (temp != NULL) {
                         if (temp->carp == (struct conscell *)returnhold) {
                             value = temp->cdrp;
                             goto done;
                         } else
                             if (temp->carp == (struct conscell *)gohold)
                                 pp = temp->cdrp;
                             else
                                 pp = pp->cdrp;                 /* do next SEXP */
                     } else                                     /* null return*/
                         pp = pp->cdrp;                         /* do next list*/
                 }
             }
       }
done:  unbindvar(local);                                        /* done, so pop the local and labels */
       if (has_labels) {
          while (form != NULL) {
             if ((form->carp != NULL)&&(form->carp->celltype == ALPHAATOM))
                 unbindvar(form->carp);
             form = form->cdrp;
          }
       }
       lillev -= 1;                                            /* down lexical scope */
       fret(value,2);
er:    ierror("foreach");
}
