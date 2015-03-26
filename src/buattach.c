#include        <stdio.h>
#include        <ctype.h>
#include        "lisp.h"

/*
 | (attach <exp> <list>)
 |
 | This primitive will physically modify <list> so that it has <exp> as the
 | new head. However, this is done by reusing the old head cell so that all
 | references to <list> now include the new head. The contents of the old
 | head are copied to a new cons cell which is linked in as the second element
 | in the list thus effecting the required change.
 */
struct conscell *buattach(form)
struct conscell *form;
{      struct conscell *n, *e, *l;

      /*
       | Basic error checking. Make sure have two parameters the second of
       | which must be a list.
       */
       if (form == NULL) goto er;
       e = form->carp;
       if ((form = form->cdrp) == NULL) goto er;
       if (form->cdrp != NULL) goto er;
       l = form->carp;
       if ((l == NULL)||(l->celltype != CONSCELL)) goto er;

      /*
       | Create a new cell 'n' and copy the head of 'l' into n's car and
       | cdr fields. Next destructively modify the head of l to point to
       | the expression 'e' and make the new cdr of 'l' be the new conscell.
       | This is a sort of destructive right shift by one. The new modified
       | list is the return value.
       */
       n = new(CONSCELL);
       n->carp = l->carp;
       n->cdrp = l->cdrp;
       l->carp = e;
       l->cdrp = n;
       return(l);

er:    ierror("attach");  /* doesn't return  */
       return NULL;
}

