

/*
 | PC-LISP (C) 1989-1992 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (declare .....) is just ignored by the interpreter.                 **
 *************************************************************************/
struct conscell *budeclare(form)
struct conscell *form;
{      return(LIST(thold));
}
