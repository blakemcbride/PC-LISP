/* EDITION AD04, APFUN PAS.802 (92/03/26 14:03:00) -- CLOSED */                 

/****************************************************************************
 **      PC-LISP (C) Peter Ashwood-Smith 1986.                             **
 **      MODULE ERRORS                                                     **
 ** ---------------------------------------------------------------------- **
 ** This module contains a series of functions which are called when an    **
 ** error condition occurs. Each handles a particular type of error. Eg    **
 ** ierror() handles an error evaluating a built in function. gerror() is  **
 ** for more specific info such as incorrect number of parms. serror() is  **
 ** for syntax errors, matherr is for math.h errors, fperr is for the sig- **
 ** nal SIGFPE. bkhit handles the 'error' generated when the user interrupt**
 ** s execution by hitting the BREAK key. UpError handles a fatal error in **
 ** the initialization phase and terminates the program.                   **
 ****************************************************************************/

#include <signal.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/****************************************************************************
 ** These are the old signal handlers. When initerrors is called we store  **
 ** the old settings here and when deiniterros is called we restore them.  **
 ** This is most useful in the embedded version of PC-LISP to allow the    **
 ** interpreter to trap errors that occur under its control and to let the **
 ** application take care of the rest.                                     **
 ****************************************************************************/
static int (*old_sigsegv)() = NULL;
static int (*old_sigint)()  = NULL;
static int (*old_sigfpe)()  = NULL;
static int (*errh)()        = NULL;

/****************************************************************************
 ** The general error message function. We throw the error to see if any   **
 ** errset will pick it up if not ThrowError will return after printing the**
 ** string. We then copy the hold stack, reset the mark stacks and tracing **
 ** flush the input buffer and longjump to the break level handler in main.**
 ****************************************************************************/
 gerror(s)
 char *s;
 {   if (errh) (*errh)(s,NULL);         /* (compile) uses override errhandler */
     HoldStackOperation(COPY_STACK);
     ThrowError(s);                     /* return to (errset) if one exists) */
     buresetlog(NULL, 2);               /* close all files opened by (load) */
     ClearMarkStacks();
     ResetTrace();
     fflush(stdin);                     /* limits cascade errors from stdin */
     jumptodebug();                     /* jump to debugger loop */
 }

/****************************************************************************
 ** ierror(s) : interpreter error while evaluating function whose name is  **
 ** stored in 's'. format the message and call the general error routine.  **
 ****************************************************************************/
 ierror(s)
 char *s;
 {    char buffer[256];
      sprintf(buffer,"error evaluating built in function [%s]",s);
      gerror(buffer);
 }

/****************************************************************************
 ** ioerror(p): I-O error while reading/writing/closing/opening file FILE  **
 ** p, generate an error message string clear the error and then call the  **
 ** general error handler function to throw the error to err system. If p  **
 ** is NULL the error was caused by I-O to a closed file. Otherwise we do  **
 ** not know what caused the error so we print the generic message and put **
 ** the file number in with it. If the errno is non zero and legal, we put **
 ** this message after the generic message. It may help the user.          **
 ****************************************************************************/
 ioerror(p)
 FILE *p;
 {    char buffer[256],*msg;
      extern int sys_nerr,errno;        /* standard UNIX error message stuff */
      extern char *sys_errlist[];
      if (p == NULL) gerror("I-O after close");
      clearerr(p);
      msg = ((errno > 0)&&(errno <= sys_nerr)) ? sys_errlist[errno] : "";
      sprintf(buffer,"I-O error on [%d] %s", fileno(p), msg);
      gerror(buffer);
 }

/****************************************************************************
 ** The fatal error message function. We format a fatal error message and  **
 ** call gerror to do the work. Note that we do not force an exit, rather  **
 ** we return to the break level and request the user exit.                **
 ****************************************************************************/
 fatalerror(s)
 char *s;
 {   char buffer[256];
     sprintf(buffer,"INTERNAL ERROR: in %s() (I suggest you quit)",s);
     gerror(buffer);
 }

