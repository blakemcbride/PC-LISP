/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:40) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (maknam '(xx yy zz)) Will take its single parameter list of atoms   **
 ** and return a new atom uninterned atom formed from the first char in **
 ** each of the atoms in the list passed to it. If the list is nil we   **
 ** return nil (which is neither interned or not interned)  When we are **
 ** building up the new atom we must check for buffer overflow.         **
 *************************************************************************/
struct conscell *bumaknam(form)
struct conscell *form;
{      char work[MAXATOMSIZE], *d, c; int n = MAXATOMSIZE;
       if ((form != NULL)&&(form->cdrp == NULL)) {
           form = form->carp;
           if (form == NULL) return(NULL);
           if (form->celltype != CONSCELL) goto ERR;
           for(d = work; form != NULL; form = form->cdrp) {
               if (form->celltype != CONSCELL) goto ERR;
               if (!GetChar(form->carp,&c)) goto ERR;
               if (--n <= 0) gerror("atom too big");
               *d++ = c;
           }
           *d = '\0';
           return(LIST(CreateUninternedAtom(work)));
       }
  ERR: ierror("maknam");
}
