/* EDITION AB01, APFUN PAS.765 (91/12/10 16:57:46) -- CLOSED */                 

/*
 | PC-LISP
 */
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include "lisp.h"

/*
 | We use the network independent short/long format if available, otherwise just
 | store in machine format.
 */
#if HASTCP
#   include <sys/types.h>
#   include <netinet/in.h>
#else
#   define htonl(x) x
#   define ntohl(x) x
#endif

static r_bwrite();
static struct conscell *r_bread();


/*
 | libio.c - LISP binary input and output routines
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 |
 | libwrite(e,port,timeout) -> t   | nil : writes the S-expression 'e' to in binary.
 | libread(port,timeout)    -> (e) | nil : returns the S-expression (e) read in binary.
 |                                         both return nil if any error occurs.
 |
 | &
 |
 | (b-write e fp [timeout])           : from LISP calls libwrite
 | (b-read fp [timeout]) -> (e) | nil : from LISP calls libread
 |
 | These routines will allow the writing and reading of a binary form of LISP
 | S-expressions. This is useful when S-expressions are being written into
 | object files along with code for later retrieval at dynamic load time. This
 | can also be used to send binary S-expressions from one socket to another
 | more effieciently than the textual form.
 |
 | The basic format is pretty simple however it gets a little complicated when
 | we try to reduce the number of bytes needed. For example the list (a (b 10) d)
 | would be stored as:
 |
 |               LB AT1 'A' LB AT1 B FX1 10 LE AT1 D LE
 |
 | Where LB/LE => list begin/end
 |          AT => atom, AT0..AT40 are atom of length 0..40 respectively
 |          ST => string, ST0..ST40 are strings of length 0..40 respectively
 |          FX1=> 1 byte fixnum
 |          FX => a 4 byte fixnum
 |          TT => atom 't'
 |          .. => other frequently occurring atoms
 |          HK => hunks
 |          AR => arrays
 |          PT => opened port causes send of file.
 |          CL => clisp (evaled)
 |          CN => clisp (non-evaled)
 |          XX => fixfix cell
 |          .. => 22 unused opcodes for future expansion!
 */

/*
 | These are the prefix bytes used to indicate what follows them.
 */
#define         BASE  128            /* binary S-expression op codes are NOT ascii!! */
#define          NL   BASE+0         /* nil */
#define          LB   BASE+1         /* begin a list */
#define          LE   BASE+2         /* end a list */
#define          LD   BASE+3         /* end a list with dotted pair */
#define          AT   BASE+4         /* a NULL terminated atom */
#define          ST   BASE+5         /* a NULL terminated string */
#define          FX   BASE+6         /* a fixnum */
#define          FX1  BASE+7         /* 1 byte fixnum */
#define          FL   BASE+8         /* a floating point number */
#define          TT   BASE+9         /* atom t */
#define          QT   BASE+10        /* atom quote */
#define          SQ   BASE+11        /* atom setq */
#define          CO   BASE+12        /* atom cond */
#define          CS   BASE+13        /* atom cons */
#define          CR   BASE+14        /* atom car */
#define          CD   BASE+15        /* atom cdr */
#define          EQ   BASE+16        /* atom eq */
#define          NU   BASE+17        /* atom nul */
#define          HK   BASE+18        /* hunk */
#define          AR   BASE+19        /* array */
#define          PT   BASE+20        /* port ie a file */
#define          CL   BASE+21        /* CLISP/EVAL args code */
#define          LL   BASE+21        /* LAST OF ABOVE used for delta into AT0 ... */

/*
 | Optimizations on the above codes. We reserve 8 through 8+40 for atoms whose
 | lengths are 1..40 bytes long. And also a bank of 40 for strings of length
 | 0..40. Strings and atoms bigger than this are ST and AT respectively.
 */
#define          WID   40            /* width of STR and ATOM special byte codes */
#define          AT0  (LL+1)         /* 0 byte atom */
#define          ATL  (AT0+WID)      /* last special length atom */
#define          ST0  (ATL+1)        /* 0 byte string */
#define          STL  (ST0+WID)      /* last special length string STL should be exactly 0xff */

