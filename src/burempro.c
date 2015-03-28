

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** buremprop: Will remove the property named from the property list of **
 ** the given atom. ie (remprop 'peter 'lastname)                       **
 *************************************************************************/
struct conscell *buremprop(form)
struct conscell *form;
{      struct alphacell *at;
       struct conscell  *ex,**prev,*next;
       if (form != NULL)
       {  if (form->carp != NULL)
          {   if (form->carp->celltype == ALPHAATOM)
              {   at = ALPHA(form->carp);
                  form = form->cdrp;
                  if (form != NULL)
                  {   if (form->carp != NULL)
                      {   ex = form->carp;
                          next = *(prev = &(at->proplist));
                          while(next != NULL)
                          {     if (next->carp != NULL)
                                    if (equal(next->carp->carp,ex))
                                    {   *prev = next->cdrp;
                                        return(next);
                                    };
                                prev = &(next->cdrp);
                                next = next->cdrp;
                          };
                          return(NULL);
                      };
                  };
              };
          };
       };
       ierror("remprop");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
