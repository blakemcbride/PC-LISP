/* EDITION AA01, APFUN PAS.765 (91/12/10 16:57:44) -- CLOSED */                 
/* --- */

/*
 | PC-LISP (C) 1991-1992 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (printstack <expr> <subexpr> [port])                                **
 ** ------------------------------------                                **
 ** buprinstack: just like print except that it will pass the substitute**
 ** arg to printlist. This is most useful when dumping tracebacks that  **
 ** should contain the <**> form.                                       **
 *************************************************************************/
struct conscell *buprintstack(form)
struct conscell *form;
{      struct conscell *expr, *subexpr;
       struct filecell *port;

      /*
       | Extract the arguments <expr> <subexpr> and optionally [port].
       */
       if (form == NULL) goto er;
       expr = form->carp;
       form = form->cdrp;
       if (form == NULL) goto er;
       subexpr = form->carp;
       form = form->cdrp;
       if (form != NULL) {
           port = PORT(form->carp);
           if ((port == NULL) || (port->celltype != FILECELL) || (form->cdrp != NULL)) goto er;
       } else
           port = poporthold;

      /*
       | Now actually print the expression using the subexpression to substitute as we go. If reading
       | from a socket then rewind it first and set state to writing.
       */
       if ((port == NULL) || (port->atom == NULL)) goto IOERR;
       if (port->issocket && port->state == 1) rewind(port->atom);
       port->state = 2;
       printlist(port->atom,expr,DELIM_ON,subexpr,NULL);
       if (ferror(port->atom)) goto IOERR;
       return(expr);

   er: ierror("printstack");
IOERR: ioerror(port);
}
