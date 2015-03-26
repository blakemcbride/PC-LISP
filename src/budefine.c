

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** budefine: (def symbol function-body) Will associate the function    **
 ** body with symbol by calling the funcinstall function to attach the  **
 ** lambda expression to the atom symbol. ARGS NOT EVALED. Note that we **
 ** call lexprify(r) to process any &optional stuff and turn the lambda **
 ** into into an lexpr if necessary.                                    **
 *************************************************************************/
struct conscell *budefine(form)
struct conscell *form;
{      struct conscell *atm,*body,*type;
       if (form == NULL) return(NULL);
       atm = form->carp;
       form = form->cdrp;
       if ((form != NULL)&&(atm != NULL)&&(atm->celltype == ALPHAATOM))
       {   body = form->carp;
           form = form->cdrp;
           if ((form == NULL)&&(body != NULL)&&(body->celltype == CONSCELL))
           {    type = body->carp;
                if ((type==LIST(lambdahold))||(type==LIST(nlambdahold))||
                    (type==LIST(lexprhold)))
                {    funcinstall(FN_USEXPR,FUNCTION(lexprify(body)),NULL,atm);
                     return(atm);
                }
                else
                     if (type == (struct conscell *)macrohold)
                     {   funcinstall(FN_USMACRO,FUNCTION(lexprify(body)),NULL,atm);
                         return(atm);
                     };
           };
       };
       ierror("def");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
