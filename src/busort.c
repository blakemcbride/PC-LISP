

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** TestFunction(f,flag,k1,k2) Will compare k1 with k2 and return '1'.  **
 ** If the test is true. If flag is '1' both the car of k1 and k2 are   **
 ** compared otherwise k1 and k2 are compared. If f is NULL then we test**
 ** for k1 less than k2 alphabetically. Otherwise we test using the     **
 ** function 'func' we apply it to the arg list (k1 k2) and if we get   **
 ** a non nil back we return '1' otherwise we return '0'.               **
 *************************************************************************/
int TestFunction(f,flag,k1,k2)
struct conscell *f,*k1,*k2;
int flag;
{   char *s1,*s2;
    struct conscell *n,*apply();
    if (flag)
    {   if (k1 != NULL)
           if (k1->celltype == CONSCELL) k1 = k1->carp; else goto ER;
        if (k2 != NULL)
           if (k2->celltype == CONSCELL) k2 = k2->carp; else goto ER;
    };
    if (f == NULL)                                      /* simple case */
    {   if (GetString(k1,&s1) && GetString(k2,&s2))
            return(strcmp(s1,s2) < 0);
        goto ER;
    };                                                  /* user func case */
    push(n); xpush(k1); xpush(k2);
    n = new(CONSCELL);                                  /* make parm list */
    n->cdrp = new(CONSCELL);
    n->carp = k1; n->cdrp->carp = k2;                   /* put args in list */
    n = apply(f,n);                                     /* call function */
    xpop(3);                                            /* un GC protect */
    return(n != NULL);
ER: ierror("sort|sortcar");
}

/*************************************************************************
 ** partition(l1,mid,l2,l,f,flag) Will partition the list l into 2 lists**
 ** l1 and l2 and one element 'mid' whose values are such that keys in  **
 ** l1 are less than or equal to mid, and keys in l2 are greater or eq  **
 ** to mid. Mid is chosen as the first element in l. We call TestFunct  **
 ** to do the testing work. Flag is passed down to decide on caring.    **
 ** If the entire partition is equal then all data mid and l2 contain   **
 ** the partition and we return 0 to indicate that there is only 1 valid**
 ** partition. The top level sort will then avoid the recursive calls to**
 ** sort in this case hence saving enourmouns amounts of time when there**
 ** are many duplicates in a list.                                      **
 *************************************************************************/
partition(l1,mid,l2,l,f,flag)
struct conscell **l1,**mid,**l2,*l,*f;
int flag;
{   struct conscell *key,*ol1,*ol2,*temp; int allEq = 1;
    if (l->cdrp == NULL) {                                      /* if trivial list just return now with mid point */
       *l1 = *l2 = NULL;
       *mid = l;
       return(0);                                               /* there are no partitions */
    }
    push(ol1); push(ol2); push(key); push(temp); xpush(l);      /* mark variables for garbage collection */
    key = l;                                                    /* chose first element as the mid point of partition */
    for(l = l->cdrp; l != NULL; l = temp)  {                    /* iterate over all other elements in list 'l' */
        temp = l->cdrp;
        if (TestFunction(f,flag,l->carp,key->carp)) {           /* compare element against key for < (NB GC may occur here) */
           l->cdrp = ol1; ol1 = l;                              /* it was less than so add to list l1 */
           allEq = 0;                                           /* l->carp < key->carp ==> they are not all equal ! */
        }  else {                                               /* l->carp >= key->carp */
           if (allEq) {                                         /* are all previous elements Eq to key? */
               if (flag)                                        /* if sortcar'ing */
                   allEq = equal(l->carp->carp,key->carp->carp);/* they are allEq if l->carp->carp == key->carp->carp */
               else                                             /* if not sortcar'ing ie sort'ing */
                   allEq = equal(l->carp,key->carp);            /* they are allEq if l->carp == key->carp */
           }
           l->cdrp = ol2; ol2 = l;                              /* it was greater than or equal to so add to list l2 */
        }
    }
    xpop(5);
    *l1 = ol1; *l2 = ol2; *mid = key;
    return(!allEq);                                             /* return TRUE if two partitions */
}

