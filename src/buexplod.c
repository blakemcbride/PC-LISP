

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"



/*************************************************************************
 ** buexplode:built in atom explosion function. Given an atom it will   **
 ** create a list of letters corresponding to the letters in the name.  **
 ** Note that atom 'nil expands to (n i l) and likewise a null car      **
 ** pointer will expand to (n i l).                                     **
 *************************************************************************/
struct conscell *buexplode(form)
struct conscell *form;
{      char work[2],*s; struct conscell *f,*l;
       work[1] = '\0';
       push(f); push(l);
       if  ((form!=NULL)&&(form->cdrp == NULL))
       {    if (!GetNumberOrString(form->carp,&s)) goto ERR;
            f = l = NULL;
            while(*s)
            {   f = new(CONSCELL);
                f->cdrp = l;
                l = f;
                *work = *s++;
                f->carp = LIST(CreateInternedAtom(work));
            };
            xret(reverse(f),2);
       };
  ERR: ierror("explode");
}
