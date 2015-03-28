

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** bupairlis : is the opposite of the buassoc function. It will take   **
 ** the parameters 'vars' 'vals' and 'alist' from its parameters 'form' **
 ** and return the result of a call to the pairlis function. For example**
 ** (pairlis '(GE IBM) '(appliances computers) '( (a.A) (B.b)))         **
 ** is ( (GE.appliances) (IBM.computers) (a.A) (B.b) )                  **
 *************************************************************************/
struct conscell *bupairlis(form)
struct conscell *form;
{      struct conscell *vars,*vals,*alist;
       xpush(form);
       if (form != NULL) {
            vars = form->carp;
            form = form->cdrp;
            if (form != NULL) {
                vals = form->carp;
                form = form->cdrp;
                if (form != NULL) {
                     alist = form->carp;
                     if (form->cdrp) goto er;
                     if (vars == NULL) return(alist);
                     if (vars->celltype != CONSCELL) goto er;
                     if ((vals != NULL) && (vals->celltype != CONSCELL)) goto er;
                     if ((alist != NULL) && (alist->celltype != CONSCELL)) goto er;
                     xret(pairlis(vars,vals,alist),1);
                }
            }
       }
  er:  ierror("pairlis");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
