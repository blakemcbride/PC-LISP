

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bustringp: (stringp expr) Return t if expr is of type string.       **
 *************************************************************************/
struct conscell *bustringp(form)
struct conscell *form;
{      if (form != NULL)
       {   if (form->cdrp == NULL)
           {    if (form->carp != NULL)
                {  if (form->carp->celltype == STRINGATOM)
                       return(LIST(thold));
                };
                return(NULL);
           };
       };
       ierror("stringp");
}