/*
 | Extended objects beyond 5.02 binary format. Must maintain binary format compatibility so
 | make sure none of the above numbers EVER change. Always extend... never change.
 */
#define          CN   STL+1          /* CLISP/NEVAL no eval args code */
#define          XX   STL+2          /* FIX / FIX cell */

/*
 | These opcodes are unused but reserved for future use, this fills the entire 128..255
 | range completely.
 */
#define          U1   XX + 1
#define          U2   XX + 2
#define          U3   XX + 3
#define          U4   XX + 4
#define          U5   XX + 5
#define          U6   XX + 6
#define          U7   XX + 7
#define          U8   XX + 8
#define          U10  XX + 9
#define          U11  XX + 11
#define          U12  XX + 12
#define          U13  XX + 13
#define          U14  XX + 14
#define          U15  XX + 15
#define          U16  XX + 16
#define          U17  XX + 17
#define          U18  XX + 18
#define          U19  XX + 19
#define          U20  XX + 20
#define          U21  XX + 21
#define          LAST U21           /* last code, used for assert(LAST <= 0xff) */

/*
 | Instead of a FILE *, the r_bread and r_bwrite use a SINK *, this contains
 | the FILE * but also a jump buffer for error handling and information about
 | the stream we are reading/writing from. This saves stack space because all
 | data is stored only once on the libread/libwrite routines stack.
 */
typedef struct sink_s {
          FILE    *fp;              /* where data is comming from/ going to */
          jmp_buf  erh;             /* where to longjmp on any error */
          int      mytop;           /* what to restore mytop to after longjmp */
          unsigned timeout;         /* how long to wait w/o response before timeout */
          unsigned otimeout;        /* old alarm timer value before we clobbered it */
          void    (*ofunc)();       /* old handler function before we clobbered SIGALRM */
          int      count;           /* number of calls to r_bread or r_bwrite so far for alarm rearming */
          unsigned issocket:1;      /* TRUE if fp is a TCP/IP socket ie (socketopen)'ed */
      } SINK;

/*
 | This next routine s_alarm(op, sink) will either initialize timeouts, clear
 | them rearm them, or handle the SIGALRM when it fires. If the SIGALRM occurs when
 | we are garbage collecting then we reshedule the alarm and return. This can happen
 | when the timeouts are short and lots of data is arriving into a large memory space
 | and the timer has not been rearmed because of the GC pause.
 */
#define          INIT   -1       /* initialize alarm to timeout seconds & save old one */
#define          TERM   -2       /* clear alarm and restore old value and handler */
#define          REARM  -3       /* rearm the timer since we got data recently */
#define          REARMC  256     /* rearm timer every 256 calls to r_bread or r_bwrite */

#if defined(SIGALRM)
    static void s_alarm(op, sink)
         int op; SINK *sink;
    {
         static SINK *s_sink;
         extern int marking;            /* variable from limman > 0 if in GC */
         extern unsigned alarm();       /* OS alarm function UNIX */
         switch(op) {
             case INIT:
                  s_sink = sink;
                  s_sink->ofunc = signal(SIGALRM, s_alarm);
                  s_sink->otimeout = alarm(s_sink->timeout);
                  break;
             case TERM:
                  signal(SIGALRM, s_sink->ofunc);
                  alarm(s_sink->otimeout);
                  break;
             case REARM:
                  alarm(s_sink->timeout);
                  break;
             case SIGALRM:
                  if (marking) {
                      alarm(s_sink->timeout);
                      signal(SIGALRM, s_alarm);
                      return;
                  }
                  longjmp(s_sink->erh, 1);
         }
    }
#else
#   define s_alarm(a,b)           /* MSDOS does not support SIGALRM */
#endif

/*
 | The leaf level write routines which dump byte, int, long, double and strings on error
 | longjmp to error handler.
 */
