

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** buhashstat() Will return a list of the number of bucket entries for **
 ** each table slot in the atom hash table. We call the main.c routine  **
 ** HashStatus() to get this info for us.                               **
 *************************************************************************/
struct conscell *buhashstat(form)
struct conscell *form;
{      if (form == NULL)                        /* check zero parameters */
           return(HashStatus());
       ierror("hashtabstat");                   /* more than zero parms  */
}
