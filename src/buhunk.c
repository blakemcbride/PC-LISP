

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buhunk: (hunk e1 e2 ... eN) Turn the list (e1 ... eN) into a hunk of**
 ** N elements. Just call the ListToHunk function with the parm list.   **
 *************************************************************************/
struct conscell *buhunk(form)
struct conscell *form;
{      if (form != NULL)
           return(LIST(ListToHunk(form)));
       ierror("hunk");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}

