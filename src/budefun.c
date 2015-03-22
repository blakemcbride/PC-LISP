/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:26) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** budefun: Will make a construct a lambda expression for the user func**
 ** ie: (defun try(x)(print 'hi)(+ x 1)) will construct the lambda expr **
 ** (lambda(x)(print 'hi)(+ x 1)) and call funcinstall to have it made  **
 ** part of the lisp system from now on. Note that fexprs are also      **
 ** handled. We pick up this parameter and make it an nlambda expression**
 ** Note that we call lexprify(r) to process any &optional args and to  **
 ** convert the expression to an lexpr of the appropriate form.         **
 *************************************************************************/
struct conscell *budefun(form)
struct conscell *form;
{      struct conscell *r,*s,*atm,*args,*type;
       xpush(form); push(r);push(s); push(args);
       if (form == NULL) fret(NULL,4);
       atm = form->carp;
       if ((atm != NULL)&&(atm->celltype == ALPHAATOM))
       {   form = form->cdrp;
           if (form != NULL)
           {   args = form->carp;
               if ((args != NULL)&&(args->celltype == ALPHAATOM))
               {   if (args == LIST(lookupatom("fexpr",INTERNED)))
                       type = LIST(nlambdahold);
                   else
                   {   if (args == LIST(lookupatom("expr",INTERNED)))
                           type = LIST(lambdahold);
                       else
                           if (args == LIST(lookupatom("macro",INTERNED)))
                               type = LIST(macrohold);
                           else
                               type = LIST(lexprhold);
                   };
                   if (type == LIST(lexprhold))
                       args = enlist(args);
                   else
                   {   form = form->cdrp;
                       if (form != NULL)
                           args = form->carp;
                       else
                            ierror("defun");
                   };
               }
               else
                   type = LIST(lambdahold);
               form = form->cdrp;
               if (form != NULL)
               {   r = new(CONSCELL);
                   r->carp = type;              /* lambda or nlambda */
                   s = new(CONSCELL);
                   r->cdrp = s;
                   s->carp = args;
                   s->cdrp = form;
                   if (type == (struct conscell *)macrohold)
                       funcinstall(FN_USMACRO,FUNCTION(lexprify(r)),NULL,atm);
                   else
                       funcinstall(FN_USEXPR,FUNCTION(lexprify(r)),NULL,atm);
                   fret(atm,4);
               };
           };
        };
        ierror("defun");
}

