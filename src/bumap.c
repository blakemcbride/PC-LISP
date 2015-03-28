
/*************************************************************************
 **         PC-LISP (C) 1986 Peter Ashwood-Smith                        **
 **         Module Mapping.c                                            **
 ** ------------------------------------------------------------------- **
 **    This module defines the built in functions map,mapc,mapcon,mapcar**
 ** and maplist. They all basically take a function an a number of lists**
 ** of arguments to these functions and apply the function to successive**
 ** cars,cdrs etc of the list and return a list of the results. They    **
 ** differ in the function applied to generate the next set of args and **
 ** the side effect, some are destructive others are not.               **
 *************************************************************************/

#include <stdio.h>
#include "lisp.h"

/*************************************************************************
 ** bumapcar: This function takes a function as its first parameter and **
 ** some number of lists. Each list contains arguments which when pair  **
 ** wise combined with each other will form one set of arguments to be  **
 ** used in evaluating the function. For example the function :         **
 ** (mapcar '+ '(1 2) '(4 5)) Will result in (+ 1 4),(+ 2 5) all        **
 ** being evaluated and the results chained into a list (5 7)   and     **
 ** returned. To make things easier I  built the list backwards and then**
 ** reverse it afterwards. Here is an example of the list structure that**
 ** we get, this is what form points to:                                **
 **                                                                     **
 **         [*|*]--------------->[*|*]---------------->[*|/]            **
 **          |                    |                     |               **
 **          v                    v                     v               **
 **          [atom '+']          [*|*]--->[*|/]        [*|*]--->[*|/]   **
 **                               |        |            |        |      **
 **                               v        v            v        v      **
 **                               1.0      2.0          4.0     5.0     **
 **                                                                     **
 ** The way that we evaluate this beast is as follows. First we extract **
 ** the function that's put in 'func'. Next we take a top level copy of **
 ** the list of list or arguments. We then take a copy of this and turn **
 ** it into a list of arguments. The first element in each sub list.    **
 ** Thats the first while loop traversing 'largs' and copying the caar  **
 ** to the car. Then we built a new element of our result list whose    **
 ** car is the result of applying the function to the arg list largs.   **
 ** We then modify the 'work' list by advancing all the car pointer to  **
 ** point one down the list. Ie from 1 to 2 and from 4 to 5. We then do **
 ** the same thing all over again. We repeat this until there are no    **
 ** more args in the first list. We then reverse the list we were build-**
 ** ing because it is backwards and return it. During this construction **
 ** the work list and the constructed list of results must be stacked   **
 ** incase the garbage collection gets activated.                       **
 *************************************************************************/
struct conscell *bumapcar(form)
struct conscell *form;
{      struct conscell *work,*func,*last,*temp,*largs,*t;
       if ((form!=NULL)&&(form->carp != NULL))
       {   func = form->carp;
           push(work); push(last); push(t); push(largs);
           work = topcopy(form->cdrp);
           if (work != NULL)
           {   last = NULL;
               while(work->carp != NULL)
               {   temp = largs = topcopy(work);
                   while(temp != NULL)
                   {     if (temp->carp != NULL) {
                             if (temp->carp->celltype != CONSCELL) goto er;
                             temp->carp = temp->carp->carp;
                         }
                         temp = temp->cdrp;
                   };
                   t = new(CONSCELL);
                   t->carp = apply(func,largs);
                   t->cdrp = last;
                   last = t;
                   temp = work;
                   while(temp != NULL)
                   {     if (temp->carp != NULL) {
                             if (temp->carp->celltype != CONSCELL) goto er;
                             temp->carp = temp->carp->cdrp;
                         }
                         temp = temp->cdrp;
                   }
                }
                xret(reverse(last),4);
           }
       }
  er:  ierror("mapcar");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}

