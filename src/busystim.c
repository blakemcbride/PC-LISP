

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "lisp.h"

/*************************************************************************
 ** (sys:time) Will return the time in seconds since the OS's idea of   **
 ** creation. On UNIX this is Jan 1 GMT, on MS-DOS it is Jan 01 1980    **
 ** local time.                                                         **
 *************************************************************************/
struct conscell *busystime(form)
struct conscell *form;
{      if (form == NULL)
           return(newintop(time(NULL)));
       ierror("sys:time");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
