

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bulessp (lessp -numbers-) Return t if the numbers are strictly in   **
 ** increasing order from left to right.                                **
 *************************************************************************/
struct conscell *bulessp(form)
struct conscell *form;
{      double val,op; long temp;
       if (form == NULL) return(LIST(thold));
       if (!GetFloat(form->carp,&val)) goto ERR;
       form = form->cdrp;
       while(form!=NULL)
       {    if (!GetFloat(form->carp,&op)) goto ERR;
            if (op <= val) return(NULL);
            val = op;
            form = form->cdrp;
       };
       return(LIST(thold));
ERR:   ierror("lessp");
}