#define putbyte(_l,_sink)      { putc((_l), (_sink)->fp); }
#define putlong(_l,_sink)      { long ol = htonl(_l); if (fwrite((char *)&(ol),sizeof(long),1,_sink->fp) != 1)   longjmp(_sink->erh,1);}
#define putint(_l,_sink)       { long ol = htonl(_l); if (fwrite((char *)&(ol),sizeof(long), 1, _sink->fp) != 1)  longjmp(_sink->erh,1);}
#define putdouble(_d,_sink)    { if (fwrite((char *)&(_d),sizeof(double),1,_sink->fp) != 1) longjmp(_sink->erh,1);}
#define putstring(_s,_n,_sink) { if (fwrite((_s),(_n),1,(_sink)->fp) != 1)                  longjmp(_sink->erh,1);}

/*
 | Routine to extract an integer that preceeds a given pointer in memory. Word alignment is assumed on the
 | pointer. This is used to extract the length of the array of literals and code stored on clisp objects.
 */
#define intXtract(p)           (*(int *)((char *)(p) - sizeof(int)))
#define intXput(p,val)        ((*(int *)((char *)(p) - sizeof(int))) = (val))

/*
 | ports are written as <long> <byte> ..... where <long> is the number of
 | bytes to transfer. Basically we are sending the opened file byte by
 | byte. The truename however is lost and the receiver is expected to close
 | the file then move it wherever they want. We start by figuring out the
 | current position of the file pointer so that we can restore it after the
 | copy. Next we seek to the end of the file so that we know how many bytes
 | to write and generate the <long> size. We then enter the loop writing the
 | entire file from the current position. Finally we seek back to where we
 | were before doing the copy. We rearm the timout value (if it applies to
 | this sink) every 512 bytes.
 */
static putport(fp, sink)
     FILE *fp; SINK *sink;
{    long opos, size;
     if (fp == NULL) goto er;                             /* if closed goto error */
     opos = ftell(fp);                                    /* opos = original file pointer */
     if (opos < 0) goto er;
     if (fseek(fp, 0L, 2) != 0) goto er;                  /* seek to end of file to figure out how big it is */
     size = ftell(fp) - opos;                             /* size in bytes to write is file size - original position */
     if (size < 0) goto er;
     putlong(size, sink);                                 /* so write out the <long> size to sink */
     if (fseek(fp, opos, 0) != 0) goto er;                /* restore original file pointer before reading */
     while(--size >= 0) {                                 /* write out file from current position to */
         if (((size % REARMC) == 0)&&(sink->timeout > 0)) /* rearm timer periodically as we are */
             s_alarm(REARM,NULL);                         /* getting data if timeouts desired. */
         if (ferror(fp)) goto er;                         /* any errors reading fail */
         putbyte(getc(fp), sink);                         /* end of file byte by byte to sink */
     }
     if (fseek(fp, opos, 0) != 0) goto er;                /* restore original file pointer */
     return;                                              /* done so go home */
er:  longjmp(sink->erh, 1);                               /* error so jump to the sink's handler */
}

/*
 | clisps are written as C{L|N} <N> <lit1> ....<litN> <M> <byte1> ... <byteM>
 | We rearm the timout value (if it applies to this sink) periodically.
 */
static putclisp(clisp, sink)
     struct clispcell *clisp; SINK *sink;
{
     struct conscell **l; char *code; int size;
     l = clisp->literal;
     size = intXtract(l);                                 /* extract size from front of malloc'ed region */
     size = (size / sizeof(struct conscell *)) - 1;       /* convert it to an element count */
     putint(size, sink);                                  /* so write out the element count to sink */
     l += 1;                                              /* skip literal 0 when writing */
     while(--size >= 0) {                                 /* write out each literal expression */
         r_bwrite(*l++, sink);
         if (((size % REARMC) == 0)&&(sink->timeout > 0)) /* rearm timer periodically as we are */
             s_alarm(REARM,NULL);                         /* getting data if timeouts desired. */
     }
     code = clisp->code;
     size = intXtract(code);                              /* extract size from front of malloc'ed region */
     putint(size, sink);                                  /* so write out the <long> size to sink */
     while(--size >= 0) {                                 /* write out file from current position to */
         if (((size % REARMC) == 0)&&(sink->timeout > 0)) /* rearm timer periodically as we are */
             s_alarm(REARM,NULL);                         /* getting data if timeouts desired. */
         putbyte(*code++, sink);                          /* write next byte of code to sink */
     }
}

/*
 | Leaf read of a byte, on error longjump to error handler.
 */
