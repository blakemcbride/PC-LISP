#include        <stdio.h>
#include        <ctype.h>
#include        "lisp.h"

/*
 |    (subst <exp1> <exp2> <list>)
 |  & (dsubst <exp1> <exp2> <list>)
 |
 | These built in functions will either destructively dsubst or non-destructively
 | subst substitute all 'equal' occurrences of <exp2> in <list> for <exp1>. This
 | is done recursively for all levels in <list>.
 */

/*
 | Extract parms for (subst) and (dsubst).
 */
static int getsubstparms(form, e1, e2, l)
struct conscell *form, **e1,**e2, **l;
{      if (form == NULL) goto er;
       *e1 = form->carp;
       if ((form = form->cdrp) == NULL) goto er;
       *e2 = form->carp;
       if ((form = form->cdrp) == NULL) goto er;
       *l = form->carp;
       if ((*l != NULL)&&((*l)->celltype != CONSCELL)) goto er;
       if (form->cdrp != NULL) goto er;
       return(1);
er:    return(0);
}

/*
 | dsubst(e1,e2,l) - will recursively traverse 'l' at all levels and will
 | destructively substitute all 'equal' occurrences of e2 with e1. It also
 | correctly handles CDR pointers that point to things other than CONS cells.
 | I.e it handles dotted pairs properly.
 */
static void dsubst(e1,e2,l)
struct conscell *e1, *e2, *l;
{      struct conscell *n;
       if ((l != NULL)&&(l->celltype == CONSCELL)) /* not a base case ? */
       {  for(;;) {                                /* loop accross list */
               if (equal(e2, l->carp))             /* match found ? */
                   l->carp = e1;                   /* destructively replace */
               else                                /* else no match so */
                   dsubst(e1, e2, l->carp);        /* recursively do sub lists */
               if ((n = l->cdrp) == NULL) break;   /* end of list ? */
               if (n->celltype != CONSCELL) {      /* dotted pair ? */
                   if (equal(e2, n))               /* if second of pair equal*/
                       l->cdrp = e1;               /* substitute e1 for it */
                   break;                          /* exit, end of list */
               } else                              /* else not dotted pair */
                   l = n;                          /* advance to next CONS */
          }
       }
}

/*
 | (dsubst <exp1> <exp2> <list>)
 |
 | This primitive will recursively replace all 'equal' occurrences of <exp1>
 | in <list> with <exp1>.
 */
struct conscell *budsubst(form)
struct conscell *form;
{      struct conscell *e1=NULL, *e2=NULL, *l=NULL;    /*  all =NULL to keep compiler happy  */
       if (!getsubstparms(form, &e1, &e2, &l)) ierror("dsubst");
       dsubst(e1, e2, l);
       return(l);
}

/*
 | (subst <exp1> <exp2> <list>)
 |
 | This primitive will recursively replace all 'equal' occurrences of <exp1>
 | in a copy of <list> with <exp1>.
 */
struct conscell *busubst(form)
struct conscell *form;
{      struct conscell *e1=NULL, *e2=NULL, *l=NULL;    /*  all =NULL to keep compiler happy  */
       if (!getsubstparms(form, &e1, &e2, &l)) ierror("subst");
       l = copy(l);
       dsubst(e1, e2, l);
       return(l);
}
