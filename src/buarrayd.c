

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (arraydims array) Returns the dimension list of the array 'array'   **
 *************************************************************************/
struct conscell *buarraydims(form)
struct conscell *form;
{      struct conscell *n; struct arraycell *temp;
       if ((form != NULL)&&(form->cdrp == NULL))
       {   if (ExtractArray(form->carp,&temp))
           {  n = new(CONSCELL);
              n->carp = LIST(thold);
              n->cdrp = temp->info->cdrp;
              return(n);
           };
       };
       ierror("arraydims");
}