/****************************************************************************
 ** catcherror called by throw() which may seem strange. What has happened **
 ** is that a throw has been issued with a tag that has no catcher so we   **
 ** come off the top and issue the error.                                  **
 ****************************************************************************/
 catcherror(s)
 char *s;
 {   char buffer[300];
     sprintf(buffer,"no catch for this tag [%s]",s);
     gerror(buffer);
 }

/****************************************************************************
 ** bindingerror called by eval(). The atom with print name pointed to by  **
 ** s is unbound, generate the appropriate error message.                  **
 ****************************************************************************/
 bindingerror(s)
 char *s;
 {   char buffer[300];
     sprintf(buffer,"unbound atom [%s]",s);
     gerror(buffer);
 }

/****************************************************************************
 ** The error message function. serror(s) Will inform user of a syntax err **
 ** it then calls for a COPY of the mark stack, resets any trace indent,   **
 ** and longjmps the hell out of the recursion into the break level 'er>'  **
 ** As with the other errors we format the error message and throw the err **
 ** to see if anyone will catch it, if not the ThrowError returns and we   **
 ** continue processing the error.                                         **
 ****************************************************************************/
 serror(l,s1,s2,num2)
 struct conscell *l; char *s1,*s2; int num2;
{    char buffer[MAXATOMSIZE + 256]; char anum[32];
     HoldStackOperation(COPY_STACK);
     if (s1 == NULL) s1 = "";
     if (s2 == NULL) s2 = "";
     if (num2 > 0) sprintf(anum,"(started on line# %d)", num2); else anum[0] = '\0';
     sprintf(buffer,"error: near line# %05d: %s %s : %s", ScanSetLineNum(1), s1, anum, s2);
     ThrowError(buffer);
     if (l != NULL) {
        printf("--- a close S-expression is ---\n");
        prettyprint(l,0,5,stdout);
        putchar('\n');
     }
     buresetlog(NULL, 2);               /* close all files opened by (load) */
     ClearMarkStacks();
     ResetTrace();
     fflush(stdin);                     /* limits cascade errors from stdin */
     jumptodebug();                     /* jump to debugger loop */
 }

/***************************************************************************
 ** stkovfl():We come here when a stack overflow occurs. This could be the**
 ** C stack overflowing, the mark stacks overflowing, or a SIGSEGV signal.**
 ** The C stack overflow is trapped on some implementations by extra.asm. **
 ** on others it never overflows before the mark stack and is not tested. **
 ** The mark stack overflow occurs when the eval and other stacks meet in **
 ** the middle of mystack. A stack overflow may trigger a SIGSEGV on some **
 ** machines so we may come here as a result of this signal. In any case  **
 ** the things we must do are the same and the problems similar. If called**
 ** from the middle of the garbage collector we will restart the collection*
 ** cycle and set marking=2. If another stack overflow occurs we are in   **
 ** a fix! We cannot mark everything. The most probable cause is a very   **
 ** long shallow binding stack. So rather than abort we unwind the scopes **
 ** back to the global level and try again. If this fails then we really  **
 ** are stuck so we tell the user he/she should quit right away. We must  **
 ** reset both stack tops mytop and emytop to either end of the mark stack**
 ** A stack overflow CAN NEVER BE caught with (errset) because the stack  **
 ** has been corrupted for any procedures nested deeper than the first 4  **
 ** or 5 below main (thats the price of my stack overflow recovery mech). **
 ** The debug dump of the mark stack usage is useful for systems whose C  **
 ** stack always overflows before the mark stack and allows you to set the**
 ** mark stack to a reasonable size, you can drop MSSIZE until they are   **
 ** both nearly full at same time.                                        **
 ***************************************************************************/
