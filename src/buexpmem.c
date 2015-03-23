/* -- */

/*
 | PC-LISP (C) 1989-1992 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (expand-memory +cells+ +alpha+ +heap+). Will expand the memory pools**
 ** for cells by +cells+ bytes, for alpha by +alpha+ bytes and the heap **
 ** by +heap+ bytes. The actual amounts are rounded up to the nearest   **
 ** block so the exact byte counts are not honored.                     **
 *************************************************************************/
struct conscell *buexpandmemory(form)
struct conscell *form;
{
       long int a, c, h;

      /*
       | Extract the three fixnum arguments.
       */
       if ((form == NULL)||(!GetFix(form->carp, &c))) goto er;
       form = form->cdrp;
       if ((form == NULL)||(!GetFix(form->carp, &a))) goto er;
       form = form->cdrp;
       if ((form == NULL)||(!GetFix(form->carp, &h))) goto er;
       if (form->cdrp != NULL) goto er;

      /*
       | If we are able to allocate the requested memory or at least some
       | memory then return t.
       */
       if (liexpmem(a,c,h) == 0) return(LIST(thold));

      /*
       | Unable to allocate the memory for some reason, return NULL.
       */
       return(NULL);

  er:  ierror("expand-memory");
}
