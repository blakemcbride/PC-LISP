/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:36) -- CLOSED */                   

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buimplode:built in atom implosion function. Given a list of atoms   **
 ** it will compress the first character in all of them into one big at.**
 *************************************************************************/
struct conscell *buimplode(form)
struct conscell *form;
{      char work[MAXATOMSIZE],c,*d;
       int n = MAXATOMSIZE;
       d = work;
       if  ((form!=NULL)&&(form->cdrp == NULL)) {
            form = form->carp;
            if (form == NULL) return(NULL);
            if (form->celltype != CONSCELL) goto ERR;
            while(form != NULL)  {
               if (!GetChar(form->carp,&c)) goto ERR;
               if (--n <= 0) gerror("atom too big");
               *d++ = c;
               form = form->cdrp;
            }
            *d = '\0';
            return(LIST(CreateInternedAtom(work)));
       }
  ERR: ierror("implode");
}
