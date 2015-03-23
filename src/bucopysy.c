

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (copysymbol at flag) Will return a new uninterned atom which is of  **
 ** same form as at. Ie we built a new alphacell with new print name etc**
 ** and return it. If the flag is non nil, we set all the property,bind **
 ** and function info to be 'eq' to that of the copied atom 'at'.       **
 *************************************************************************/
struct conscell *bucopysymbol(form)
struct conscell *form;
{      struct alphacell *at,*nat;
       if ((form != NULL)&&(form->carp!=NULL))
       {   at = ALPHA(form->carp);
           if (at->celltype == ALPHAATOM)
           {   form = form->cdrp;
               if ((form != NULL)&&(form->cdrp == NULL))
               {   nat = CreateUninternedAtom(at->atom);
                   if (form->carp != NULL)
                   {   nat->fntype = at->fntype;
                       nat->func = at->func;
                       nat->proplist = at->proplist;
                       nat->botvaris = at->botvaris;
                       nat->valstack = at->valstack;
                   };
                   return(LIST(nat));
               };
           };
       };
       ierror("copysymbol");
}