static int getbyte(sink)
     SINK *sink;
{    register int l;
     l = getc(sink->fp);
     return(l);
}

/*
 | Leaf read of a long, on error longjump to error handler.
 */
static long getlong(sink)
     SINK *sink;
{    long l;
     if (fread((char *)&(l), sizeof(long), 1, sink->fp) == 1)
         return((long)ntohl(l));
     longjmp(sink->erh, 1);
}

/*
 | Leaf read of an int, on error longjump to error handler.
 */
static int getint(sink)
     SINK *sink;
{    long l;
     if (fread((char *)&(l), sizeof(long), 1, sink->fp) == 1)
         return((int)ntohl(l));
     longjmp(sink->erh, 1);
}

/*
 | Leaf read of a double on error longjump to error hander.
 */
static double getdouble(sink)
     SINK *sink;
{    double d;
     if (fread((char *)&(d), sizeof(double), 1, sink->fp) == 1)
         return(d);
     longjmp(sink->erh, 1);
}

/*
 | Leaf read of a string of length 'n' or until '\0' is read on error longjump
 | to error handler. -1 means read until \0 otherwise read exactly the given
 | number of bytes.
 */
static char *getstring(sink,n)
     SINK *sink;
{    static char s[MAXATOMSIZE]; register char *t = s; int m = MAXATOMSIZE-1;
     if (n != 0) {
         while(*t++ = getc(sink->fp)) {
            if (--m == 0) goto er;
            if (--n == 0) { *t = '\0'; break; }
         }
     } else
         s[0] = '\0';
     return(s);
er:  longjmp(sink->erh, 1);
}

/*
 | Read a port from the given sink. A port is formatted <long> <byte> .... where
 | the long gives the number of bytes that follow. First however we must create
 | an opened file descriptor into which we will copy the data. This descriptor is
 | then rewound and the opened descriptor is returned. We rearm the timout value
 | (if it applies to this sink) every 512 bytes.
 */
static struct conscell *getport(sink)
     SINK *sink;
{    FILE *fd;
     char fname[20];
     long size = getlong(sink);
     strcpy(fname, "tmpXXXXXX");
     int fn = mkstemp(fname);
     errno = 0;                                          /* tmpname leaves errno non zero */
     if (size < 0) goto er;
     fd = fdopen(fn,"w+");
     if (fd == NULL) goto er;
     while(--size >= 0) {
        if (((size % REARMC) == 0)&&(sink->timeout > 0)) /* rearm timer periodically as long as we */
             s_alarm(REARM,NULL);                        /* are getting data and timeouts desired */
        putc(getbyte(sink), fd);
     }
     rewind(fd);
     return(MakePort(fd,CreateInternedAtom(fname)));
er:  longjmp(sink->erh, 1);
}

/*
 | clisps are written as C{L|N} <N> <lit1> ....<litN> <M> <byte1> ... <byteM>
 | We rearm the timout value (if it applies to this sink) periodically.
 */
