

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** busleep:(sleep -number-) Sleeps for number seconds and returns t.   **
 ** If time is greater than 1 minute then ignore the fractional part of **
 ** the sleep period (UNIX will never be that acurate anyway). If the   **
 ** sleep period is less than 60 seconds then convert to microseconds   **
 ** and use the usleep function.                                        **
 *************************************************************************/
struct conscell *busleep(form)
struct conscell *form;
{      double f;
       if ((form != NULL)&&(form->cdrp == NULL)) {
          if (GetFloat(form->carp, &f)) {
              if (f > 60.0) {
                  if (f < MAXINT) {
                      sleep((unsigned) f);
                      return( LIST( thold ) );
                  }
              } else {
                  if (f >= 0.0) {
                      f *= 1000000.0;                 /* convert seconds to microseconds */
                      usleep( (unsigned) f);
                      return( LIST( thold ) );
                  }
              }
          }
       }
       ierror("sleep");
}
