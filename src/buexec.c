

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <signal.h>
#include "lisp.h"

/*************************************************************************
 ** (exec -"str"-) Will concatenate each of the str parameters with a   **
 ** space between each and then call system to execute the command. It  **
 ** returns the result of the system call. During the call we explicitly**
 ** mask out SIGCHLD signals otherwise the sigchld handler may cause the**
 ** wait to miss the child process and block forever.                   **
 *************************************************************************/
struct conscell * buexec(struct conscell *form)
{      char *str,buff[2048];
       int n = 0; struct conscell *r;
       *buff = '\0';
       while(form != NULL) {
           if (!GetString(form->carp,&str)) ierror("exec");
           if (sizeof(buff) <= (n += strlen(str) + 1)) ierror("exec");
           strcat(buff,str);
           strcat(buff," ");
           form = form->cdrp;
       }
#ifdef _MSC_VER
       r = newintop(((long)system(buff)));
#else
       {
           sigset_t mask, omask;
           sigemptyset(&mask);
           sigaddset(&mask, SIGCHLD);
           sigprocmask(SIG_BLOCK, &mask, &omask);
           r = newintop(((long)system(buff)));
           sigprocmask(SIG_SETMASK, &omask, NULL);
       }
#endif
       return(r);
}

