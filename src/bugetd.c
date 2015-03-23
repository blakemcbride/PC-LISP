

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** bugetd: (getd symbol) Will return the lambda body associated with   **
 ** the symbol 'symbol' or nil if none is found.                        **
 *************************************************************************/
struct conscell *bugetd(form)
struct conscell *form;
{      struct alphacell *atm;
       if (form == NULL) goto ERR;
       atm = (struct alphacell *)form->carp;
       if ((form->cdrp == NULL)&&(atm != NULL)&&(atm->celltype == ALPHAATOM)) {
           if (FN_ISUS(atm->fntype) || FN_ISCLISP(atm->fntype))
                return((struct conscell *)atm->func);
           if (FN_ISBU(atm->fntype))
                return(LIST(CreateInternedAtom("binary")));
           if ((atm->valstack != NULL) && (atm->valstack->carp != NULL) &&
               (atm->valstack->carp->celltype == ARRAYATOM))
                return(atm->valstack->carp);
           return(NULL);
       }
ERR:   ierror("getd");
}
