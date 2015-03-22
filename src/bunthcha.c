/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:46) -- CLOSED */                   
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** bunthchar: Will return the nth character from the print name of the **
 ** alpha atom which is the first parameter, nil if outside range.      **
 *************************************************************************/
struct conscell *bunthchar(form)
struct conscell *form;
{      char *atparm; long int ixparm; char temp[2]; extern int strlen();
       if ((form != NULL)&&(GetString(form->carp,&atparm)))
       {  form = form->cdrp;
          if (form != NULL)
          {   if ((form->cdrp == NULL)&&(GetFix(form->carp,&ixparm)))
              {   if ((--ixparm >= 0L)&&(ixparm < strlen(atparm)))
                  {    temp[0] = atparm[(int)ixparm];
                       temp[1] = '\0';
                       return(LIST(CreateInternedAtom(temp)));
                  }
                  else
                       return(NULL);
              };
          };
       };
       ierror("nthchar");
}
