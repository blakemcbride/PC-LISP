

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include        <stdio.h>
#include        <math.h>
#include        "lisp.h"

/*************************************************************************
 ** bumemstat(): return a list of the percentages of cell,alpha, and    **
 ** heap space and the total amount of cell, alpha and heap space avail **
 ** able. We call the memory manager to get this information. The call  **
 ** memorystatus(n) returns the appropriate info for 'n' = 0,1,2,3,4 & 5**
 *************************************************************************/
struct conscell *bumemstat(form)
struct conscell *form;
{      struct conscell *t,*h;
       push(t); push(h);
       if (form==NULL)
       {
           t = new(CONSCELL);
           t->carp = newintop((long)memorystatus(5));   /* total heap bytes */
           h = t;

           t = new(CONSCELL);
           t->carp = newintop((long)memorystatus(4));   /* total alpha bytes */
           t->cdrp = h;
           h = t;

           t = new(CONSCELL);
           t->carp = newintop((long)memorystatus(3));   /* total cell bytes */
           t->cdrp = h;
           h = t;

           t = new(CONSCELL);
           t->carp = newintop((long)memorystatus(2));   /* percent heap used */
           t->cdrp = h;
           h = t;

           t = new(CONSCELL);
           t->carp = newintop((long)memorystatus(1));   /* percent alpha used */
           t->cdrp = h;
           h = t;

           t = new(CONSCELL);
           t->carp = newintop((long)memorystatus(0));   /* percent cell used */
           t->cdrp = h;

           fret(t,2);
       }
       ierror("memstat");
}
