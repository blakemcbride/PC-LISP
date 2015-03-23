

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (sstatus option expr) Will call SetOption with the appropriate opt  **
 ** and flag (1 or 0) according to the printname of 'option and the val **
 ** of expr nil = 0 non nil = t. Neither parameter is evaluated.        **
 *************************************************************************/
struct conscell *busstatus(form)
struct conscell *form;
{      char *option; struct conscell *expr;
       int opt;
       if (form != NULL)
       {   if (GetString(form->carp,&option) && (form->cdrp != NULL))
           {   form = form->cdrp;
               if ((form != NULL)&&(form->cdrp == NULL))
               {   expr = form->carp;
                   if (strcmp(option,"smart-slash") == 0)
                       opt = SMARTSLASH; else
                   if (strcmp(option,"ignoreeof") == 0)
                       opt = IGNOREEOF; else
                   if (strcmp(option,"automatic-reset") == 0)
                       opt = AUTORESET; else
                   if (strcmp(option,"chainatom") == 0)
                       opt = CHAINATOM;
                   else
                       goto ERR;
                   SetOption(opt,expr != NULL);
                   return(LIST(thold));
               };
           };
       };
ERR:   ierror("sstatus");
}
