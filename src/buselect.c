/* EDITION AA02, APFUN PAS.824 (92/04/28 11:01:18) -- CLOSED */                 
/* --- */

/*
 | PC-LISP (C) 1989-1992 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <sys/time.h>

/*
 | Pure SYSV machines require an include of select.h
 */
#if !defined(FD_SET)
#   include <sys/select.h>
#endif

/*
 | The normal LISP include files.
 */
#include "lisp.h"

/************************************************************************************************
 ** (*select port port ... [timeout]) will return the first port that is ready for a read with **
 ** a wait of no more than timeout seconds. Fractions of seconds are converted to microseconds **
 ** by multiplying by 1 million.                                                               **
 ************************************************************************************************/

struct conscell *buselect(form)
struct conscell *form;
{
       fd_set rfdset, wfdset;
       struct filecell *port; FILE *fp;
       struct timeval tv_s, *tv = NULL;
       int n, fd, maxfd = -1;
       struct conscell *s;

       FD_ZERO(&rfdset);                                            /* set of descriptors to check for read = {empty set} */
       FD_ZERO(&wfdset);                                            /* set of descriptors to check for write = {empty set} */
       for(s = form; s != NULL; s = s->cdrp) {                      /* foreach argument to (*select ...) */
           if (s->celltype != CONSCELL) goto er;                    /* make sure no dotted pairs */
           port = PORT(s->carp);                                    /* extract Nth argument */
           if (!port) goto er;                                      /* nil argument not allowed */
           if (port->celltype == FILECELL) {                        /* if argument is a port */
               fp = port->atom;                                     /* extract FILE * from atom */
               if (fp == NULL) ioerror(fp);                         /* a null 'atom' indicates file closed */
               if ((fp->_cnt > 0) && (fp->_flag & _IOREAD))         /* a read only port with data pending causes an immediate return */
                    return(LIST(port));                             /* of that port */
               if (fp->_flag & (_IOREAD | _IORW))                   /* if a read or read/write port add to set to check for read */
                    FD_SET(fileno(fp), &rfdset);                    /* add the file # of this FILE * to set to check */
               if (fp->_flag & _IOWRT)                              /* if a pure write only port */
                    FD_SET(fileno(fp), &wfdset);                    /* add the file # of this FILE * to set to check for write */
               if (fileno(fp) > maxfd) maxfd = fileno(fp);          /* also track largest file # seen so far for select call */
           } else {
               double timeout; long secs;                           /* argument not a port, must be the optional timeout */
               if (!GetFloat(port, &timeout)) goto er;              /* try to get a float from this argument */
               if (s->cdrp) goto er;                                /* if other arguments follow this then throw an error */
               tv = &tv_s;                                          /* a timeout is specified so call select with it instead of NULL */
               secs = floor(timeout);                               /* #seconds to wait is floor of timeout value */
               timeout -= secs;                                     /* now get rid of the seconds from the timeout leaving fraction */
               tv->tv_sec = secs;                                   /* assign seconds to the tv-> structure */
               tv->tv_usec = timeout * 1000000.0;                   /* compute microseconds and assign to tv->structure */
           }
       }

      /*
       | If no arguments provided then just exit with nil.
       */
       if (maxfd < 0) goto nil;

      /*
       | Block for tv seconds until one of the file descriptors is ready for read then return
       | the first of the ready descriptors. May have to loop on EINTR if interrupted by a SIGCHLD
       | or something. If it was a sigint then run the TEST_BREAK and possible jump to debugger
       | code.
       */
       for(;;) {
           n = select(maxfd+1, &rfdset, &wfdset, 0, tv);            /* wait tv sec/usec for one of rfdset/wfdset descriptors have data */
           if (n > 0) break;                                        /* positive n indicates how many descriptors have data */
           if (n == 0) goto nil;                                    /* n = 0 indicates timeout occurred */
           TEST_BREAK();                                            /* otherwise we got an error so check for break now */
           if (errno != EINTR) ierror("*select");                   /* if error was not caused by SIGCHLD etc. throw error */
       }                                                            /* otherwise loop around again and restart the select call */

      /*
       | Loop through all the passed ports and determine the first one in the returned
       | set of 'ready' descriptors.
       */
       for(s = form; s != NULL; s = s->cdrp) {                      /* for each argument */
           port = PORT(s->carp);                                    /* get the actual argument */
           if (port->celltype == FILECELL) {                        /* if it is a port */
               fd = fileno(port->atom);                             /* get file # of the FILE * on the port */
               if (FD_ISSET(fd, &rfdset))                           /* if this file # is in the subset returned by select */
                   return(LIST(port));                              /* just return it */
               if (FD_ISSET(fd, &wfdset))                           /* if this file # is in the subset returned by select */
                   return(LIST(port));                              /* just return it */
           }
       }

      /*
       | Normal Exit when no data present in given timeout.
       */
 nil:  return(NULL);

      /*
       | Something wrong with the arguments.
       */
  er:  ierror("*select");
}


