/* */
/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*
 | Utility routine used by (readln) it will read chars from 'fp' into work up
 | to n characters. If EOF is encountered then -1 is returned. If too many
 | chars are read before a newline then 0 is returned. Note that 'n' is
 | the size of the buffer in total so we have to count the '\0'. This is
 | the reason for the n-- at the start of the loop. If we are reading from
 | standard input we flush any buffered input by other routines (notably the
 | scanner to get the desired effect because after reading S-expressions a
 | single \n will remain in the buffer causing all (readln)s to return "").
 */
static int get_a_line(fp, work, n)
   FILE *fp; char *work; int  n;
{
   register int c;
   if (fp == stdin) fflush(stdin);                /* get rid of ungot \n */
   for(n-- ; ; ) {                                /* loop, need space for \0 */
        if (n-- < 0) return(0);                   /* if out of space return 0 */
        if ((c = fgetc(fp)) == EOF) return(-1);   /* get char on eof return -1 */
        if (c == '\n') break;                     /* if end of line exit loop */
        *work++ = c;                              /* not end of line store char */
   }
   *work = '\0';                                  /* end of line, null terminate */
   return(1);                                     /* return 1 ==> sucess */
}

/*
 |  string <- (readln [port] [eof-flag])
 |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 |  This function takes one line from the given port and returns it. We
 |  must take care of the case that we are called from within a read macro
 |  if this is the case there will be no parameters  and the "macroport"
 |  atom will have a file cell type on its stack, we use this atom for
 |  the stream input. If the stack is empty then we are being called
 |  normally so just read from stdin. Otherwise read from the parameter
 |  port given, and return if appropriate the optional end of file flag.
 |  Note also that as soon as we establish the I/O port being used we
 |  assign it to "zapee" a global variable read by the (zapline) function.
 */
struct conscell *bureadln(form)
   struct conscell *form;
{
   struct conscell *rexp; char work[MAXATOMSIZE]; int rc = 0;
   struct filecell *p;

   if (form == NULL) {
      zapee = stdin;
      if ((rexp = macroporthold->valstack) != NULL)
         if (rexp->carp->celltype == FILECELL)
             if ((zapee = PORT(rexp->carp)->atom) == NULL) goto IO;
      if (feof(zapee)) return(NULL);
      if (ferror(zapee)) goto IO;
      if ((rc = get_a_line(zapee, work, sizeof(work))) == -1) return(NULL);
   } else {
      if ((form->carp != NULL) && (form->carp->celltype == FILECELL)) {
          p = PORT(form->carp);
          zapee = p->atom;
          form = form->cdrp;
          rexp = (form != NULL) ? form->carp : NULL;
          if ((form == NULL) || (form->cdrp == NULL)) {
             if ((zapee == NULL)|| ferror(zapee)) goto IO;
             if (p->issocket && p->state == 2) rewind(zapee);           /* if was writing socket rewind before reading */
             p->state = 1;                                              /* set new state to reading */
             if (feof(zapee)) return(rexp);
             if ((rc = get_a_line(zapee, work, sizeof(work))) == -1) return(rexp);
          }
      }
   }
   if (rc == 1)
      return(LIST(insertstring(work)));
   ierror("readln");
IO:ioerror(zapee);
}
