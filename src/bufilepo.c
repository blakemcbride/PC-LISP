/* EDITION AC01, APFUN PAS.765 (91/12/10 16:57:44) -- CLOSED */                 
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (filepos port [fix]) If num is given we seek to offset [num] on the **
 ** stream port, if num is not given the current file position is what  **
 ** we return, otherwise [num] is returned.                             **
 *************************************************************************/
struct conscell *bufilepos(form)
struct conscell *form;
{      FILE *port; long offset; extern long ftell();
       if ((form == NULL)||(form->carp == NULL)) goto ERR;
       if (form->carp->celltype != FILECELL) goto ERR;
       port = ((struct filecell *)form->carp)->atom;
       if (port == NULL) goto IOERR;
       if (form->cdrp == NULL)
           return(newintop((long)ftell(port)));
       form = form->cdrp;
       if ((form->cdrp != NULL)||(form->carp == NULL)) goto ERR;
       if (form->carp->celltype != FIXATOM) goto ERR;
       offset = FIX(form->carp)->atom;
       if (fseek(port,offset,0) == 0)
           return(newintop(offset));
IOERR: ioerror(port);
  ERR: ierror("filepos");
}
