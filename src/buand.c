

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** buand: Logically and all the parameters we have. any nulls return nil*
 *************************************************************************/
struct conscell *buand(form)
struct conscell *form;
{      struct conscell *r;
       if (form != NULL)
       {   do
           {  r = eval(form->carp);
              if (r==NULL)
                 return(NULL);
              form = form->cdrp;
           }
           while(form != NULL);
           return(r);
       };
       return((struct conscell *)thold);
}