static struct clispcell *getclisp(sink)
     SINK *sink;
{    struct clispcell *clisp = NULL;
     struct conscell *r_bread(), **t, **l;
     char *bc, *code = NULL; int size;
     size = getint(sink)+1;                                                /* read size of literals array in bytes convert to elements */
     l = (struct conscell **)calloc(size + 1, sizeof(struct conscell *));
     if (l == NULL) goto er;                                               /* if no more memory we've got problems */
     t = l + 1;                                                            /* advance to literal[0] */
     intXput(t, size * sizeof(struct conscell *));                         /* write the byte size of the array 1 word before array */
     clisp = CLISP(new(CLISPCELL)); clisp->pure = 0;                       /* allocate the CLISP structure and populate it */
     clisp->eval = 1; clisp->literal = t;                                  /* with literal and data. Must handle the PURE bit */
     clisp->code = NULL;                                                   /* and eval bits later as part of binary protocol */
     xpush(clisp);                                                         /* don't gather this CLISP if gc occurs */
     *t++ = LIST(clisp);                                                   /* set literal[0] to point to self */
     size -= 1;                                                            /* then adjust count to skip literal[0] */
     while(--size >= 0) {                                                  /* read in the literals array */
         *t++ = r_bread(sink);
         if (((size % REARMC) == 0)&&(sink->timeout > 0))                  /* rearm timer periodically as we are */
             s_alarm(REARM,NULL);                                          /* getting data if timeouts desired. */
     }
     xpop(1);                                                              /* no risk of GCing the clisp till end of func */
     size = getint(sink);                                                  /* get the number of bytes of code */
     code = (char *)malloc(size + 1 + sizeof(int));
     if (code == NULL) goto er;                                            /* allocate array of code ofset by 1 int for storage of size */
     bc = code = code + sizeof(int);
     intXput(code, size);                                                  /* write the byte size of the array 1 word before array */
     clisp->code = code;                                                   /* clisp now references the code array */
     while(--size >= 0) {                                                  /* now start reading in the byte array 1 byte aat a time */
         *bc++ = getbyte(sink);
         if (((size % REARMC) == 0)&&(sink->timeout > 0))                  /* rearm timer periodically as we are */
             s_alarm(REARM,NULL);                                          /* getting data if timeouts desired. */
     }
     *bc = 0;                                                              /* always invalid instruction terminated */
     return(clisp);                                                        /* done so return he CLISP object */
er:  if (code) free(code);                                                 /* error so free up any malloced space we */
     if (l) free(l);                                                       /* cannot use */
     if (clisp) {                                                          /* if we created a new CLISP cell then we */
         clisp->celltype == CONSCELL;                                      /* must turn it into a CONSCEL so that the */
         LIST(clisp)->carp = LIST(clisp)->cdrp = NULL;                     /* GC will later pick it up without problems */
     }                                                                     /* this is because GC gather would free lit & code */
     longjmp(sink->erh, 1);                                                /* error so jump to the sink's handler */
}

/*
 | Recursively output the expression 'e' in binary format to the port or
 | socket fp, this is the exact inverse of the r_bread routine below.
 */
