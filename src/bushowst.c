

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** bushowstack: Will call HoldStackOperation(f)  in main.c which will  **
 ** have copied a portion of the mark stack when the error occured. If  **
 ** we are not at the break-level HoldStackOperation will show the last **
 ** held chunk of the mark stack, this may be empty.                    **
 *************************************************************************/
struct  conscell *bushowstack(form)
struct  conscell *form;
{       if (form == NULL)
        {   HoldStackOperation(DUMP_STACK);
            return(LIST(thold));
        };
        ierror("showstack");
}