/*************************************************************************
 ** sort(list,func,carflag) Will run a quicksort algorighm on the list  **
 ** 'list' to put it in sorted order. The comparison function 'func' is **
 ** used, or alphabetical ordering if func is NULL. Carflag if '1' will **
 ** cause the keys to be found by car'ing one extra time. This allows   **
 ** lists of lists to be sorted based on the head of the list as a key. **
 ** First we call partation which will delete all elements less than a  **
 ** key from list 'l1' and put them in a list called 'l2'. Now we have  **
 ** the all(l1) < all(l2) and can recurse and  sort the two halves then **
 ** append the list of larger elements to the list of smaller elements. **
 ** and return this result. All of this stuff destroys top level list.  **
 ** Quicksort behaves very badly when the partition in question has many**
 ** duplicates in it. In particular if the entire partition is the same **
 ** the performance goes very rapidly to n^2 so our partition call will **
 ** detect a partition which is completely equal.                       **
 *************************************************************************/
struct conscell *sort(l,f,flag)
struct conscell *l,*f;
int flag;
{   struct conscell *l1,*l2,*mid;
    if ((l == NULL)||(l->cdrp == NULL)) return(l);      /* base case */
    if (l->celltype!=CONSCELL) ierror("sort|sortcar");
    push(l1); push(l2); push(mid);                      /* GC protect */
    if (partition(&l1,&mid,&l2,l,f,flag)) {             /* partition and if not all equal */
        l1 = sort(l1,f,flag); l2 = sort(l2,f,flag);     /* divide&conquer */
        mid->cdrp = l2;                                 /* join mid to tail */
        xpop(3);                                        /* GC unprotect */
        if (l1 == NULL) return(mid);                    /* check null head */
        for(l=l1; l->cdrp != NULL; l=l->cdrp);          /* scan to end of l1 */
        l->cdrp = mid;                                  /* append l2 to mid*/
        return(l1);                                     /* head+mid+tail */
    } else {                                            /* nothing to partition all were equal */
        mid->cdrp = l2;                                 /* so (mid, l2...) contains entire partition */
        xpop(3);                                        /* pop the GC protected vars */
        return(mid);                                    /* return entire partition unsorted */
    }
}

/*************************************************************************
 ** (sort list function) Will destructively sort the list 'list' based  **
 ** on the comparisson function 'function'. If function is 'nil' it will**
 ** use alphabetical order. We call a quick sort algorithm to do it.    **
 ** The sort routine takes a third parameter 0 or 1 which if 1 causes   **
 ** the sort to behave like sortcar ie it cars to get the keys.         **
 *************************************************************************/
struct conscell *busort(form)
struct conscell *form;
{     struct conscell *list,*func;
      if ((form != NULL)&&(form->cdrp != NULL))
      {   list = form->carp;
          if ((list == NULL)||(list->celltype == CONSCELL))
          {    form = form->cdrp;
               if ((form != NULL)&&(form->cdrp == NULL))
               {    func = form->carp;
                    return(sort(list,func,0));
               };
          };
      };
      ierror("sort");
}

/*************************************************************************
 ** (sortcar list function) Destructively sort the list 'list' based    **
 ** on the comparisson function 'function'. If function is 'nil' it will**
 ** use alphabetical order. We call a quick sort algorithm to do it.    **
 ** The sort routine takes a third parameter 0 or 1 which if 1 causes   **
 ** the sort to behave like sortcar ie it cars to get the keys.         **
 *************************************************************************/
struct conscell *busortcar(form)
struct conscell *form;
{     struct conscell *list,*func;
      if ((form != NULL)&&(form->cdrp != NULL))
      {   list = form->carp;
          if ((list == NULL)||(list->celltype == CONSCELL))
          {    form = form->cdrp;
               if ((form != NULL)&&(form->cdrp == NULL))
               {    func = form->carp;
                    return(sort(list,func,1));
               };
          };
      };
      ierror("sortcar");
}

