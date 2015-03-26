

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** bualphp() Alpha less than predicate "alphalessp" will return 't' if **
 ** the first atom is < than the second atom. We use strcmp.            **
 *************************************************************************/
struct conscell *bualphp(form)
struct conscell *form;
{      char *e1, *e2;
       if ((form != NULL)&&(GetString(form->carp,&e1)))
       {    form = form->cdrp;
            if ((form != NULL)&&(GetString(form->carp,&e2)))
            {   if (form->cdrp == NULL)
                {   if (strcmp(e1,e2) < 0)
                        return(LIST(thold));
                    else
                        return(NULL);
                };
            };
       };
       ierror("alphalessp");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
