

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** bugetenv: Will return the value of the environment variable 'x'     **
 *************************************************************************/
struct conscell *bugetenv(form)
struct conscell *form;
{      extern char *getenv(); char *val,*s;
       if ((form != NULL)&&(form->cdrp == NULL))
       {  if (GetString(form->carp,&s))
          {  if ((val = getenv(s)) == NULL) return(NULL);
             if (strlen(val) <= MAXATOMSIZE)
                return(LIST(CreateInternedAtom(val)));
          };
       };
       ierror("getenv");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
