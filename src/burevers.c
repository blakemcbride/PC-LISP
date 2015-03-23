#include <stdio.h>
#include "lisp.h"

/*************************************************************************
 ** (nreverse <list>) returns a the list reversed at the top level. This**
 ** is a destructive copy.                                              **
 *************************************************************************/
struct conscell *bunreverse(form)
struct conscell *form;
{      extern struct conscell *nreverse();
       if ((form != NULL)&&(form->cdrp == NULL))
       {  if (form->carp == NULL) return(NULL);
          if (form->carp->celltype==CONSCELL)
              return(nreverse(form->carp));
       };
       ierror("nreverse");
}

/*************************************************************************
 ** (reverse <list>) returns a copy of the list reversed at the top     **
 ** level. The 'reverse' utility does this for us.                      **
 *************************************************************************/
struct conscell *bureverse(form)
struct conscell *form;
{      if ((form != NULL)&&(form->cdrp == NULL))
       {  if (form->carp == NULL) return(NULL);
          if (form->carp->celltype==CONSCELL)
              return(reverse(form->carp));
       };
       ierror("reverse");
}

