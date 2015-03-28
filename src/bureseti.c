

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <assert.h>
#include "lisp.h"

#if !defined(_NFILE)                         /* _NFILE usually defined in stdio.h but if not */
#  define _NFILE 128                         /* set the value to something reasonably big */
#endif

extern int close();

/*************************************************************************
 ** lisp_opened is an array of booleans which are '1' if the file was   **
 ** opened by the LISP interpreter. It is updated when fopen is called  **
 ** or fclose is called. limain from load, bufopen and bufclose call the**
 ** buresetlog function with 1 or 2 to indicate they opened and 0 for a **
 ** close so that we can remember what is open so that resetio will work**
 ** In addition load calls with kind = 2 so that we can do a close of   **
 ** all opened files by load. When ierror occurs. This is done by making**
 ** a call to buresetlog(NULL, 2) to close all kind=2 descriptors.      **
 *************************************************************************/

static struct {
            short kind;                      /* used to allow multiple closes of a certain kind */
            int   flag;                      /* count of number of outstanding opens */
            FILE *fp;                        /* if (flag) then this was opened by LISP */
            int   asc_fd;                    /* another fd to be closed at same time if > 0 */
       } lisp_opened[_NFILE];                /* will be all 0's to begin with! */

void buresetlog(fp, kind)
FILE *fp; int kind;
{   int i, fd;
    if (fp == NULL) {
        for(i = 0; i < _NFILE; i++) {
            if ((lisp_opened[i].flag > 0) && (lisp_opened[i].kind == kind)) {
                 lisp_opened[i].flag = 0;
                 lisp_opened[i].kind = -1;
                 if (lisp_opened[i].fp != NULL) {
                     fclose(lisp_opened[i].fp);
                     lisp_opened[i].fp = NULL;
                     if (lisp_opened[i].asc_fd >= 0)
                         close(lisp_opened[i].asc_fd);
                     lisp_opened[i].asc_fd = -1;
                 }
            }
        }
    } else {
        fd = fileno(fp);                     /* turn FILE * into handle */
        assert((fd >= 0) && (fd < _NFILE));  /* make sure handle in range */
        if (kind > 0) {                      /* kind > 0 means open op */
            lisp_opened[fd].flag++;          /* increment open count and */
            lisp_opened[fd].fp = fp;         /* store the 'fp' here */
        } else {
            lisp_opened[fd].flag--;          /* kind <= 0 means close op */
            lisp_opened[fd].fp = NULL;       /* so dec open count and clear */
            lisp_opened[fd].asc_fd = -1;     /* no more associated file descriptor */
        }
        lisp_opened[fd].kind = kind;         /* store the kind only care if positive */
    }
}

/*************************************************************************
 ** buresetasc: Will record the fact that open file descriptor fp is    **
 ** associated with opened file descriptor 'asc_fd'. When one is closed **
 ** the other will be closed at the same time via (resetio). These are  **
 ** usually created by *process which may create two pairs of control   **
 ** and data file descriptors, only the data descriptors are stored here**
 ** the control descriptors are asc_fd associated.                      **
 *************************************************************************/
void buresetasc(fp, asc_fd)
FILE *fp; int asc_fd;
{     int fd;
      buresetlog(fp, 1);
      fd = fileno(fp);
      lisp_opened[fd].asc_fd = asc_fd;
}

/*************************************************************************
 ** (resetio) Will close all open files except the standard input/output**
 ** and error. It is useful when too many (load's) result in errors. And**
 ** provides a way of allowing more (load's) after we run out of fd's.  **
 *************************************************************************/
struct conscell *buresetio(form)
struct conscell *form;
{
    int i, ok = 1;
    if (form != NULL) ierror("resetio");
    for(i = 0; i < _NFILE; i++) {
       if (lisp_opened[i].flag > 0) {
           lisp_opened[i].flag = 0;
           if (lisp_opened[i].fp != NULL) {
              if (fclose(lisp_opened[i].fp) == EOF)
                 ok = 0;
              if (lisp_opened[i].asc_fd >= 0) {
                  close(lisp_opened[i].asc_fd);
                  lisp_opened[i].asc_fd = -1;
              }
           }
       }
    }
    if (ok)
        return(LIST(thold));
    else
        return(LIST(nilhold));
}