/*************************************************************************
 ** bumapc:   This function takes a function as its first parameter and **
 ** some number of lists. Each list contains arguments which when pair  **
 ** wise combined with each other will form one set of arguments to be  **
 ** used in evaluating the function. For example the function :         **
 ** (mapc   '+ '(1 2) '(4 5)) Will result in (+ 1 4),(+ 2 5) all        **
 ** being evaluated and the result of the first evaluation 5 being      **
 ** returned. Here is the type of structure 'form' points to:           **
 **                                                                     **
 **         [*|*]--------------->[*|*]---------------->[*|/]            **
 **          |                    |                     |               **
 **          v                    v                     v               **
 **          [atom '+']          [*|*]--->[*|/]        [*|*]--->[*|/]   **
 **                               |        |            |        |      **
 **                               v        v            v        v      **
 **                               1.0      2.0          4.0     5.0     **
 **                                                                     **
 ** The way that we evaluate this beast is as follows. First we extract **
 ** the function that's put in 'func'. Next we take a top level copy of **
 ** the list of list or arguments. We then take a copy of this and turn **
 ** it into a list of arguments. The first element in each sub list.    **
 ** Thats the first while loop traversing 'largs' and copying the caar  **
 ** to the car. Then we built a new element of our result list whose    **
 ** car is the result of applying the function to the arg list largs.   **
 ** We then modify the 'work' list by advancing all the car pointer to  **
 ** point one down the list. Ie from 1 to 2 and from 4 to 5. We then do **
 ** the same thing all over again. We repeat this until there are no    **
 ** more args in the first list.                                        **
 *************************************************************************/
struct conscell *bumapc(form)
struct conscell *form;
{      struct conscell *work,*func,*temp,*largs,*ret;
       if ((form!=NULL)&&(form->carp != NULL))
       {   func = form->carp;
           push(work); push(ret); push(largs);
           work = topcopy(form->cdrp);
           if (work != NULL)
           {   ret = work->carp;
               while(work->carp != NULL)
               {   temp = largs = topcopy(work);
                   while(temp != NULL)
                   {     if (temp->carp != NULL) {
                             if (temp->carp->celltype != CONSCELL) goto er;
                             temp->carp = temp->carp->carp;
                         }
                         temp = temp->cdrp;
                   }
                   apply(func,largs);
                   temp = work;
                   while(temp != NULL)
                   {     if (temp->carp != NULL) {
                             if (temp->carp->celltype != CONSCELL) goto er;
                             temp->carp = temp->carp->cdrp;
                         }
                         temp = temp->cdrp;
                   }
                }
           }
           fret(ret,3);
       }
  er:  ierror("mapc");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}

/*************************************************************************
 ** bumaplist: This function is like mapcar except that successive cdrs **
 ** are taken of the arg lists rather than successive cars. There is a  **
 ** single change in the code of the above mapcar routine to do a cdr.  **
 *************************************************************************/
struct conscell *bumaplist(form)
struct conscell *form;
{      struct conscell *func,*last,*temp,*scan,*t;
       if ((form!=NULL)&&(form->carp != NULL))
       {   func = form->carp;
           push(temp); push(last); push(t);
           temp = form->cdrp;
           if (temp != NULL)
           {   last = NULL;
               while(temp->carp != NULL)
               {   t = new(CONSCELL);
                   t->carp = apply(func,temp);
                   t->cdrp = last;
                   last = t;
                   scan = temp = topcopy(temp);
                   while(scan != NULL)
                   {     if (scan->carp != NULL) {
                            if (scan->carp->celltype != CONSCELL) goto er;
                            scan->carp = scan->carp->cdrp;
                         }
                         scan = scan->cdrp;
                   }
                }
                xret(reverse(last),3);
           }
       }
 er:   ierror("maplist");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}

/*************************************************************************
 ** Map - like mapc but it does cdr's down the list. Have a look at the **
 ** mapc code.                                                          **
 *************************************************************************/
struct conscell *bumap(form)
struct conscell *form;
{      struct conscell *work,*func,*temp,*ret;
       if ((form!=NULL)&&(form->carp != NULL))
       {   func = form->carp;
           push(work); push(ret);
           work = form->cdrp;
           if (work != NULL)
           {   ret = work->carp;
               while(work->carp != NULL)
               {   apply(func,work);
                   work = temp = topcopy(work);
                   while(temp != NULL)
                   {     if (temp->carp != NULL) {
                             if (temp->carp->celltype != CONSCELL) goto er;
                             temp->carp = temp->carp->cdrp;
                         }
                         temp = temp->cdrp;
                   }
               }
           }
           fret(ret,2);
       }
  er:  ierror("map");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
