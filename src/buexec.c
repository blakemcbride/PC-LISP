/* EDITION AD01, APFUN PAS.820 (92/04/16 16:14:28) -- CLOSED */                 
/* --- */

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
struct conscell *buexec(form)
struct conscell *form;
{      char *str,buff[2048];
       int mask, n = 0; struct conscell *r;
       *buff = '\0';
       while(form != NULL) {
           if (!GetString(form->carp,&str)) ierror("exec");
           if (sizeof(buff) <= (n += strlen(str) + 1)) ierror("exec");
           strcat(buff,str);
           strcat(buff," ");
           form = form->cdrp;
       }
#if 1
       mask = sigblock(sigmask(SIGCHLD));
       r = newintop(((long)system(buff)));
       sigsetmask(mask);
#else   /*  need to update this but not sure how right now.  */
       mask = sigprocmask(SIG_BLOCK, sigmask(SIGCHLD));
       r = newintop(((long)system(buff)));
       sigprocmask(SIG_SETMASK, mask);
#endif
       return(r);
}

