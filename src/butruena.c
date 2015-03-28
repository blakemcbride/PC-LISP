

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (truename port) Will return the atom associated with stream port.   **
 ** This is just the ascii name under which port was opened.            **
 *************************************************************************/
struct conscell *butruename(form)
struct conscell *form;
{      if ((form != NULL)&&(form->carp != NULL)&&(form->cdrp == NULL))
       {    if (form->carp->celltype == FILECELL)
                return(LIST(PORT(form->carp)->fname));
       };
       ierror("truename");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
