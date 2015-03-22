/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:30) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bugc: Asks for garbage collection to occur right now please. Unlike **
 ** The mman it does not trigger allocation of new blocks if the percent**
 ** thresholds are not met.                                             **
 *************************************************************************/
struct conscell *bugc(form)
struct conscell *form;
{      extern long gccount;
       if (form == NULL) {
          marking = 1;            /* tell stkovfl we are */
          mark();                 /* doing gc so error can */
          gather(NULL,NULL);      /* be told to user. See*/
          marking = 0;            /* stkovfl() in main.c */
          if (TestForNonNil("$gcprint",0)) printstats();
          SetLongVar("$gccount$",gccount);
          return(LIST(thold));    /* return value is 't' */
       }
       ierror("gc");
}
