/* EDITION AB01, APFUN MR.68 (90/04/18 09:24:08) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** zapee - the target port of a (zapline) call. Whenever any input is  **
 ** done. zapee is set to the port being read. Then when (zapline) is   **
 ** called it can figure out the target port.                           **
 *************************************************************************/
FILE *zapee = stdin;

/*************************************************************************
 ** (zapline) will read all characters up to and including a '\n' on the**
 ** last port that was used for input. This last port is kept in zapee. **
 *************************************************************************/
struct conscell *buzapline(form)
struct conscell *form;
{      if (form != NULL) ierror("zapline");
       while(getc(zapee)!='\n') TEST_BREAK();     /* zapee declared at top */
       return(NULL);
}
