

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (drain [port]) will flush the 'port' or the standard output port if **
 ** port is not given.                                                  **
 *************************************************************************/
struct conscell *budrain(form)
struct conscell *form;
{      if (form == NULL)
           fflush(stdout);
       else
       {   struct conscell *car;
           if (((car = form->carp) == NULL)||(form->cdrp != NULL)) goto ERR;
           if (car->celltype != FILECELL) goto ERR;
           fflush(PORT(car)->atom);
       };
       return(LIST(thold));
  ERR: ierror("drain");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
