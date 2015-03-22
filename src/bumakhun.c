/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:40) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bumakhunk: (makhunk N) or (makhunk (e1 e2 ...)). If our parameter is**
 ** a number we return a nil filled hunk of N elements. That is just the**
 ** function of inserthunk(n). If the parameter is a non nil list we    **
 ** call ListToHunk with the list parameter to convert it for us.       **
 *************************************************************************/
struct conscell *bumakhunk(form)
struct conscell *form;
{      long int size;
       if ((form != NULL)&&(form->carp != NULL)&&(form->cdrp == NULL))
       {    if (GetFix(form->carp,&size))
                return(LIST(inserthunk((int)size)));
            if (form->carp->celltype == CONSCELL)
                return(LIST(ListToHunk(form->carp)));
       };
       ierror("makhunk");
}
