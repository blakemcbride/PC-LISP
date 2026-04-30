

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

struct conscell * bunot(struct conscell *form)
{      if ((form != NULL)&&(form->cdrp == NULL))
       {  if (form->carp == NULL)
               return((struct conscell *)thold);
          else
               return(NULL);
       };
       ierror("not");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
