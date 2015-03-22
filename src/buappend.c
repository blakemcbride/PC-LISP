/* EDITION AC01, APFUN PAS.765 (91/12/10 16:57:44) -- CLOSED */                 
/* --- */

/*
 | PC-LISP (C) 1989-1992 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** append(l1,l2) : will return a pointer to a list which is a copy of  **
 ** l1 with l2 following it. We do not need to copy l2 because we are   **
 ** not changing it.                                                    **
 *************************************************************************/
static struct conscell *append(l,l2)
struct conscell *l,*l2;
{      struct conscell *first,*last,*s;
       if (l2 == NULL) return(l);                /* nothing to append ? */
       if (l != NULL) {
           xpush(l); xpush(l2); push(first);
           first = last = new(CONSCELL);
           last->carp = l->carp;
           l = l->cdrp;
           while(l != NULL) {
                 s = new(CONSCELL);
                 s->carp = l->carp;
                 last->cdrp = s;
                 last = s;
                 l = l->cdrp;
           }
           last->cdrp = l2;
           fret(first,3);
       }
       fret(l2,3);
}

/*************************************************************************
 ** (append) Will take a list of lists and return the result of appe-   **
 ** ding them all. It is important to note that all but the last list   **
 ** require copying. A simple way to do this is to recursively append   **
 ** the lists together but this means that very large lists of lists    **
 ** cannot be appended together (i.e. recursive limit). To get around   **
 ** this problem we operate non recursively on a reversed version of the**
 ** list. The algorithm reverses its input then appends the lists       **
 ** together backwards. Finally it reverses its input list again and    **
 ** returns the constructed list. For example:                          **
 **                                                                     **
 **  #1   form = ( (a b c) (d e f) (g h i) )                            **
 **  #2   form = ( (g h i) (d e f) (a b c) )                            **
 **  #3   l    = (g h i)                                                **
 **  #4   l    = append((d e f), l)                                     **
 **  #5   l    = append((a b c), l)                                     **
 **  #6   form = ( (a b c) (d e f) (g h i) )                            **
 *************************************************************************/
struct conscell *buappend(form)
       struct conscell *form;
{      struct conscell *l,*t;
       if (form == NULL) return(NULL);
       form = nreverse(form);                           /* temporarily reverse for processing */
       xpush(form); xpush(l);
       l = form->carp;
       for(t = form->cdrp; t != NULL; t = t->cdrp)
           if (t->carp != NULL)
               l = append(t->carp, l);
       nreverse(form);
       xpop(2);
       return(l);
}

