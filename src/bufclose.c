

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bufclose: Close file whose file pointer is my parameter note that we**
 ** make the file pointer illegal after the close this causes i/o errs  **
 ** if writing is attempted to the file after it is closed. fclose will **
 ** return EOF on a close error, we trap this condition. Note that after**
 ** a successfull close we call buresetlog with the file pointer and 0  **
 ** to tell the (resetio) function that this file pointer no longer is  **
 ** opened by lisp.                                                     **
 ** To support the (*process) primitive we check for a special print    **
 ** name for the port which if it contains (%d* forces a close of this  **
 ** %d file also. These are the control ports that force a pseudo TTY to**
 ** stay open even after the child has died so that the buffers remain  **
 ** intact for reading by the parent at its leisure.                    **
 *************************************************************************/
struct conscell *bufclose(form)
struct conscell *form;
{      FILE **p; char *s; int fd;
       if (form != NULL) {
          if (form->carp != NULL) {
              if (form->carp->celltype == FILECELL) {
                  p = &(PORT(form->carp)->atom);
                  if (*p == NULL) ioerror(*p);
                  buresetlog(*p, 0);
                  if (fclose(*p) == EOF) ioerror(*p);
                  *p = NULL;                     /* now illegal FILE! */
                  if (GetString(PORT(form->carp)->fname, &s)) {
                      if (sscanf(s,"%*s (%d*", &fd) == 1)
                          close(fd);
                  }
                  return(LIST(thold));
              }
          }
       }
       ierror("close");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
