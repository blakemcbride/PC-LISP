

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bunumbp(numbp expr) Return t expr is a fixnum or flonum.            **
 *************************************************************************/
struct conscell *bunumbp(form)
struct conscell *form;
{      if (form != NULL)
       {   if (form->cdrp == NULL)
           {    if (form->carp != NULL)
                {  switch(form->carp->celltype)
                   {  case REALATOM : return(LIST(thold));
                      case FIXATOM  : return(LIST(thold));
                   };
                };
                return(NULL);
           };
       };
       ierror("numbp|numberp");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
