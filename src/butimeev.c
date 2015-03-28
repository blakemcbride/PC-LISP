/* */
/*
 | PC-LISP (C) 1984-1990 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#ifdef _MSC_VER
#include <time.h>
#else
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include "lisp.h"

/*
 | <fix> <-- (time-eval <arg>)
 |
 | Will evaluate its argument and return the number of seconds of user CPU
 | time that it took.  The caluclation is done by subtracting the current
 | user seconds from previous seconds and the current micro seconds from the
 | previous micro seconds then passing the carry. Finally the result is
 | returned as a float in seconds.
 */
struct conscell *butimeev(form)
struct conscell *form;
{
       double diff;
#ifdef _MSC_VER
       clock_t start, end;
       if (!form || form->cdrp) goto er;
       start = clock();
       eval(form->carp);
       end = clock();
       diff = end - start;
#else
       struct rusage buf;
       long t0s, t1s, t0us, t1us;
       if (!form || form->cdrp) goto er;
       getrusage(RUSAGE_SELF, &buf);
       t0s = buf.ru_utime.tv_sec;
       t0us = buf.ru_utime.tv_usec;
       eval(form->carp);
       getrusage(RUSAGE_SELF, &buf);
       t1s = buf.ru_utime.tv_sec;
       t1us = buf.ru_utime.tv_usec;
       t1s -= t0s;
       t1us -= t0us;
       if (t1us < 0) { t1s -= 1; t1us += 1000000; }
       diff = (float) t1s + (t1us / 1000000.0);
#endif
       return(LIST(newrealop(diff)));
  er:  ierror("time-eval");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
