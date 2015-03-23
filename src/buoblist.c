

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** buoblist() Will return a list of objects. We just call the function **
 ** CopyOblist() in main.c to get a copy of the atom hash table.        **
 *************************************************************************/
struct conscell *buoblist(form)
struct conscell *form;
{      if (form == NULL)                        /* check zero parameters */
           return(CopyOblist());
       ierror("oblist");                        /* more than zero parms */
}