int stkovfl(cause)
    int cause;
{   extern int marking;
#   if DEBUG
    printf("--- mark stack usage = %d ---\n",mytop + (MSSIZE-emytop));
#   endif
    printf("\n--- Stack OverFlow");
    if (cause != 0) printf(" / Segmentation Violation (signal=%d)", cause);
    switch(marking)
    {   case 0 : HoldStackOperation(COPY_STACK);
                 ClearMarkStacks();
                 break;
        case 1 : printf(" in gc, will restart the gc! (showstack not current)");
                 printf("\nTHIS SHOULD NOT HAPPEN please forward following trace to developement!\n");
                 printf("--- mark stack usage = %d ---\n",mytop + (MSSIZE-emytop));
                 HoldStackOperation(COPY_STACK);
                 HoldStackOperation(DUMP_STACK);
                 ClearMarkStacks();
                 marking = 2; unmark(); mark(); gather(); marking = 0;
                 break;
        case 2 : printf(" again! Removing non global bindings and retrying!");
                 ClearMarkStacks();
                 unwindscope();
                 marking = 3; unmark(); mark(); gather(); marking = 0;
                 break;
        case 3:  printf(" again! MEMORY CORRUPT! YOU MUST QUIT RIGHT NOW!\n");
                 ClearMarkStacks();
                 break;
        default: fatalerror("stkovfl");
    };
    printf(" ---\n");
#   if SIGSEGVWORKS
       signal(SIGSEGV,stkovfl);
#   endif
    ResetTrace();
    jumptodebug();                      /* jump to debugger loop */
}

#if SIGFPEWORKS
/***************************************************************************
 ** A doubleing point exception occured ie signal SIGFPE. We do not know   **
 ** where this happened but the showstack gives the information so just   **
 ** abort and tell the user where to look for the information. Not every  **
 ** machine/compiler allows trapping this. Only MSC4.0 allows it under    **
 ** MSDOS, most unix'es allow this though.                                **
 ***************************************************************************/
int fperr(n)
int n;
{   signal(SIGFPE,fperr);
    gerror("floating point exception");
}
#endif

#if WANTERRNOTESTING
/***************************************************************************
 ** The errno value has changed and is now non zero. This was detected in **
 ** apply() after invoking a built in function. We will format an error   **
 ** message that perror would have done and then call gerror to handle the**
 ** error. This allows the error to become part of the normal LISP error  **
 ** mechanism. We check the value of errno to make sure it is legal ie in **
 ** the range 1..sys_nerr. If it is not we produce an 'unknown system err'**
 ** message, otherwise we extract the string from the table, format it,   **
 ** and dispatch the non returning function gerror().                     **
 ***************************************************************************/
int syserror()
{   char buffer[256];
    extern int sys_nerr,errno;        /* standard UNIX error message stuff */
    extern char *sys_errlist[];
    if ((errno > sys_nerr)||(errno < 1))
        sprintf(buffer,"apply: system error #%d ?",errno);
    else
        sprintf(buffer,"apply: %s",sys_errlist[errno]);
    errno = 0;
    gerror(buffer);
}
#endif

#if HASMATHERRFUNCTION
/***************************************************************************
 ** Standard Math library exception handler 'matherr' (see math.h). This  **
 ** routine is called when a math library function detects a problem, it  **
 ** simply decodes the exception->type field and prints an appropriate    **
 ** message. It then generates and internal error using the math function **
 ** name as the built in function causing the exception. Ierror then does **
 ** the stack copying and longjmps out of the error to the break level.   **
 ***************************************************************************/
int matherr(x)
struct exception *x;
{   char *s;
    switch(x->type)
    {   case DOMAIN   : s = "argument domain"; break;
        case SING     : s = "argument singularity"; break;
        case OVERFLOW : s = "overflow range"; break;
        case UNDERFLOW: s = "underflow range"; break;
        case TLOSS    : s = "total loss of significance"; break;
        case PLOSS    : s = "partial loss of significance"; break;
        default       : gerror("unknown math exception occured");
    };
    gerror(s);
}
#endif

