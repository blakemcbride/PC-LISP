

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bufixp:(fixp expr) Return t expr is a fixnum, nil otherwise.        **
 *************************************************************************/
struct conscell *bufixp(form)
struct conscell *form;
{      if (form != NULL)
       {   if (form->cdrp == NULL)
           {    if ((form->carp != NULL)&&(form->carp->celltype == FIXATOM))
                    return(LIST(thold));
                return(NULL);
           };
       };
       ierror("fixp");
}
