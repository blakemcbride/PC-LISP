

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (uconcat 'xx 'yy 'zz....) Will take all of its atom parameters and  **
 ** make a compressed atom out of them. Checking to make sure that the  **
 ** result does not overflow the work buffer. The resulting atom is not **
 ** and interned atom.                                                  **
 *************************************************************************/
struct conscell *buuconcat(form)
struct conscell *form;
{      char work[MAXATOMSIZE], *d,*s; int n = MAXATOMSIZE;
       if (form == NULL) return(NULL);
       for(d = work; form != NULL; form = form->cdrp, d--)
       {   if (!GetNumberOrString(form->carp,&s)) goto ERR;
           while(*d++ = *s++)
               if (--n <= 0) gerror("atom too big");
       }
       *d = '\0';
       return(LIST(CreateUninternedAtom(work)));
  ERR: ierror("uconcat");
}
