

/*
 | PC-LISP (C) 1989-1992 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#ifndef _MSC_VER
#include <sgtty.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif
#include "lisp.h"

/*************************************************************************
 ** first_process if TRUE causes any of these *process commands to add  **
 ** a sigchld handler if there is not one so as to avoid <defunc> pid's **
 *************************************************************************/
#ifndef _MSC_VER
static void sigchld_handler()
{      int status; struct rusage rusage;
       signal(SIGCHLD,sigchld_handler);
       wait3(&status, WNOHANG, &rusage);
}
#endif

/*************************************************************************
 ** (*process command [t|nil] [t|nil]).                                 **
 ** With 1 argument is exactly like buexec() except that the arguments  **
 ** have already been evaluated. If the second argument is non nil then **
 ** command is run as a process with an opened read pipe. If the third  **
 ** argument is non nil then command is created with an opened write    **
 ** pipe. If read/write ports are asked for then a list of the form     **
 ** (readport writeport pid) is returned.                               **
 *************************************************************************/
struct conscell *buprocess(form)
struct conscell *form;
{
#ifndef _MSC_VER
       char *str, *s, *t; FILE *fd_pr = NULL, *fd_pw = NULL;
       struct conscell *h, *n, *buexec();
       char fname[MAXATOMSIZE + 50]; int i, tty, tty2, pid;
       int want_pr = 0, want_pw = 0; char ttybuf[16], ptybuf[16];
       static int first_process = 1;
#if !defined(__linux__)
       struct sgttyb sgttyb;
#endif

      /*
       | If no args provided just "exec" the command. Otherwise get the command 'str'
       | argument and the readflag and writeflag options.
       */
       if (form == NULL) goto er;
       if (form->cdrp == NULL) return(buexec(form));
       if (!GetString(form->carp, &str)) goto er;
       form = form->cdrp;
       if (form->carp) want_pr = 1;
       form = form->cdrp;
       if (form && form->carp) { want_pw = 1; if (form->cdrp) goto er; }

      /*
       | Open the master pseudo tty to which this LISP process will be reading
       | and writing. Try all possible pseudo TTY's until a free one is found
       | or we exhaust the normal set.
       */
       for(s = "pqrs"; *s != '\0'; s++) {
           for(t = "0123456789abcdef"; *t != '\0'; t++) {
               sprintf(ptybuf, "/dev/pty%c%c", *s, *t);
               if ((tty = open(ptybuf, O_RDWR)) >= 0) {
                    strcpy(ttybuf, ptybuf);
                    ttybuf[5] = 't';
                    tty2 = open(ttybuf, O_RDWR);
                    goto found;
                }
           }
       }

      /*
       | Exhausted all possible pseudo TTY's so just get out now.
       */
       goto er;

found:

      /*
       | Set some modes. First we prefer to use the REMOTE process mode so that read/writes are aligned
       | but this does not seem to work on the RS so we cannot use it. We also want exclusive access to
       | the PTY so that fast opens/closes by multiple processes will not accidentally get the same PTY
       | and get their inputs multiplexed. Finally we want to make sure the input/output queues are empty
       | since the last user of the PTY sometimes leaves garbage. (I think this is a bug?).
       */
#      if ! defined(RS6000)  &&  ! defined(__linux__)
            i = 1; ioctl(tty, TIOCREMOTE, &i);
#      endif
       i = 1; ioctl(tty, TIOCEXCL,  &i);
#ifndef __linux__
       i = 0; ioctl(tty, TIOCFLUSH, &i);
#endif

      /*
       | If never called before make sure there is at lease a sigchld handler installed to avoid <defunct>
       | zombie processes.
       */
       if (first_process) {
           void (*f)();
           first_process = 0;
           f = signal(SIGCHLD, sigchld_handler);
           if (f != SIG_DFL) signal(SIGCHLD, f);
       }

      /*
       | Fork a child and in the child run /bin/sh -c "str". Must set the no echo mode and no /n => /r
       | substitution modes on the slave PTY. Then dup the PTY as stdin/stdout and stderr. If no read
       | port is desired then the stdout and stderr are remapped to /dev/null and if no write port is
       | desired then the stdin comes from /dev/null.
       */
       if ((pid = vfork()) == 0) {
#ifdef __linux__
           struct termios ts;
           tcgetattr(tty2, &ts);
           ts.c_lflag &= ~ECHO;
           tcsetattr(tty2, TCSANOW, &ts);
#else
           ioctl(tty2, TIOCGETP, &sgttyb);
           sgttyb.sg_flags &= ~ECHO;
           sgttyb.sg_flags &= ~CRMOD;
           ioctl(tty2, TIOCSETP, &sgttyb);
#endif
           dup2(tty2, 0); dup2(tty2, 1); dup2(tty2, 2);
           if (!want_pw) { dup2(open("/dev/null", O_RDWR), 0); }
           if (!want_pr) { dup2(open("/dev/null", O_RDWR), 1); dup2(1,2); }
           for(i = getdtablesize(); i > 3; close(--i));
           execl("/bin/sh", "/bin/sh", "-c", str, NULL);
           perror("*process/child");
           _exit(-4);
       }

      /*
       | If for some reason we cannot fork print out the ERRNO, close down the master and slave PTY
       | halves and return NIL.
       */
       if (pid < 0) {
           perror("*process/fork");
           close(tty);
           close(tty2);
           goto er;
       }

      /*
       | Now assign the file descriptors for write by parent to child and read by parent from child
       | to FILE * objects. Also close up the other halves of the pipe which we are no longer using
       | since only the child requires the other ends of the pipe.
       */
       if (want_pw) {
           fd_pw = fdopen(tty, "w");
           if (want_pr) tty = dup(tty);   /* must dup so that both fd_pw and fd_pr can be fclose'd */
       }
       if (want_pr)
           fd_pr = fdopen(tty, "r");

      /*
       | Structure the output list of the form (readport | nil writeport | nil pid) and make sure
       | that the two new opened file descriptors are logged for future (resetio) calls. We actually
       | create 3 new file descriptors, the third is a just the slave pty's descriptor which we must
       | keep open until the two parent descriptors are closed. This is done by storing the descriptor
       | number in the printname of the port and then parsing this out during the close and actually
       | closing both descriptors. We chose *%d) as the delimiters for the number so that close can
       | easily find a file name with a '*' (which is almost unheard of) and then parse out the control
       | port and close it. If both read & write ports are required we get two control ports.
       */
       push(h);
       h = n = new(CONSCELL);
       h->carp = newintop((long) pid);
       n = new(CONSCELL);
       n->cdrp = h;
       h = n;
       if (fd_pw) {
           sprintf(fname, "write (%d*%s) => '%s'", tty2, ptybuf, str);
           n->carp = LIST(MakePort(fd_pw, CreateInternedAtom(fname)));
           buresetasc(fd_pw, tty2);
           if (fd_pr) tty2 = dup(tty2);
       }
       n = new(CONSCELL);
       n->cdrp = h;
       h = n;
       if (fd_pr) {
           sprintf(fname, "read (%d*%s) <= '%s'", tty2, ttybuf, str);
           n->carp = LIST(MakePort(fd_pr, CreateInternedAtom(fname)));
           buresetasc(fd_pr, tty2);
       }

      /*
       | Pop mark stack and return the list we just created.
       */
       xpop(1);
       return(h);
#endif /*  _MSC_VER  */
er:    ierror("*process");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
