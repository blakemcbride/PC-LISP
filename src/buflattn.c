/* */
/*
 | PC-LISP (C) 1984-1990 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/* history

   ms:25aug92
   - Added an additional test to skip out nested nils. PRS62474 & PRS61686.


*/



/*
 | Given an input list create a flat copy of it which is then returned. The
 | tail of this returned list is also returned.
 */
static struct conscell *flatcopy(l, ttail)
       struct conscell *l, **ttail;
{
       struct conscell *tmp, *head, *tail, *ntail;

      /*
       | If nothing in list then return nothing and set tail ptr to NULL.
       */
       if (l == NULL) { *ttail = NULL; return(NULL); }

      /*
       | Traverse the list building up a list head->...->tail as we go. When
       | hit nil elements just skip them as flattening them should cause a
       | splice into the list. If we hit a non conscell element then we will
       | allocate a new conscell for the copy and append it to the tail of
       | the return list. When we hit a conscell element we must get a flat
       | copy of its contents. Which we append to the return list and set
       | its tail to be our tail.
       */
       push(head);
       for(tail = head = NULL; l != NULL; l = l->cdrp) {
	   if (l->celltype != CONSCELL) {                 /* dotted pairs are */
	       tmp = new(CONSCELL);                       /* appended to the end */
	       tmp->cdrp = NULL;
	       tmp->carp = l;                             /* of the list after listifying */
	       tail = tail->cdrp = tmp;
	       continue;
	   }
	   if (l->carp == NULL) continue;                 /* nils are skipped (spliced) */

	   if (l->carp->celltype != CONSCELL) {           /* other non lists are copied */
	       tmp = new(CONSCELL);
	       tmp->cdrp = NULL;
	       tmp->carp = l->carp;
	       if (head)
		  tail = tail->cdrp = tmp;                /* and then appended */
	       else
		  head = tail = tmp;
	   } else {                                       /* finally lists are recursively */

	       tmp = flatcopy(l->carp, &ntail);           /* flat copied and then appended */

               if (tmp != NULL) {                         /* ms250892: skip nested nils */
	          if (head)
	    	      tail->cdrp = tmp;                      /* to end of output list */
	          else
		      head = tmp;                            /* tail about to be set */
	          tail = ntail;                              /* set tail to new one */
               }

	   }
       }

      /*
       | All done so return the head of the list and its tail.
       */
       *ttail = tail;
       fret(head,1);
}

/*
 | An efficient flattening routine. Will take a list like '( a (b c) (d e) f g)
 | and return '(a b c d e f g). This routine breaks its input into two parts: The
 | last_tail which is the last list in the input list whose car is a list. In
 | the example that would be the list ((d e) f). The cdr of the last_tail is then
 | a part of the input list that does NOT require flattening or copying and can
 | be spliced onto the flattened front part of the list and still producing a
 | correct answer. This also allows us to make a quick check to see if the list
 | is already flat by seeing if last_tail == l and if so we just return without
 | doing any work at all. Note that we temporarily set the last_tail->cdrp to
 | NULL so that the input is split into '(a (b c) (d e)) and '(f g)) of which
 | the former is passed to flatcopy and the latter is then appened to. The
 | original last_tail->cdrp is then reset so that the original input list is
 | left intact.
 */
static struct conscell *flatten(l)
       struct conscell *l;
{
       struct conscell *last_tail, *tail;
       struct conscell *tmp, *prev;

      /*
       | Look for the last flat list in the input list or the first non list
       | on the cdr (dotted pair). Initially it is the head of the list.
       */
       prev = last_tail = NULL;
       for(tail = l; tail != NULL; tail = tail->cdrp) {
	   if (tail->celltype != CONSCELL) { last_tail = prev; break; }
	   if ((tail->carp == NULL)||(tail->carp->celltype == CONSCELL)) last_tail = tail;
           prev = tail;
       }

       if (last_tail == NULL) return(l);        /* if already flat just return input */
       if ((last_tail->carp != NULL) &&         /* if stopped due to dotted pair at end and already flat */
           (last_tail->carp->celltype != CONSCELL)) return(l);  /* just return input */
       push(tmp);                               /* mark for GC */
       tmp = last_tail->cdrp;                   /* temporarily terminate head list */
       last_tail->cdrp = NULL;                  /* remembering old value first */
       l = flatcopy(l, &tail);                  /* copy l flat and return l=head, tail=tail */
       if (tail) tail->cdrp = tmp;              /* reset tail to point to flat tail of original list */
       last_tail->cdrp = tmp;                   /* restore original input list */
       xpop(1);                                 /* unmark for GC */
       return(l ? l : tmp);                     /* return flat copy of head unless nil otherwise tail */
}

/*
 |  list <- (flatten list)
 |  ~~~~~~~~~~~~~~~~~~~~~~
 |  This function will return a copy of the input list flattened to the
 |  leaves.
 */
struct conscell *buflatten(form)
       struct conscell *form;
{
       if (form == NULL) goto er;
       if (form->cdrp != NULL) goto er;
       if ((form = form->carp) == NULL) return(NULL);
       if (form->celltype != CONSCELL) goto er;
       return(flatten(form));
er:    ierror("flatten");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}

