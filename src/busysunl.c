/* EDITION AB01, APFUN MR.68 (90/04/18 09:24:04) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (sys:unlink file) Will unlink the file 'file' and return the value  **
 ** returned by the system command unlink().                            **
 *************************************************************************/
struct conscell *busysunlink(form)
struct conscell *form;
{      extern int unlink(); char *name;
       if ((form != NULL)&&(form->cdrp == NULL))
       {    if (GetString(form->carp,&name))
                return(newintop((long)unlink(name)));
       };
       ierror("sys:unlink");
}
