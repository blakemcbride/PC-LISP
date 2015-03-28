

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** busetplist: Will set the property list of its first parameter to the**
 ** value given in its second parameter. It returns this property list. **
 *************************************************************************/
struct conscell *busetplist(form)
struct conscell *form;
{      struct alphacell *at;
       struct conscell  *ex;
       if (form != NULL)
       {  if (form->carp != NULL)
          {   if (form->carp->celltype == ALPHAATOM)
              {   at = ALPHA(form->carp);
                  form = form->cdrp;
                  if (form != NULL)
                  {   if (form->cdrp == NULL)
                      {   ex = form->carp;
                          at->proplist = ex;
                          return(ex);
                      }
                  }
              }
          }
       }
       ierror("setplist");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