/***************************************************************************
 ** UpError(s) : Death on initialization because of reason 's'. Must back **
 ** up to overwrite the 'wait..' on the console. Emit \010 backspace.     **
 ***************************************************************************/
UpError(s)
char *s;
{   printf("\010\010\010\010\010\010Abort: %s\n",s);
    exit(0);
}

/*
 | This little structure is used to store a stack of jump buffers. When the errors
 | are initialized, we push the old jump buffer on this stack so that the new one
 | after it has clobbered it and gone away, we can restore the old one with deinit
 | errors. This is part of the recursive entry to lidomac requirements.
 */
static struct jb_s {
              jmp_buf env;                /* copy of jump buffer to be restored */
              int (*segv)();              /* previous segmentation violation handler */
              int (*fpe)();               /* previous floating point handler */
              int (*intr)();              /* previous INTERRUPT handler */
              struct jb_s *next;          /* pointer to next thing in the stack */
       } *jb_tos = NULL;

/*
 | Push the current jump buffer environment onto the top of the stack. Allocate a
 | new top of stack and set it.
 */
static void push_env()
{
    struct jb_s *top = (struct jb_s *) malloc(sizeof(*top));
    assert(top);
#   if JMP_BUFISARRAY
       memcpy(top->env, env, sizeof(jmp_buf));
#   else
       memcpy(&(top->env), &env, sizeof(jmp_buf));
#   endif
    top->segv = old_sigsegv;
    top->fpe  = old_sigfpe;
    top->intr = old_sigint;
    top->next = jb_tos;
    jb_tos = top;
}

/*
 | Pop and restore the jump buffer environment from the top of the stack.
 */
static void pop_env()
{
    struct jb_s *top = jb_tos;
    assert(top);
#   if JMP_BUFISARRAY
       memcpy(env, top->env, sizeof(jmp_buf));
#   else
       memcpy(&env, &(top->env), sizeof(jmp_buf));
#   endif
    old_sigsegv = top->segv;
    old_sigfpe  = top->fpe;
    old_sigint  = top->intr;
    jb_tos      = top->next;
    free(top);
}

/*
 | Return TRUE if it is ok to do a LISP reboot now. We do not allow reboot unless
 | the nesting level in lidomac is exactly 0. This means that there are no environments
 | stored on the jump buffer stack.
 */
int reboot_ok()
{
    return(jb_tos == NULL);
}

/*
 | These variables are used to inhibit the setting of signal handlers.
 */
static int want_sigint   = 1;
static int want_sigsegv  = 1;
static int want_sigfpe   = 1;

/***************************************************************************
 ** brkhit(): We come here when the user has hit a key indicating that the**
 ** execution should be aborted. This may be a CONTROL-C or CONTROL-BREAK **
 ** in MSDOS or the SIGINT in UNIX or similar systems. To accomodate the  **
 ** simplest operating system (MSDOS) we must not do anything reentrant.  **
 ** To get around this problem the global variable bkhitcount is normally **
 ** 0 but will increment to 0 when a break condition occurs. Certain key  **
 ** routines test the value of this variable periodically and call brkhit **
 ** when it changes. Brkhit then handles the longjmp and logging of the   **
 ** mark stack data for use by (showstack). It then resets the brkhitcount**
 ** to zero to clear the break condition. In UNIX this is easy to do, we  **
 ** just call 'signal(SIGINT,brkhit)' and everything is taken care of. In **
 ** MSDOS this is not as easy because some compilers SIGINT is only the   **
 ** CONTROL-BREAK interrupt which is not much use because you can only get**
 ** this during I/O. What is more useful is the CONTROL-BEAK key. This is **
 ** handled by code in extra.asm which uses UpVector and DownVector to    **
 ** install a routine which increments the value of brkhitcount when the  **
 ** user hits the CONTROL-BREAK key. The interrupt is logged immediately. **
 ** And brkhit() will be called as soon as control arrives at a test point**
 ***************************************************************************/
