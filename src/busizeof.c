/* EDITION AD01, APFUN PAS.765 (91/12/10 16:57:46) -- CLOSED */                 
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (sizeof exp) Will return the size of the cell required to store one **
 ** object of type exp. No incidental heap space is considered. Also the**
 ** size returned is the individual object size, The real sizes of the  **
 ** cons/file/real & fix cell is the maximum of all their sizes because **
 ** they are all stored in the same size block of storage.              **
 *************************************************************************/
struct conscell *busizeof(form)
struct conscell *form;
{      char *exp;
       if ((form!=NULL)&&(form->carp != NULL)&&(form->cdrp == NULL))
       {    if (GetString(form->carp,&exp))
            {   if (strcmp(exp,"list") == 0)
                    return(newintop((long int) sizeof(struct conscell)));
                if (strcmp(exp,"port") == 0)
                    return(newintop((long int) sizeof(struct filecell)));
                if (strcmp(exp,"flonum") == 0)
                    return(newintop((long int) sizeof(struct realcell)));
                if (strcmp(exp,"fixnum") == 0)
                    return(newintop((long int) sizeof(struct fixcell)));
                if (strcmp(exp,"string") == 0)
                    return(newintop((long int) sizeof(struct stringcell)));
                if (strcmp(exp,"symbol") == 0)
                    return(newintop((long int) sizeof(struct alphacell)));
                if (strcmp(exp,"hunk") == 0)
                    return(newintop((long int) sizeof(struct hunkcell)));
                if (strcmp(exp,"array") == 0)
                    return(newintop((long int) sizeof(struct arraycell)));
                if (strcmp(exp,"clisp") == 0)
                    return(newintop((long int) sizeof(struct clispcell)));
                if (strcmp(exp,"fixfix") == 0)
                    return(newintop((long int) sizeof(struct fixfixcell)));
            };
       };
       ierror("sizeof");
}
