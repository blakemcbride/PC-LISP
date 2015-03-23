

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** built in cdr function will when given one nonempty list as a parm   **
 ** return the cdr or rest of the list. eg: (cdr (a b c d)) is (b c d)  **
 ** Note if (sstatus chainatom) is set then we allow the error of taking**
 ** the car of an object that is not a cons cell or a nil cell.         **
 *************************************************************************/
struct conscell *bucdr(form)
struct conscell *form;
{      if ((form != NULL)&&(form->cdrp == NULL))
       {   form = form->carp;
           if (form == NULL)
               return(NULL);
           if (form->celltype == CONSCELL)
               return(form->cdrp);
           if (GetOption(CHAINATOM))
               return(NULL);
       };
       ierror("cdr");
}
