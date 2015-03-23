

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** bulistp : Given a parameter return 't' if a list or 'nil otherwise. **
 *************************************************************************/
struct conscell *bulistp(form)
struct conscell *form;
{      if ((form != NULL)&&(form->cdrp == NULL))
       {   if ((form->carp == NULL)||(form->carp->celltype == CONSCELL))
               return(LIST(thold));
           else
               return(NULL);
       };
       ierror("listp");
}
