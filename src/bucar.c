

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** The built in car function. The list of paramters are in 'form' if   **
 ** we are given one list as a parameter we return the head or car of   **
 ** the list: eg (car (a b c d)) is 'a'  . Note the CHAINATOM option.   **
 *************************************************************************/
struct conscell *bucar(form)
struct conscell *form;
{      if ((form != NULL)&&(form->cdrp == NULL))
       {   form = form->carp;
           if (form == NULL)
               return(NULL);
           if (form->celltype == CONSCELL)
               return(form->carp);
           if (GetOption(CHAINATOM))
               return(NULL);
       };
       ierror("car");
}
