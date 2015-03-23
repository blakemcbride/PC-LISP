#include        <stdio.h>
#include        <ctype.h>
#include        "lisp.h"

/*
 |    (remove <exp> <list> [n])
 |  & (remq   <exp> <list> [n])
 |  & (delete <exp> <list> [n])
 |  & (delq   <exp> <list> [n])
 |
 | These built in functions will either destructively del* or non-destructively
 | rem* remove <exp> from <list> up to <n> times, or all occurrences if <n> is
 | not specified. The *q versions of the function will use 'eq' to decide if
 | two s-exprs are the same and the other versions use 'equal' to decide.
 */

/*
 | If user does not provide the N option then N_ALL is assigned by the
 | getdelparms function.
 */
# define N_ALL    MAXINT

/*
 | There are three similar primtives called 'delete' 'remove' 'delq' and
 | 'remq' That this file implementes so this 'getdelparms' routine handles
 | the parameter fetch and error checking for all of them. The parameters
 | are (<exp> <list> [<fixnum>]).
 */
static int getdelparms(form, e, l, n)
struct conscell *form, **e, **l; long int *n;
{      *n = N_ALL;
       if (form == NULL) goto er;
       *e = form->carp;
       if ((form = form->cdrp) == NULL) goto er;
       *l = form->carp;
       if ((*l != NULL)&&((*l)->celltype != CONSCELL)) goto er;
       if ((form = form->cdrp) == NULL) goto ok;
       if (form->cdrp != NULL) goto er;
       if (!GetFix(form->carp, n)) goto er;
       if (*n < 0L) goto er;
ok:    return(1);
er:    return(0);
}

/*
 | deletexp - is the workhorse of these delete primitives. It destructively
 | removes from 'l' all elements that compare equal to to 'e' using the 'cmp'
 | function. It returns the new head pointer to the list.
 */
static struct conscell *deletexp(l, e, n, cmp)
struct conscell *l, *e; int n; int (*cmp)();
{       struct conscell *h = l, *t;

       /*
        | Trivial case either empty list, or request to delete 0 elements
        | causes the list itself to be returned.
        */
        if ((h == NULL)||(n <= 0)) goto done;

       /*
        | Advance the new head 'h' through the list as long as its cars
        | compare equal to 'e'. If N ever goes non positive then return
        | this new head, our work is done.
        */
        for( ; (h != NULL) && (*cmp)(h->carp, e); h = h->cdrp)
            if (--n < 0) goto done;

       /*
        | Has the entire list has been deleted or the number of allowed deletions
        | been exhaused? If so just return NULL.
        */
        if ((h == NULL)||(n <= 0)) goto done;

       /*
        | Our new head 'h' has been established so now scan a tail forward
        | through the list unlinking elements which compare equal. We reuse
        | 'l' as the last element in the list examined so that we can alter
        | its cdrp if necessary. Note, we know n>0 initially so we track the
        | count AFTER elements are deleted thus allowing us to stop at the
        | earliest possible moment.
        */
        for(l = h, t = h->cdrp; t != NULL; t = t->cdrp) {
            if ((*cmp)(t->carp, e)) {
                l->cdrp = t->cdrp;
                if (--n <= 0) goto done;
            } else
                l = t;
        }

       /*
        | All done, just return the new head of the modified list 'h'.
        */
done:
        return(h);
}

/*
 | (delete <exp> <list> [n])
 |
 | This primitive will physically delete the first 'n' occurrences of <exp>
 | from <list>. The function 'equal' is used to do the comparison.
 */
struct conscell *budelete(form)
struct conscell *form;
{      struct conscell *e, *l;
       long int n; extern int equal();
       if (!getdelparms(form, &e, &l, &n)) ierror("delete");
       return(deletexp(l, e, (int) n, equal));
}

/*
 | (delq <exp> <list> [n])
 |
 | This primitive will physically delete the first 'n' occurrences of <exp>
 | from <list>. The function 'eq' us used to do the comparison.
 */
struct conscell *budelq(form)
struct conscell *form;
{      struct conscell *e, *l;
       long int n; extern int eq();
       if (!getdelparms(form, &e, &l, &n)) ierror("delq");
       return(deletexp(l, e, (int) n, eq));
}

/*
 | (remove <exp> <list> [n])
 |
 | This primitive will non destructively delete the first 'n' occurrences of
 | <exp> from <list>. The function 'equal' is used to do the comparison.
 */
struct conscell *buremove(form)
struct conscell *form;
{      struct conscell *e, *l;
       long int n; extern int equal();
       if (!getdelparms(form, &e, &l, &n)) ierror("remove");
       return(deletexp(topcopy(l), e, (int) n, equal));
}

/*
 | (remq <exp> <list> [n])
 |
 | This primitive will non destructively delete the first 'n' occurrences of
 | <exp> from <list>. The function 'eq' is used to do the comparison.
 */
struct conscell *buremq(form)
struct conscell *form;
{      struct conscell *e, *l;
       long int n; extern int eq();
       if (!getdelparms(form, &e, &l, &n)) ierror("remq");
       return(deletexp(topcopy(l), e, (int) n, eq));
}