static r_bwrite(e, sink)
     struct conscell *e;
     SINK *sink;
{
     char *x; int n;

    /*
     | If timeouts are enabled on this sink then periodically rearm the timer
     | so that timeout value is reset. If we ever fail to get here in time the
     | timeout will occur and the transmission will be aborted unless we are in
     | the garbage collector in which case the timeout is rearmed and ignored.
     */
     if ((sink->timeout > 0) && ((sink->count++ % REARMC) == 0))
         s_alarm(REARM,NULL);

    /*
     | Nil is stored as byte NL
     */
     if (e == NULL) { putbyte(NL, sink); return; }

     switch(e->celltype) {

         /*
          | fixnums less than 256 are stored as FX1 <byte> if greather than 255 they
          | are stored as FX <long>
          */
          case FIXATOM   : if ((FIX(e)->atom >= 0) && (FIX(e)->atom <= 0xff)) {
                               putbyte(FX1, sink); putbyte( (int)(FIX(e)->atom & 0xff), sink);
                           } else {
                               putbyte(FX, sink); putlong(FIX(e)->atom, sink);
                           }
                           break;

         /*
          | fixfix cells are just stored as XX <long> <long>.
          */
          case FIXFIXATOM: putbyte(XX, sink);
                           putlong(FIXFIX(e)->atom1, sink);
                           putlong(FIXFIX(e)->atom2, sink);
                           break;

         /*
          | flonums are stored as FL <double>
          */
          case REALATOM  : putbyte(FL, sink); putdouble(REAL(e)->atom, sink);
                           break;

         /*
          | strings less than WID are stored as ST<n> "str" if greater than WID chars
          | long then they are stored as ST "str\0" this allows us to save a byte
          | for most small strings.
          */
          case STRINGATOM: n = strlen(STRING(e)->atom);
                           if (n > WID) { putbyte(ST,sink); n++; } else putbyte(ST0+n,sink);
                           if (n > 0) putstring(STRING(e)->atom, n, sink);
                           break;

         /*
          | alphaatoms are stored explicitly for t,quote,setq,cond,cons,null,car,cdr and eq
          | as TT,...EQ bytes. Since these are frequently occuring atoms this saves space
          | when saving function bodies. If not one of the above atoms then we save the
          | atom just like the strings above. If less than WID chars it is stored as AT<n>
          | "atom" otherwise it is stored as AT "atom\0".
          */
          case ALPHAATOM : if (e == LIST(thold)) { putbyte(TT, sink); break; }
                           n = strlen(ALPHA(e)->atom);
                           if ((n == 5) && (memcmp(ALPHA(e)->atom,"quote",5) == 0)) { putbyte(QT, sink); break; }
                           if (n == 4) { if (memcmp(ALPHA(e)->atom,"setq",4) == 0)  { putbyte(SQ, sink); break; }
                                         if (memcmp(ALPHA(e)->atom,"cond",4) == 0)  { putbyte(CO, sink); break; }
                                         if (memcmp(ALPHA(e)->atom,"cons",4) == 0)  { putbyte(CS, sink); break; }
                                         if (memcmp(ALPHA(e)->atom,"null",4) == 0)  { putbyte(NU, sink); break; }
                                       }
                           if (n == 3) { if (memcmp(ALPHA(e)->atom,"car", 3) == 0)  { putbyte(CR, sink); break; }
                                         if (memcmp(ALPHA(e)->atom,"cdr", 3) == 0)  { putbyte(CD, sink); break; }
                                       }
                           if ((n == 2) && (memcmp(ALPHA(e)->atom,"eq",  2) == 0))  { putbyte(EQ, sink); break; }
                           if (n > WID) { putbyte(AT,sink); n++; } else putbyte(AT0+n,sink);
                           if (n > 0) putstring(ALPHA(e)->atom, n, sink);
                           break;

         /*
          | lists are stored as LB <subexpr>.... LE if last element is not a dotted
          | pair, otherwise if last element is a dotted pair they are stored as
          | LB <subexpr> ..... LD <subexpr>.
          */
          case CONSCELL  : putbyte(LB, sink);
                           for(; ; e = e->cdrp) {
                               if (e == NULL) { putbyte(LE, sink); break; }
                               if (e->celltype != CONSCELL) {
                                   putbyte(LD, sink);
                                   r_bwrite(e, sink);
                                   break;
                               }
                               r_bwrite(e->carp, sink);
                           }
                           break;

         /*
          | hunks are stored as HK <int> <subexpr> ..... where the length of the
          | hunk is stored after HK and the subexprs are stored in reverse oreder.
          */
          case HUNKATOM  : putbyte(HK, sink);
                           n = HUNK(e)->size;
                           putint(n, sink);
                           while(--n >= 0) {
                               r_bwrite(*GetHunkIndex(e, n), sink);
                           }
                           break;

         /*
          | Arrays are stored as AR <info> <base> where <info> and <base> are both
          | just S-expressions themselves.
          */
          case ARRAYATOM : putbyte(AR, sink);
                           r_bwrite(ARRAY(e)->info, sink);
                           r_bwrite(ARRAY(e)->base, sink);
                           break;

         /*
          | ports are sent as PT <long> <byte> ..... where <long> is the number
          | of bytes to transfer. Basically we are sending the opened file byte by
          | byte.
          */
          case FILECELL  : if (PORT(e)->issocket) goto er;  /* cannot copy a socket */
                           putbyte(PT, sink);
                           putport(PORT(e)->atom, sink);
                           break;

         /*
          | CLISP cells are sent as C{L|N} <N> <lit1...> <litN> <M> <codebyte1> .... <codebyteM>
          */
          case CLISPCELL : if (CLISP(e)->eval == 1) {
                               putbyte(CL, sink);
                           } else {
                               putbyte(CN, sink);
                           }
                           putclisp(e, sink);
                           break;

          default        : goto er;
     }
     return;

    /*
     | Error occurred writing to sink so longjump to sink's error hander.
     */
 er: longjmp(sink->erh, 1);
}

/*
 | Recursively read the binary expression from the port or socket 'fp' and
 | return it. This is the exact inverse of the function r_bwrite so look at
 | the comments above to figure out what I am doing here.
 */
