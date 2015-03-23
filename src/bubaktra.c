#include        <stdio.h>
#include        <ctype.h>
#include        "lisp.h"

/*
 | (baktrace)
 |
 | Is like (showstack) but works on a running program. It takes a snap shot
 | of the evaluation stack by dong a HoldStackOperation(COPY_STACK) and then
 | requests a dump of the copy by doing a HoldStackOperation(DUMP_STACK).
 */
struct conscell *bubaktrace(form)
struct conscell *form;
{      if (form != NULL) ierror("baktrace");
       HoldStackOperation(COPY_STACK);
       HoldStackOperation(DUMP_STACK);
       return(LIST(thold));
}

