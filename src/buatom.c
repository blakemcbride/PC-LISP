

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** buatom : This is the 'atom' predicate which if given an atom as a   **
 ** parameter will return the atom 't', otherwise it will return the    **
 ** atom 'nil'. For example : (atom x) is 't' but (atom (x)) is 'nil'   **
 ** Also note that (atom ()) is 't because () is equivalent to 'nil.    **
 *************************************************************************/
struct conscell *buatom(form)
struct conscell *form;
{      if ((form != NULL)&&(form->cdrp == NULL))
       {  if (form->carp != NULL)
          {   if ((form->carp->celltype == CONSCELL)||
                  (form->carp->celltype == HUNKATOM))
                  return(NULL);
          };
          return(LIST(thold));
       };
       ierror("atom");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