static struct conscell *r_bread(sink)
     SINK *sink;                      /* where to get data and how to behave */
{    struct conscell **p, *f;
     int i, c;

    /*
     | If timeouts are enabled on this sink then periodically rearm the timer
     | so that timeout value is reset. If we ever fail to get here in time the
     | timeout will occur and the transmission will be aborted unless we are in
     | the garbage collector in which case the timeout is rearmed and ignored.
     */
     if ((sink->timeout > 0) && ((sink->count++ % REARMC) == 0))
         s_alarm(REARM,NULL);

     switch(c = getbyte(sink)) {

         /*
          | Either EOF or NL token cause return of NULL by r_bread().
          */
          case EOF:
          case NL : return(NULL);

         /*
          | Trivial cases of true, long, small long, float, string ,atom, and
          | hard coded atoms quote, setq ....
          */
          case TT : return(LIST(thold));
          case FX : return(LIST(newintop(getlong(sink))));
          case FX1: return(LIST(newintop((long) (getbyte(sink) & 0xff))));
          case FL : return(LIST(newrealop(getdouble(sink))));
          case ST : return(LIST(insertstring(getstring(sink,-1))));
          case AT : return(LIST(CreateInternedAtom(getstring(sink,-1))));
          case QT : return(LIST(CreateInternedAtom("quote")));
          case SQ : return(LIST(CreateInternedAtom("setq")));
          case CO : return(LIST(CreateInternedAtom("cond")));
          case CS : return(LIST(CreateInternedAtom("cons")));
          case CR : return(LIST(CreateInternedAtom("car")));
          case CD : return(LIST(CreateInternedAtom("cdr")));
          case EQ : return(LIST(CreateInternedAtom("eq")));
          case NU : return(LIST(CreateInternedAtom("null")));

         /*
          | lists are stored as LB <subexpr>.... LE if last element is not a dotted
          | pair, otherwise if last element is a dotted pair they are stored as
          | LB <subexpr> ..... LD <subexpr>.
          */
          case LB : f = new(CONSCELL);
                    xpush(f);
                    f->carp = r_bread(sink);
                    for(p = &(f->cdrp) ; ; ) {
                        if ((c = getbyte(sink)) == LE) break;
                        if (c == LD) { *p = r_bread(sink); goto dot; }
                        ungetc(c, sink->fp);
                        *p = new(CONSCELL);
                        (*p)->carp = r_bread(sink);
                        p = &((*p)->cdrp);
                    }
                    *p = NULL;
               dot: xpop(1);
                    return(f);

         /*
          | hunks are stored as HK <int> <subexpr> ..... where the length of the
          | hunk is stored after HK and the subexprs are stored in reverse oreder.
          */
          case HK : i = getint(sink);
                    if ((i <= 0) || (i > (MAXATOMSIZE*2)/sizeof(char *))) goto er;
                    f = LIST(inserthunk(i));
                    xpush(f);
                    while(--i >= 0) {
                        (*GetHunkIndex(f, i)) = r_bread(sink);
                    }
                    xpop(1);
                    return(f);

         /*
          | Arrays are stored as AR <info> <base> where <info> and <base> are both
          | just S-expressions themselves.
          */
          case AR : f = LIST(new(ARRAYATOM));
                    ARRAY(f)->info = NULL;
                    ARRAY(f)->base = NULL;
                    xpush(f);
                    ARRAY(f)->info = r_bread(sink);
                    ARRAY(f)->base = HUNK(r_bread(sink));
                    xpop(1);
                    return(f);

         /*
          | ports are read as PT <long> <byte> ..... where <long> is the number
          | of bytes to read. First we open a new port object to write to and then
          | read the port into it.
          */
          case PT : return(getport(sink));

         /*
          | CLISP cells are sent as C{|N}L <lit1...> <litn> EL <M> <codebyte1> .... <codebyteM>
          | CL's have their eval bit set, CN's have the eval bit cleared.
          */
          case CL : f = LIST(getclisp(sink)); return(f);
          case CN : f = LIST(getclisp(sink)); CLISP(f)->eval = 0; return(f);

         /*
          | FIXFIX cells are sent as XX <long> <long>.
          */
          case XX : f = LIST(new(FIXFIXATOM));
                    FIXFIX(f)->atom1 = getlong(sink);
                    FIXFIX(f)->atom2 = getlong(sink);
                    return(f);

         /*
          | The default cases handles AT0..AT0+<WID> and ST0..ST0+<WID> tokens
          | AT0..AT<WID> represent atoms whose length is <WID> and whose bytes follow
          | without a \0 terminator. ST0..ST0+<WID> represent strings whose length is
          | <WID> and whose bytes follow without a \0 terminator. This compaction
          | scheme reduces the output binary form somewhat.
          */
          default : if ((c >= AT0) && (c <= ATL))
                        return(LIST(CreateInternedAtom(getstring(sink,c-AT0))));
                    if ((c >= ST0) && (c <= STL))
                        return(LIST(insertstring(getstring(sink,c-ST0))));
     }
 er: longjmp(sink->erh, 1);
}

