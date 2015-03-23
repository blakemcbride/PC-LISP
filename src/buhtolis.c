

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buhtolis: (hunk-to-list H) Will make a list consisting of the elems **
 ** in the hunk H. Note we just call the HunkToList function to do this.**
 *************************************************************************/
struct conscell *buhtolis(form)
struct conscell *form;
{      if ((form != NULL)&&(form->carp != NULL)&&(form->cdrp == NULL))
       {    if (form->carp->celltype == HUNKATOM)
                return(HunkToList(form->carp));
       };
       ierror("hunk-to-list");
}
