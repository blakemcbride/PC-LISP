

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (setsyntax ': 'vmacro '(lambda() -expr-) ) Will set the syntax of   **
 ** its first atom parameter to be a v or vsplicing macro whose body is **
 ** the lambda expression that is the last parameter. The second parm is**
 ** 'vmacro or 'vsplicing-macro. All we have to do is to verify the args**
 ** then if all is ok call the ScanSetSynClassMacro function to set up  **
 ** the scanner to recognize the atom char as a MACRO trigger. Then we  **
 ** bind the body to the atom globally, this will be how we look up the **
 ** body when the macro is triggered and executed by RunReadMacro in the**
 ** main.c module.                                                      **
 *************************************************************************/
struct conscell *busetsyntax(form)
struct conscell *form;
{      struct alphacell *at,*kind; struct conscell *expr;
       if (form != NULL)
       {   at = ALPHA(form->carp);
           form = form->cdrp;
           if ((form != NULL)&&(at != NULL)&&(at->celltype == ALPHAATOM))
           {   kind = ALPHA(form->carp);
               form = form->cdrp;
               if ((form != NULL)&&(form->cdrp == NULL))
               {   expr = form->carp;
                   if ((kind != NULL)&&(kind->celltype == ALPHAATOM))
                   {   if ((expr==NULL)||(expr->celltype!=CONSCELL)) goto ERR;
                       expr = expr->cdrp;
                       if ((expr==NULL)||(expr->celltype!=CONSCELL)) goto ERR;
                       expr = expr->cdrp;
                       if ((expr==NULL)||(expr->celltype!=CONSCELL)) goto ERR;
                       expr = expr->carp;
                       if (strcmp(kind->atom,"vmacro") == 0)
                           ScanSetSynClassMacro(*(at->atom),0);
                       else
                       {   if (strcmp(kind->atom,"vsplicing-macro") == 0)
                               ScanSetSynClassMacro(*(at->atom),1);
                           else
                               goto ERR;
                       };
                       bindvar(at,expr);
                       at->botvaris = GLOBALVAR;
                       at->permbit = PERM;        /* atom is now permantent */
                       return(LIST(thold));
                   };
               };
           };
       };
  ERR: ierror("setsyntax");
}