#if SIGINTWORKS                         /* if SIGINT works for all breaks */
    int bkhitcount;                     /* this var is local */
#else
    extern int bkhitcount;              /* else it is declared in extra.asm */
#endif

int brktrap()                                     /* target of SIGINT interrupt */
{
    if (want_sigint)                              /* if currently monitoring them */
       bkhitcount += 1;                           /* increment for later testing*/
    else {                                        /* else if there is a sigint handler */
       struct jb_s *top;
       for(top = jb_tos; top != NULL; top = top->next)      /* walk up the stack trying to find someone */
           if (top->intr && (top->intr != brktrap))         /* elses SIGINT handler which we will call */
               return((top->intr)(SIGINT,NULL));
    }
}

int brkhit()                                      /* this routine actually processes the INTERRUPT */
{   bkhitcount = 0;                               /* from the evaluator loop */
#   if SIGINTWORKS
       signal(SIGINT,brktrap);                    /* reset for future signal catches */
#   endif
    gerror(INTERRUPT);                            /* handle the "Interrupt" error */
}

/*
 | Enable code to trap CONTROL-BREAK and CONTROL-C or SIGINT. These functions may
 | be called in a stack like manner 'initerrors ... initerrors .. deiniterrrs ...'
 | so we only do the installation on the frist level call but the error handler
 | jump buffer must be set and restored each time we enter and exit to allow a
 | recursive call to lidomac and lidomaca to work. This is necessary so that C
 | can call LISP can call C etc ....
 */
initerrors()
{
    bkhitcount = 0;
#   if SIGINTWORKS
       old_sigint = signal(SIGINT,brktrap);
#   else
       UpVector1();                            /* MSDOS assembly language CTRL-BREAK */
       UpVector2();                            /* and CTRL-C traps (they increment bkhitcount) */
#   endif
#   if SIGSEGVWORKS
       old_sigsegv = signal(SIGSEGV,stkovfl);
#   endif
#   if SIGFPEWORKS
       old_sigfpe = signal(SIGFPE,fperr);
#   endif
    push_env();                                /* push previous environemnt as we are about to create a new one */
}

/*
 | remove code to trap CTRL-BREAK and CTRL-C interrupts.
 */
deiniterrors()
{
    pop_env();                                 /* restore the previous jump buffer environment */
#   if SIGINTWORKS                             /* properly implement SIGINT */
       signal(SIGINT,old_sigint);              /* put back the old handler */
#   else
       DownVector1();                          /* take out CTRL-BREAK */
       DownVector2();                          /* take out CTRL-C */
#   endif
#   if SIGSEGVWORKS
       signal(SIGSEGV,old_sigsegv);            /* restore the old segv handler */
#   endif
#   if SIGFPEWORKS
       signal(SIGFPE,old_sigfpe);              /* restore the old float handler */
#   endif
}

/*
 | This routine is called after the general error has been generated and is used to
 | return control to the top level debugger. We do this by poping the env stack until
 | there is only one left on it at which time we jump to its environment. This is
 | necessary because there may be multiple levels of nesting of calls to lidomac and
 | we can only handle one break level at the moment hence we MUST jump to the top
 | level debugger.
 */
jumptodebug()
{   assert(jb_tos);
    lillev = 0;                                /* reset lexical (go) level */
    while(jb_tos->next != NULL) pop_env();     /* pop all but the top level environment */
    longjmp(jb_tos->env, 1);                   /* then jump to its setjmp debugger point */
}

/*
 | This code will allow a specific error trap to be disabled.
 */
disableerrors(er, enable)
    int er, enable;
{
    if (er == 0) want_sigint  = enable;
    if (er == 1) want_sigsegv = enable;
    if (er == 2) want_sigfpe  = enable;
}

/*
 | This function allows a caller to override the error handling function of LISP.
 | This function is used ONLY by the compiler and an abort MUST follow the error
 | since the shallow stacks are not rewound!
 */
lierrh(func)
    int (*func)();
{
    errh = func;
}
