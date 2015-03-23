

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** buor:  Logically or all the parameters we have. any t return t      **
 *************************************************************************/
struct conscell *buor(form)
struct conscell *form;
{      struct conscell *r;
       if (form != NULL)
       {   do
           {  r = eval(form->carp);
              if (r!=NULL)
                 return(r);
              form = form->cdrp;
           }
           while(form != NULL);
           return(NULL);
       };
       return((struct conscell *)thold);
}
