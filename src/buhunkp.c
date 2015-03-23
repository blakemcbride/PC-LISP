

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buhunkp (hunkp s) Returns t if s is a Hunk otherwise it returns nil **
 *************************************************************************/
struct conscell *buhunkp(form)
struct conscell *form;
{      if ((form != NULL)&&(form->cdrp == NULL))
       {   if (form->carp != NULL)
           {   if (form->carp->celltype == HUNKATOM)
                   return(LIST(thold));
           };
           return(NULL);
       };
       ierror("hunkp");
}

