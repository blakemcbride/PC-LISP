

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** buabs:Return the absolute value of the single double parameter.     **
 *************************************************************************/
struct conscell *buabs(form)
struct conscell *form;
{      double f;
       if ((form != NULL)&&(form->cdrp == NULL))
            if (GetFloat(form->carp,&f))
                return(newrealop(fabs(f)));
       ierror("abs");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
