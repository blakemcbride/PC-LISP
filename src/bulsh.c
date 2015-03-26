

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bulsh:(lsh fix1 fix2) Left shift fix1 by fix2 bits return result    **
 *************************************************************************/
struct conscell *bulsh(form)
struct conscell *form;
{      long fix1, fix2;
       if ((form != NULL)&&(GetFix(form->carp,&fix1)))
       {  form = form->cdrp;
          if ((form != NULL)&&(GetFix(form->carp,&fix2))&&(form->cdrp==NULL))
          {  if (fix2 >= 0L)
                 return(newintop((long)(fix1 << fix2)));
             if (fix2 < 0L)
             {   fix2 = -fix2;
                 return(newintop((long)(fix1 >> fix2)));
             };
          };
       };
       ierror("lsh");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
