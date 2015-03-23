

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (nconc l1 l2 .. ln) Will physically join l2 to l1, l3 to l2 etc...  **
 ** by changing the cdr pointer of each of the end cells in l1...ln-1   **
 ** to point to the next argument. We advance 'form' through the arg    **
 ** list keeping 'last' as the arg prior to 'form'. Each time we advance**
 ** we scan through 'last' til we reach the end cons cell. We then join **
 ** the 'form' argument to it. We have to do some contortions to avoid  **
 ** the possibility of 'nil' screwing us up. Note we scan past all the  **
 ** nil args to find the first value for 'last'. Then we we reset last  **
 ** we check that we are not setting it to 'nil'.  We also test for the **
 ** user break here because it is possible to nconc a list to itself and**
 ** thus make a circular list whose end will never be found by the two  **
 ** while loops in this routine. We want then to be able to break out.  **
 *************************************************************************/
struct conscell *bunconc(form)
struct conscell *form;
{      struct conscell *last,*save;
       while((form != NULL) && (form->carp == NULL))
       {   form = form->cdrp;
           TEST_BREAK();
       };
       if (form == NULL)
           return(NULL);
       last = (save = form->carp);
       for(;;)
       {   form = form->cdrp;
           if (form == NULL) break;
           while((last->celltype == CONSCELL)&&(last->cdrp != NULL))
           {   last = last->cdrp;
               TEST_BREAK();
           };
           if (last->celltype != CONSCELL)
               ierror("nconc");
           if ((last->cdrp = form->carp) != NULL)
               last = last->cdrp;
       };
       return(save);
}
