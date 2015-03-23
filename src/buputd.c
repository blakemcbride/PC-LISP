

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buputd: (putd symbol function-body) Will associate the function     **
 ** body with symbol by calling the funcinstall function to attach the  **
 ** lambda expression to the atom symbol. ARGS ARE EVALUATED. This will **
 ** also handle the case where the function body is nil, this causes the**
 ** old body pointer to be changed to NULL and the function kind to be  **
 ** NONE, this will allow the atom to be garbage collected later if it  **
 ** has no property of binding.                                         **
 *************************************************************************/
struct conscell *buputd(form)
struct conscell *form;
{      struct conscell *body,*type;
       struct alphacell *atm;
       if (form == NULL) goto ERR;
       atm = ALPHA(form->carp);
       form = form->cdrp;
       if ((form != NULL)&&(atm != NULL)&&(atm->celltype == ALPHAATOM)) {
           body = form->carp;
           form = form->cdrp;
           if (form == NULL) {
                if (body != NULL) {
                  if (body->celltype == CONSCELL) {
                     type = body->carp;
                     if ((type==LIST(lambdahold))||(type==LIST(nlambdahold))||
                         (type==LIST(lexprhold))) {
                          funcinstall(FN_USEXPR,FUNCTION(body),NULL,atm);
                          return(LIST(atm));
                     } else {
                          if (type == (struct conscell *)macrohold) {
                             funcinstall(FN_USMACRO,FUNCTION(body),NULL,atm);
                             return(LIST(atm));
                          }
                     }
                  } else {
                     if (body->celltype == CLISPCELL) {
                         funcinstall(FN_CLISP,FUNCTION(body),NULL,atm);
                         return(LIST(atm));
                     }
                  }
                } else {
                  atm->fntype = FN_NONE;
                  atm->func = NULL;
                  return(LIST(atm));
                }
           }
       }
ERR:   ierror("putd");
}
