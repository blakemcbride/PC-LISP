

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** bufloatp:Given a parameter return 't' if a double or 'nil otherwise.**
 *************************************************************************/
struct conscell *bufloatp(form)
struct conscell *form;
{
       if ((form != NULL)&&(form->cdrp == NULL)) {
           if ((form->carp != NULL) && (form->carp->celltype == REALATOM))
               return(LIST(thold));
           else
               return(NULL);
       }
       ierror("floatp");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
