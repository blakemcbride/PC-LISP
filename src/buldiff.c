/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:38) -- CLOSED */                   
#include        <stdio.h>
#include        <ctype.h>
#include        "lisp.h"

/*
 | (ldiff <list1> <list2>)
 |
 | This primitive will return a list of all elements in list1 hat are not in
 | list2. The value returned is a new top level list. If list2 is nil then
 | list1 is returned. Eg:
 |
 |    -->(ldiff '(a b c) '(c d))
 |    '(a b)
 |
 | We use a very simple O(NxM) algorithm to do this. We just check each element
 | in list1 against each in list2.
 */
struct conscell *buldiff(form)
struct conscell *form;
{      struct conscell *l1, *l2, *l1s, *l2s, *rh, *rl;
       extern int eq();

      /*
       | Basic error checking, do we have two list parameters? Also check the
       | trivial case of either list parameter being NULL in which case we
       | return l1.
       */
       if (form == NULL) goto er;
       l1 = form->carp;
       if ((form = form->cdrp) == NULL) goto er;
       l2 = form->carp;
       if (form->cdrp != NULL) goto er;
       if ((l1 == NULL)||(l2 == NULL)) return(l1);
       if ((l1->celltype != CONSCELL)||(l2->celltype != CONSCELL)) goto er;

      /*
       | Build the return list with head 'rh' and tail 'rl'. We build it up
       | from head to tail. We loop l1s accross list1 and then for each of
       | the l1 elements we loop l2s through list2. If we find the element
       | of list1 in list2 then we break out of the inner loop and the test
       | of l2s == NULL will be false so we skip this element entirely. If
       | the element is not found the l2s == NULL is true so we do the normal
       | append to the (possibly empty) rh/rl list. Note that a pointer to
       | the head of the new conscells is kept on the mark stack to avoid
       | garbage collecting these cells inadvertantly.
       */
       push(rh);
       for(l1s = l1; l1s != NULL; l1s = l1s->cdrp) {
            for(l2s = l2; l2s != NULL; l2s = l2s->cdrp)
                if (eq(l1s->carp, l2s->carp)) break;
            if (l2s == NULL) {
                if (rh == NULL)
                    rh = rl = new(CONSCELL);
                else
                    rl = rl->cdrp = new(CONSCELL);
                rl->carp = l1s->carp;
            }
       }
       fret(rh,1);
er:    ierror("ldiff");
}

