

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bumacroexpand: This function will expand all macros in its single   **
 ** argument. It descends the list structure looking for things of the  **
 ** form (macroname .....). When it gets one it expands it. The actual  **
 ** expansion is done by 'macroexpand'. This procedure is just a driver.**
 *************************************************************************/
struct conscell *bumacroexpand(form)
struct conscell *form;
{      if ((form != NULL)&&(form->cdrp == NULL))
           return(macroexpand(form->carp));
       ierror("macroexpand");
}

