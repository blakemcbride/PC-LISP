

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/****************************************************************************
 ** The following stack is used by the xpush, push, xpop, ret and fret     **
 ** macros. These are used for pushing items that need to be marked if gc  **
 ** occurs in the procedure that the push, or xpushes occur. This stuff is **
 ** pretty sensitive, if you do not get it right you will crash the interp **
 ** reter during garbage collection, or worse later which is hard to debug.**
 ** Two stacks are placed in mystack. One builds from the bottom up and the**
 ** other from the top down. All functions except eval() push onto lower   **
 ** stack. Eval pushes and pops from the higher stack. This makes showstack**
 ** much simpler because we just copy the higher stack and do not have to  **
 ** rummage through the mystack trying to figure out what is eval dropping.**
 ****************************************************************************/
struct conscell ***mystack;                        /* stack space array  */
int mytop,emytop;                                  /* lower and upper tops */

markstack()
{       int i; extern marklist();
        for(i=0; i < mytop; i++)                   /* mark lower stack */
            marklist(*mystack[i]);
        for(i=emytop+1; i < MSSIZE; i++)           /* mark upper stack */
            marklist(*mystack[i]);
}

/****************************************************************************
 ** InitMarkStack: Will dynamically allocate the array mystack with MSSIZE **
 ** pointers to cons cells in it. We allocate it dynamically because if    **
 ** not the space is included in the executable file which increases it by **
 ** MSSIZE * sizeof(char *) or nearly 32K bytes. ie costs space/load time. **
 ****************************************************************************/
InitMarkStack()
{       int siz; /* extern char *calloc(); */
        siz = (MSSIZE * sizeof(struct conscell **));
        if ((mystack = (struct conscell ***)calloc(siz,1)) == NULL)
            UpError("not nearly enough memory");
        mytop = 0;
        emytop = MSSIZE-1;
}

