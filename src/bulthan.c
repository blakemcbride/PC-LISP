

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (< number1 number2) Compare 2 numbers for 'less than' relation.     **
 *************************************************************************/
struct conscell *bulthan(form)
struct conscell *form;
{      struct conscell *op1,*op2; register int result;
       xpush(form);
       if (form != NULL)
       {   op1 = form->carp;
           form = form->cdrp;
           if (form != NULL)
           {    op2 = form->carp;
                form = form->cdrp;
                if (form == NULL)
                {    if ((result = MixedTypeCompare(op1,op2)) == MT_LESS)
                         fret((struct conscell*)thold,1);
                     if (result != MT_ERROR)
                         fret(NULL,1);
                };
           };
       };
       ierror("<");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