/*
 | libwrite(e, port, timeout) will return 't' if 'e' was successfully written to
 | 'port' without every having to wait more than 'timeout' seconds between bytes.
 | Otherwise it returns nil.
 */
struct conscell *libwrite(e, port, timeout)
     struct conscell *e; struct filecell *port; long timeout;
{    SINK sink;
     struct conscell *r;
     sink.fp = port->atom;
     sink.mytop = mytop;
     sink.timeout = timeout;
     sink.count = 0;
     if (timeout > 0) s_alarm(INIT, &sink);
     if (setjmp(sink.erh))
         r = NULL;
     else {
         r_bwrite(e, &sink);
         r = LIST(thold);
     }
     mytop = sink.mytop;
     if (timeout > 0) s_alarm(TERM, NULL);
     return(r);
}

/*
 | libread(port, timeout) will return (e) if 'e' was successfully read from the
 | 'port' without every having to wait more than 'timeout' seconds between bytes.
 | Otherwise it returns nil.
 */
struct conscell *libread(port, timeout)
     struct filecell *port; long timeout;
{    SINK sink;
     struct conscell *r;
     sink.fp = port->atom;
     sink.mytop = mytop;
     sink.timeout = timeout;
     sink.count = 0;
     if (timeout > 0) s_alarm(INIT, &sink);
     push(r);
     if (!setjmp(sink.erh)) {
         r = r_bread(&sink);
         r = enlist(r);
     }
     xpop(1);
     mytop = sink.mytop;
     if (timeout > 0) s_alarm(TERM, NULL);
     return(r);
}

/*
 | The actual LISP primitive function (b-write expr port [timeout]) to binary print an
 | S-expression to the given port.
 */
struct conscell *bubwrite(form)
     struct conscell *form;
{    struct conscell *expr;
     struct filecell *p;
     long timeout = 0;
     assert(LAST <= 0xff);                                                        /* build sanity check, compiler should remove this */
     if (form != NULL) {
         expr = form->carp;
         if ((form = form->cdrp) != NULL) {
             p = PORT(form->carp);
             if ((form = form->cdrp) != NULL) {
                if (!GetFix(form->carp, &timeout)) goto er;
                if (form->cdrp != NULL) goto er;
             }
             if ((p != NULL) && (p->celltype == FILECELL) && (p->atom != NULL)) {
                 if (p->issocket && p->state == 1) rewind(p->atom);               /* if was reading socket rewind before writing */
                 p->state = 2;                                                    /* set new state to writing */
                 return(libwrite(expr, p, timeout));
             }
         }
     }
er:  ierror("b-write");
}

/*
 | The actual LISP primitive function (b-read port [timeout]) to binary read an
 | S-expression from the given port.
 */
struct conscell *bubread(form)
     struct conscell *form;
{    struct filecell *p;
     long timeout = 0;
     if (form != NULL) {
         p = PORT(form->carp);
         if ((form = form->cdrp) != NULL) {
             if (!GetFix(form->carp, &timeout)) goto er;
             if (form->cdrp != NULL) goto er;
         }
         if ((p != NULL) && (p->celltype == FILECELL) && (p->atom != NULL)) {
             if (p->issocket && p->state == 2) rewind(p->atom);                   /* if was writing socket rewind before reading */
             p->state = 1;                                                        /* set new state to reading */
             return(libread(p, timeout));
         }
     }
er:  ierror("b-read");
}

