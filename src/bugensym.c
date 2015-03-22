/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:30) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** bugensym: Given an atom parameter (or none) We will generate an atom**
 ** by concatenating the string next number to the atom name creating a **
 ** new atom. We then intern it on the oblist by calling 'Create....' we**
 ** will check however that no atom exists by this name, if one does we **
 ** try again until we get a good one. The default is atom 'g.          **
 *************************************************************************/
struct conscell *bugensym(form)
struct conscell *form;
{      char work[MAXATOMSIZE],*r;
       static long int NextNumber = 0L;
       if (form == NULL)                                        /* use 'g */
       {   for(;;)
           {   sprintf(work,"g%05ld",NextNumber++);
               if (lookupatom(work,INTERNED) == NULL)
                   if (lookupatom(work,NOT_INTERNED) == NULL)
                       return(LIST(CreateUninternedAtom(work)));
           }
       }
       if (form->cdrp == NULL)                                  /* one parm */
       {   if (GetString(form->carp,&r))
           {   if (strlen(r)+5 >= MAXATOMSIZE) gerror("atom too big");
               for(;;)
               {   sprintf(work,"%s%05ld",r,NextNumber++);
                   if (lookupatom(work,INTERNED) == NULL)
                       if (lookupatom(work,NOT_INTERNED) == NULL)
                           return(LIST(CreateUninternedAtom(work)));
               }
           }
       }
  ERR: ierror("gensym");
}
