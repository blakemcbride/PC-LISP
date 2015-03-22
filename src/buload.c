/* EDITION AC01, APFUN PAS.765 (91/12/10 16:57:44) -- CLOSED */                 
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** buload: Will load the file which is given as a parameter. We just   **
 ** open the file and call the function 'loadfile' in main.c            **
 *************************************************************************/
struct conscell *buload(form)
struct conscell *form;
{      char *fname;
       if ((form != NULL)&&(GetString(form->carp,&fname)))
       {   if (form->cdrp == NULL)
               return(loadfile(fname) ? LIST(thold) : NULL);
       };
       ierror("load/include");
}
