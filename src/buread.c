

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** buread: Takes one S-expression from the given port and return it.   **
 ** If no port is given we could be being called from within some sort  **
 ** of read macro. If this is the case then the port we are to read from**
 ** is the top value in the valstack of the special symbol macroporthold**
 ** which is the global holding pointer 'macroporthold'. If not a macro **
 ** and a port is provided then we will check for an end of file atom.  **
 ** This is the second parameter to ReadExpression and will be returned **
 ** by the latter if it detects an end of file on that port. The func   **
 ** ReadExpression is located in the module main.c because of its ties  **
 ** to the scanner (it must prime the scanner).                         **
 *************************************************************************/
struct conscell *buread(form)
struct conscell *form;
{      struct conscell *rexp; struct filecell *port;
       if (form == NULL)
       {  if ((rexp = macroporthold->valstack) == NULL)
              return(ReadExpression(piporthold,NULL,0));
          if (rexp->carp->celltype == FILECELL)
              return(ReadExpression(PORT(rexp->carp),NULL,0));
       }
       else
       {  if (form->carp != NULL)
          {   if (form->carp->celltype == FILECELL)
              {   port = PORT(form->carp);
                  if (port->issocket && port->state == 2) rewind(port->atom);   /* if was writing socket rewind before reading */
                  port->state = 1;                                              /* set new state to reading */
                  form = form->cdrp;
                  if (form != NULL)
                      rexp = form->carp;
                  else
                      rexp = NULL;
                  return(ReadExpression(port,rexp,1));
              };
          };
       };
       ierror("read");
}
