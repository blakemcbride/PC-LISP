

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/***************************************************************************
 ** buconcat: (concat s1 s2 s3...sN) Make a new atom whose print name is  **
 ** formed by concatenating the print names of s1...sN (or string values) **
 ** then return the new atom created. We use the buffer work to make the  **
 ** new atoms print name. We scan s1,s2,...sN and copy them into the work **
 ** buffer after the last atoms print name, we back up once after each is **
 ** copied to copy over the '\0'. Then when we are done we make sure that **
 ** string is null terminated and create or find an atom equivalent to it.**
 ***************************************************************************/
struct conscell *buconcat(form)
struct conscell *form;
{      char work[MAXATOMSIZE],*d,*s; int len = 0;
       if (form == NULL) return(NULL);
       for(d = work; form != NULL; form = form->cdrp)
       {   if (!GetNumberOrString(form->carp,&s)) goto ERR;
           if ((len += strlen(s)) >= MAXATOMSIZE) gerror("atom too big");
           while(*d++ = *s++);
           d--;
       };
       *(d+1) = '\0';
       return(LIST(CreateInternedAtom(work)));
ERR:   ierror("concat");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
