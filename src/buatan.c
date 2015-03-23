

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** buatan:Returns the arc tangent of the double / double parameters.   **
 *************************************************************************/
struct conscell *buatan(form)
struct conscell *form;
{      double op1, op2;
       if (form != NULL)
       {   if (GetFloat(form->carp,&op1))
           {   form = form->cdrp;
               if (form != NULL)
               {   if (GetFloat(form->carp,&op2))
                       return(newrealop(atan2(op1,op2)));
               };
           };
       };
       ierror("atan");
}
