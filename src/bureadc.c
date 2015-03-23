

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bureadc: Takes one char from the given port and return it.  We must **
 ** take care of the case that we are called from within a read macro   **
 ** if this is the case their will be no parameters, and the macroport  **
 ** atom will have a file cell type on its stack, we use this atom for  **
 ** the stream input. If the stack is empty then we are being called    **
 ** normally so just read from stdin. Otherwise read from the parameter **
 ** port given, and return if appropriage the optional end of file flag **
 *************************************************************************/
struct conscell *bureadc(form)
struct conscell *form;
{      FILE *fp; struct conscell *rexp; char work[2]; int c;
       struct filecell *p;
       if (form == NULL)
       {  fp = stdin;
          if ((rexp = macroporthold->valstack) != NULL)
          {  if (rexp->carp->celltype == FILECELL)
                 fp = PORT(rexp->carp)->atom;
          };
          if (fp == NULL) goto IOERR;
          c = getc(fp);
          if (ferror(fp)) goto IOERR;
          zapee = fp;                                   /* future (zapline) */
          if (c == EOF) return(NULL);
          work[0] = (char) c;
          work[1] = '\0';
          return(LIST(CreateInternedAtom(work)));
       }
       else
       {  if (form->carp != NULL)
          {   if (form->carp->celltype == FILECELL)
              {   p = PORT(form->carp);
                  fp = p->atom;
                  form = form->cdrp;
                  if (form != NULL)
                      rexp = form->carp;
                  else
                      rexp = NULL;
                  if (fp == NULL) goto IOERR;
                  if (p->issocket && p->state == 2) rewind(p->atom);         /* if was writing socket rewind before reading */
                  p->state = 1;                                              /* set new state to reading */
                  c = getc(fp);
                  if (ferror(fp)) goto IOERR;
                  zapee = fp;                         /* future (zapline) */
                  if (c == EOF) return(rexp);
                  work[0] = (char) c;
                  work[1] = '\0';
                  return(LIST(CreateInternedAtom(work)));
              };
          };
       };
       ierror("readc");
IOERR: ioerror(fp);
}
