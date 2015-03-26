

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

static int trindent = 0;
/*************************************************************************
 ** ResetTrace() - reset the tracing indent to 0. This is used by the   **
 ** next two functions to control the increasing indent with depth.     **
 *************************************************************************/
void ResetTrace()
{      trindent = 0;
}

/*************************************************************************
 ** EnterTrace(fn,largs) - print a trace message that we are entering   **
 ** function fn with arguments largs.                                   **
 *************************************************************************/
void EnterTrace(fn,largs)
struct conscell *fn,*largs;
{      int i;
       for(i=0;i<trindent;i++) putchar(' ');
       printf("<enter> ");
       printatom(stdout,fn,DELIM_ON,NULL);
       putchar(' ');
       printlist(stdout,largs,DELIM_ON,NULL,NULL);
       putchar('\n');
       trindent++;
}


/*************************************************************************
 ** ExitTrace(fn,result) - print a trace message that we are leaving    **
 ** function fn with result list 'result'.                              **
 *************************************************************************/
void ExitTrace(fn,result)
struct conscell *fn,*result;
{      int i;
       trindent--;
       for(i=0;i<trindent;i++) putchar(' ');
       printf("<EXIT>  ");
       printatom(stdout,fn,DELIM_ON,NULL);
       putchar(' ');
       printlist(stdout,result,DELIM_ON,NULL,NULL);
       putchar('\n');
}

/***************************************************************************
 ** function GetTraced():    This function will make a list of all known  **
 ** atoms that have the trace bit set. This is used by built in functions **
 ** trace and untrace for setting and unsetting bits.                     **
 ***************************************************************************/
 struct conscell *GetTraced()
 {      struct conscell *l,*n,*w;
        struct alphacell *a; int i;
        extern struct conscell *atomtable[];
        push(l); push(w);
        for(i=0;i < ALPHATABSIZE;i++)
        {   w = atomtable[i];
            while(w != NULL)
            {     a = ALPHA(w->carp);
                  if ((a!=NULL)&&(a->celltype == ALPHAATOM))
                  {   if (a->tracebit == TRACE_ON)
                      {   n = new(CONSCELL);
                          n->carp = LIST(a);
                          n->cdrp = l;
                          l = n;
                      };
                  };
                  w = w->cdrp;
            };
        };
        fret(l,2);
 }


