/* */
#include <stdio.h>
#include <signal.h>
#include "lisp.h"

/*
 | Stand alone PC-LISP interpreter. The mainline simply calls the embedded
 | version of the interpreter either interactively or on the single file argument.
 | We also redefine the buexit() function to actually do an exit as exiting from
 | the embedded version of the interpreter is not allowed but we wish it to work
 | properly here.
 */
void SsInstall() { }

static int metrics = 1;     /* do we want metrics/auth to run ? */

/*
 | Come here on exit. Just shut down the metrics gathering.
 */
void doexit(n)
     int n;
{
     int  rc;
     char release[21];

#    if  !defined(NMETRICS)
         if ( metrics ) {
             pclispVersion( release );
             athend( "PC-LISP", release );
             mxend( &rc );
         }
#    endif
     exit(n);
}

/*
 | Catch SIGIO ready events. Don't do anything with them, just return, the sigpause will then
 | return and the event will later be processed.
 */
static int sigios = 0;

static void sigio_catcher(sig)
{
     sigios += 1;
     signal(SIGIO, sigio_catcher);
}

/*
 | Handle calls to the (exit) primitive it is allowed in the standalone interpreter
 | but is not allowed in embedded systems.
 */
void buexit(form)
     struct conscell *form;
{    long rc;
     if ((form != NULL) && GetFix(form->carp, &rc))
          doexit((int) rc);
     doexit(0);
}

/*
 | Some MX static stuff.
 */
static char mxl[4][8] = { "NOLIB", "", "", "" };

/*
 | The mainline. just take first argument and use it as the name of the file to
 | be loaded.
 */
void main(argc, argv)
     int argc;
     char *argv[];

{    int    rc;
     extern char *getenv();
     char   release[21];

    /*
     | This is a back door to turn off metrics via an environment variable.
     */
     metrics = (getenv("XXXXXXX") == NULL);

    /*
     | Get permission to run PC-LISP and start recording the fact that it
     | is being used.
     */
#    if  !defined(NMETRICS)
         if ( metrics ) {
             pclispVersion( release );
             athbeg( "PC-LISP", release );
             mxinit(&rc, "XXXXXXX", "PC-LISP", "lisp.out", release);
             mxbeg(&rc, mxl, "N/A", "N/A", "NONE", "TEXT");
         } else
             printf("XXXXXXX\n");    /* just to let us know we have bypassed auth/mx! */
#    endif

    /*
     | This should never happen unless someone screws up in an exec call.
     */
     if (argc <= 0) doexit(0);

    /*
     | An argument was not provided so run interactive.
     */
     if (argc == 1) {
        liinit("xxxsrc", "");
        limacro(NULL, NULL);
        doexit(0);
     }

    /*
     | Arguments were provided so remember them for command-line-args and run the first
     | argument through the interpreter. A special case occurs when we are running liszt.l
     | we do NOT load pclispsrc directly, rather we let liszt load it so that it can run it
     | from an errset so as to properly trap SIGINT's without dropping to the debugger.
     */
     liargs(argc, argv);
     if ((argc >= 2) && (strcmp(argv[1], "liszt.l") == 0))
         liinit("liszt.l", "");
     else {
         char *p; int port;
         signal(SIGIO, sigio_catcher);
         if (p = getenv("CAD_PORT")) {             /* AFTER loading scripts if CAD_PORT set, run async READ/EVAL/PRINT loop off port */
             port = atoi(p);
             if (busopenP(port) == 0) {            /* open the actual port asynchronously */
                 liinit("pclispsrc", argv[1]);     /* load scripts and afterwards run the REP loop */
                 while(1) {                        /* REP loop */
                    sigpause(0);                   /* wait for expression to evaluate */
                    while(sigios > 0) {
                        busopenP(-1);              /* READ/EVAL/PRINT the TCP/IP data now */
                        sigios -= 1;
                    }
                }
             } else
                printf("CAD_PORT = %d not available for TCP/IP communications\n", port);
         } else
             liinit("pclispsrc", argv[1]);            /* run script with NO TCP/IP/REP loop */
     }
     doexit(0);
}
