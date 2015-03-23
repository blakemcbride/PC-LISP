

/*
 | PC-LISP (C) 1990-1993 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bulinenum() : Will return the line number field stored in a conscell**
 ** or port object. When the lisp reader reads a list it stores the line**
 ** number of the scanner in the cons cell 'linenum' field. Every port  **
 ** cell holds the last scanned line number so that the reader can be   **
 ** directed at any port and continue where it left off.                **
 *************************************************************************/
struct conscell *bulinenum(form)
struct conscell *form;
{
       if (form != NULL) {
           if ((form->carp != NULL) && (form->cdrp == NULL)) {
               form = form->carp;
               switch(form->celltype) {
                  case CONSCELL:
                       return(newintop((long) form->linenum ));
                  case FILECELL:
                       return(newintop((long) PORT(form)->linenum ));
               }
               return(NULL);
           }
       }
       ierror("line-num");
}
