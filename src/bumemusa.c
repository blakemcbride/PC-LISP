/* EDITION AD01, APFUN PAS.765 (91/12/10 16:57:44) -- CLOSED */                 
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** memoryusage(l) Will return the approximate amount of storage that   **
 ** list 'l' is occupying. We do not consider property lists or value   **
 ** stack of shallow bindings. These could lead to infinite recursion,  **
 ** especially if the list were a recursive function body. To allow an  **
 ** exit from an infinite recursion the BREAK condition is tested.      **
 *************************************************************************/
long memoryusage(l)
struct conscell *l;
{      if (l == NULL) return(0L);
       TEST_BREAK();
       switch(l->celltype)
       {      case CONSCELL  : return( (long) sizeof(struct conscell)
                             + memoryusage(l->carp)
                             + memoryusage(l->cdrp));
              case FIXATOM   : return( (long) sizeof(struct fixcell));
              case FIXFIXATOM: return( (long) sizeof(struct fixfixcell));
              case REALATOM  : return( (long) sizeof(struct realcell));
              case FILECELL  : return( (long) sizeof(struct filecell)
                             + memoryusage(PORT(l)->fname));
              case ALPHAATOM : return( (long) sizeof(struct alphacell)
                             + strlen(ALPHA(l)->atom) + 1);
              case STRINGATOM: return( (long) sizeof(struct stringcell)
                             + strlen(STRING(l)->atom) + 1);
              case HUNKATOM  : return( (long) sizeof(struct hunkcell)
                             + (HUNK(l)->size/2)*sizeof(struct conscell)
                             + HUNK(l)->size * sizeof(struct conscell *));
              case ARRAYATOM : return( (long) sizeof(struct arraycell)
                             + FIX(ARRAY(l)->info->carp)->atom *
                               sizeof(struct conscell *));
              case CLISPCELL : return( (long) sizeof(struct clispcell) );
              default        : fatalerror("memusage");
       };
}

/*************************************************************************
 ** (memusage exp) Will return the number of bytes that the list exp is **
 ** occupying. This is only approximate, and does not count the property**
 ** and value stack bindings or function bodies. Heap overhead is also  **
 ** not counted.                                                        **
 *************************************************************************/
struct conscell *bumemusage(form)
struct conscell *form;
{      if ((form != NULL)&&(form->cdrp == NULL))
            return(newintop(memoryusage(form->carp)));
       ierror("memusage");
}
