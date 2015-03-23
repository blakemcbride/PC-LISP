

/*
 | PC-LISP (C) 1989-1992 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include "lisp.h"

/***************************************************************************
 ** buboole: (boole key fixnum1 fixnum2). Will perform a bitwise operation**
 ** involving fixnum1 and fixnum2 according to op. This is straight from  **
 ** LISPcraft page 342.                                                   **
 ***************************************************************************/
struct conscell *buboole(form)
struct conscell *form;
{
       long x = 0L, y = 0L, result, key;

      /*
       | Establish arguments key,x and y. If x or y not specified set them to 0.
       */
       if (form == NULL) goto er;
       if (!GetFix(form->carp, &key)) goto er;
       form = form->cdrp;
       if (form) {
           if (!GetFix(form->carp, &x)) goto er;
           form = form->cdrp;
           if (form) {
              if (!GetFix(form->carp, &y)) goto er;
              if (form->cdrp) goto er;
           }
       }

      /*
       | Depending on the key perform the indicated operation on x and y.
       | I'm not sure what the relationship between the key and the operation
       | performed is but this is a direct copy of the Franz implementation.
       */
       switch(key) {
           case 0 : result = 0;          break;
           case 1 : result =    x & y;   break;
           case 2 : result = (~x) & y;   break;
           case 3 : result = y;          break;
           case 4 : result = x & (~y);   break;
           case 5 : result = x;          break;
           case 6 : result = x ^ y;      break;
           case 7 : result = x | y;      break;
           case 8 : result = ~(x | y);   break;
           case 9 : result = ~(x ^ y);   break;
           case 10: result = ~(x);       break;
           case 11: result = (~x)|y;     break;
           case 12: result = (~y);       break;
           case 13: result = x | (~y);   break;
           case 14: result = (~x) | (~y);break;
           case 15: result = -1;         break;
           default: goto er;
       }

       return(newintop( result ));
 er:   ierror("boole");
}


