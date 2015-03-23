

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bulast(last list) Return the last cons cell in the single list parm.**
 *************************************************************************/
struct conscell *bulast(form)
struct conscell *form;
{      struct conscell *l;
       if (form != NULL) {
           if (form->cdrp == NULL) {
              form = form->carp;
              if (form == NULL) return(NULL);
              if (form->celltype == CONSCELL) {
                 for(; form != NULL; form = form->cdrp) {
                     if (form->celltype != CONSCELL) break;
                     l = form;
                 }
                 return(l);
              }
           }
       }
       ierror("last");
}
