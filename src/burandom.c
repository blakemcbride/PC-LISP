#include        <stdio.h>
#include        <math.h>
#if RTPC
#  include        "/usr/include/time.h"
#else
#  include        <sys/time.h>
#endif
#include        "lisp.h"

/*************************************************************************
 ** burandom: (random [limit]) Returns random in 0..parm or -fix...fix  **
 ** We use the function rand() from the math library. First we get the  **
 ** limit parameter if one is provided. If a limit is given we generate **
 ** in the range 0....limit. If no limit is given we generate in range  **
 ** -MAXLONG....0...MAXLONG. We do this by generating a random in 0..1  **
 ** then multiplying by the limit, then by the sign and returning it.   **
 ** Note rand() is assumed to return numbers in 0..2^15-1 as per UNIX.  **
 *************************************************************************/
struct conscell *burandom(form)
struct conscell *form;
{      long limit; int sign; double temp; extern int rand();
       if (form != NULL)
       {  if ((form->carp != NULL)&&(form->cdrp == NULL))
          {  if (form->carp->celltype == FIXATOM)
             {  limit =  FIX(form->carp)->atom;
                sign = 1;
                goto OK;
             };
          };
          ierror("random");
       };
       sign = rand() & 1 ? 1 : -1;               /* pick sign + or - */
       limit = MAXLONG;                          /* max value */
OK:    temp = rand();                            /* random in 0..MAXRANDVALUE */
       temp /= MAXRANDVALUE;                     /* random in 0..1 */
       temp *= limit*sign;                       /* random in 0..limit */
       limit = temp;
       return(newintop(limit));                  /* return it as fixnum*/
}

