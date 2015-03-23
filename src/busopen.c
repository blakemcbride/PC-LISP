

/*************************************************************************
 ** (socketopen ...) -> port will open a socket and return it as a port **
 ** which can then be passed to the normal LISP I/O routines to allow   **
 ** processes to exchange arbitrary LISP S-expressions. Note that the   **
 ** programmer must do (drain fp) operations between expressions and    **
 ** when the communications are to be turned around.                    **
 **                                                                     **
 ** In addition to the straight passive and active socketopen primitive **
 ** there is a function called busopenP(port | - count). Which can be   **
 ** used to accept and service asyncrounous read/eval/print requests.   **
 ** The idea is simple. A running tool complete with a graphical U.I    **
 ** cannot be blocked in socketopen waiting for requests all the time.  **
 ** To handle asyncrounous evaluation requests the application calls    **
 ** busopen(port) to set up a listener at port 'port' from any other    **
 ** machine on the network. The application is expected to have set up  **
 ** a SIGIO catcher function which it simply uses to increment a sigio  **
 ** count. It must then call busopenP(-1) whenever it determins that    **
 ** the SIGIO count has gone positive to give this code a chance to     **
 ** process all outstanding requests. From a GP based application this  **
 ** is normally done by doing a gp_force_return() from the handler after**
 ** incrementing the count: eg:                                         **
 **                                                                     **
 **       handler() {                                                   **
 **          if (busopenP(-2)) { sigios += 1; gp_force_return(); }      **
 **       }                                                             **
 **                                                                     **
 **       main() {                                                      **
 **          signal(SIGIO, handler);                                    **
 **          busopenP(2048);                                            **
 **          for(;;) {                                                  **
 **             gp_get_input(1,1,1,1);                                  **
 **             if (sigios > 0) {                                       **
 **                 sigios = 0;                                         **
 **                 busopenP(-1);                                       **
 **             }                                                       **
 **          }                                                          **
 **       }                                                             **
 *************************************************************************/

#include <stdio.h>
#include "lisp.h"

#if HASTCP                            /* only compile busopen/busopenP if TCP/IP available */

#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#if !defined(FD_SET)                  /* BERKELEY machines declare fd_set etc in types.h */
#   include <sys/select.h>            /* SYSV machines declare fd_set etc in select.h    */
#endif

#if defined(DEBUG)                    /* this is fairly complex code if you compile with -DDEBUG */
#   define Dprintf(a)  printf a       /* you will get some printf's at critical times. */
#else
#   define Dprintf(a)
#endif

/*************************************************************************
 ** fd_port, current port being used for READ/EVAL/PRINT operations. -1 **
 ** implies no port currently in use.                                   **
 *************************************************************************/
static int fd_port = -1;

/*************************************************************************
 ** fd_addr, address mask that we allow to connect to us asynchronously **
 ** default is 'anybody' i.e. INADDR_ANY.                               **
 ** implies no port currently in use.                                   **
 *************************************************************************/
static long fd_addr = INADDR_ANY;

/*************************************************************************
 ** fdwait(fd, sec, usec) return 1 iff and only if there is activity on **
 ** stream fd within sec/usec seconds. If sec is negative then this     **
 ** function blocks indefinitely until activity occurs on fd            **
 *************************************************************************/
static int fdwait(fd, sec, usec)
       int fd, sec, usec;
{      fd_set fdset;                                                      /* set up for select on socket or timeout */
       FD_ZERO(&fdset); FD_SET(fd,&fdset);                                /* this sets the fdset to have bit s1 on */
       if (sec < 0)
           return(select(fd + 1, &fdset, 0, 0, NULL) == 1);               /* block indefinitely until fd activity */
       else {
           struct timeval tv;
           tv.tv_sec = sec; tv.tv_usec = usec;
           return(select(fd + 1, &fdset, 0, 0, &tv) == 1);                /* block til fd activity or timeout */
       }
}

/*************************************************************************
 ** fdasync(fd) given a file descriptor make it operate asynchronously. **
 ** That is, we want SIGIOS to go to the current process. We do not want**
 ** any forked children to inherit this file descriptor because that can**
 ** stop listens from working so we set the close-on-exec flag on the fd**
 *************************************************************************/
static int fdasync(fd)
       int fd;
{
       fcntl(fd, F_SETFL, FASYNC);
       fcntl(fd, F_SETOWN, getpid());
       fcntl(fd, F_SETFD, 1);
}


/*************************************************************************
 ** name2addr(name) will return the internet address given a name. If   **
 ** the name is '*' the address is INADDR_ANY, if it is in a class A,B,C**
 ** form like 'NN.NN.NN.NN' inet_addr does the conversion for us, else  **
 ** we assume it is a named machine in which case we call gethostbyname **
 ** to do the dirty work of looking it up for us. A return value of 1   **
 ** indicates success and 0 means failure.                              **
 *************************************************************************/
static int name2addr(name, addr)
       char *name; long *addr;
{
       if (strcmp(name, "*") != 0) {
          *addr = inet_addr(name);
          if (*addr < 0) {
              struct hostent *host;
              if ((host = gethostbyname(name)) == NULL) return(0);
              memcpy(addr, host->h_addr, sizeof(*addr));
          }
       }
       else
          *addr = INADDR_ANY;
       return(1);
}

/*************************************************************************
 ** addrok(cl_addr, m_addr) will return '1' if the client addr cl_addr  **
 ** passes the mask addr m_addr. If mask addr is INADDR_ANY then all    **
 ** client addresses pass otherwise only those that match exactly pass. **
 *************************************************************************/
static int addrok(cl_addr, m_addr)
       long cl_addr, m_addr;
{
       if (m_addr != INADDR_ANY) {
           Dprintf(("checking mask addr=%s\n", inet_ntoa(m_addr)));
           Dprintf(("against  recv addr=%s\n", inet_ntoa(cl_addr)));
           if (m_addr == cl_addr) goto ok;
           Dprintf(("rejected!\n"));
           return(0);
       }
  ok:  return(1);
}

/*************************************************************************
 ** sopen(addr, port, wait, caddr) creates a simple interface to socket **
 ** routines. It returns a file descriptor that can easily be used for  **
 ** LISP I/O except that the I/O will go to the socket. This allows the **
 ** creation of inter communication LISP programs to be done relatively **
 ** easily. Once the connect is made the client's address is stored in  **
 ** the caddr parameter.                                                **
 *************************************************************************/
static FILE *sopen(addr, port, wait, caddr)
       long addr, port, wait; char *caddr;
{
       int len, on = 1, s1, s2 = -1; FILE *fp;
       struct sockaddr_in server, client;
       if ((s1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) goto er;                /* allocate a new socket using TCP/IP */
       bzero(&server, sizeof(server));
       server.sin_family = AF_INET;                                            /* set up our address as being TCP/IP */
       server.sin_port = htons( (short) port);                                 /* wait or connect to the following port */
       server.sin_addr.s_addr = addr;                                          /* at the following ethernet address N.N.N.N */
       if (setsockopt(s1,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0) goto er; /* want reuse of the address */
       if (wait != 0) {                                                        /* if timout not 0 seconds we are a passive connection */
	       if ( bind(s1, (struct sockaddr *)&server, sizeof(server)) < 0) goto er;  /* bind the address to the socket */
          if ( listen(s1, 5) < 0 ) goto er;                                    /* say we want to listen for connection */
          if ((wait > 0) || (wait == -1)) {                                    /* if we are to wait 'wait' secs or infinite then... */
              for(;;) {                                                        /* repeat while getting EINTR's */
                  if (!fdwait(s1, wait, 0)) goto er;                           /* wait at most wait seconds for activity on s1 */
                  len = sizeof(client);
                  if ((s2 = accept(s1,&client,&len)) >= 0) break;              /* if activity on s1 and got connection exit loop */
                  if (len != sizeof(client)) goto er;                          /* did we conenct to AF_INET addressee? */
                  if (errno != EINTR) goto er;                                 /* error occured, only one we expect is EINTR */
              }
              if (!addrok(client.sin_addr.s_addr, addr)) goto er;
              if (caddr) {strcpy(caddr, inet_ntoa(client.sin_addr));}          /* construct clients address for as a return parameter */
              close(s1);                                                       /* all ok, have a new fd s2 so can close s1 now */
          } else {                                                             /* we are not to wait (ie wait = -2), will process on SIGIO */
              fdasync(s1);                                                     /* so ask FD to give us a SIGIO when ready */
              s2 = s1;
          }
       } else {                                                                /* this is the active side of the connection ie wait = 0 */
          if (connect(s1, &server, sizeof(server)) < 0) goto er;               /* so try to make an active connect */
          s2 = s1;                                                             /* just copy s1 to s2 so that we end in same state as passive */
       }
       fp = fdopen(s2, "r+");                                                  /* convert the file descriptor s2 into a FILE * */
       if (fp) return(fp);                                                     /* if successful return it otherwse drop into error */
er:                                                                            /* various errors, clean up and exit */
       Dprintf(("\n\n*** busopen failed errno = %d ***\n", errno));
       if (s1 >= 0) close(s1);
       if (s2 >= 0) close(s2);
       return(NULL);
}

/*************************************************************************
 ** port <-- (socketopen "address" port# [wait_time] )                  **
 **                                                                     **
 ** Addresses like 47.20.10.4 are permitted as are full node names like **
 ** "bcrka398". The special address "*" is used to indicate a wild card **
 ** when making a passive connection and 'this node' when making active **
 ** connections. If a 0 (default) wait time is provided then we will    **
 ** make an active 'connect' call. If on the other hand a non zero wait **
 ** time is provided then an active 'accept' call is made but will time-**
 ** out after 'wait_time' seconds. A negative value for wait_time means **
 ** wait forever.                                                       **
 **                                                                     **
 ** There are several tricks to using the socket returned by accept/conn**
 ** -ect as a FILE * via fdopen. Basically the FILE * must be rewound   **
 ** whenever the I/O direction changes. This is done automatically by   **
 ** LISP read/readc... and print princ ... routines on a socket.        **
 ** A socket can be in one of three modes 0 => unknown. 1 => reading    **
 ** 2 => writing.                                                       **
 *************************************************************************/
struct conscell *busopen(form)
struct conscell *form;
{      char *iaddr; FILE *fd;
       struct conscell *fcell;
       struct hostent *host;
       char addr[MAXATOMSIZE + 1];
       long port, wait = 0, haddr;
       errno = 0;
       if ( (form != NULL) && GetString(form->carp,&iaddr) ) {
           form = form->cdrp;
           strcpy(addr, iaddr);
           if ( (form != NULL) && GetFix(form->carp, &port) ) {
               if (port <= IPPORT_RESERVED) goto er3;
               form = form->cdrp;
               if ((form != NULL) && !GetFix(form->carp, &wait)) goto er;
               if (wait < -1) wait = -1;                              /* -2 etc not allowed from LISP */
               if (!name2addr(addr, &haddr)) goto er2;
               if ((fd = sopen(haddr, port, wait, addr)) == NULL) return(NULL);
               buresetlog(fd, 1);
               fcell = MakePort(fd, CreateInternedAtom(addr) );
               PORT(fcell)->issocket = 1;
               PORT(fcell)->state = 0;                                /* unknown state will set on first read/write */
               return(fcell);
           }
       }
  er:  ierror("socketopen");
  er2: ierror("socketopen:bad address");
  er3: ierror("socketopen:reserved port");
}

/***************************************************************************************************
 ** This is the external busopenP(op) function. It is called with op > 0 to cause a socket open   **
 ** to port from any "*" address and op = -1 to process any pending data on that port by means    **
 ** of a read/eval/print loop. This allows an application to create a LISP interface to its tool  **
 ** through an TCP/IP port the same way it would through an ordinary stdio stream. This can       **
 ** operate in parallel with the cursor tracking and general U.I because it is all done through   **
 ** callbacks. We return 0 for success or -1 on failure. Note that we must loop as long as there  **
 ** is data available to be read on the various file descriptors.                                 **
 ***************************************************************************************************/
int    busopenP(op)
       int op;
{
       static FILE *fd_listen = NULL, *fd_talk = NULL; static int old_mask;

      /*
       | Must reset errno to see if I/O routines triggered an error.
       */
       errno = 0;

      /*
       | If being asked to open a port then remember the port and shut down any currently open listen
       | or talk sockets.
       */
       if (op > 0) {
           fd_port = op;
           if (fd_listen) { fclose(fd_listen); fd_listen = NULL; }
           if (fd_talk)   { fclose(fd_talk); fd_talk = NULL; }
           fd_listen = sopen(fd_addr, fd_port, -2, NULL);             /* open but don't wait for accept, set SIGIO tell us */
           Dprintf(("\n\n*** busopenP reopen fd_listen = %x errno = %d ***\n", fd_listen, errno));
           return(fd_listen == NULL);                                 /* return 0 for success -1 for failure */
       }

      /*
       | We are called with -2 when the caller wants to know if he needs to call busopen(-1) later
       | to process input. This function is usually called from the SIGIO handler to determine if the
       | SIGIO is of interest to us. If we do not do this we may get SIGIO's that we do not care about
       | especially when a file descriptor is ready for write which we do not care about.
       */
       if (op == -2) {
           if (fd_listen && fdwait(fileno(fd_listen), -1, 0) == 1) return(1);
           if (fd_talk && fdwait(fileno(fd_talk), -1, 0) == 1) return(1);
           return(0);
       }

      /*
       | An op of -1 means try to process one SIGIO.
       */
       if (op == -1) {
           Dprintf(("\tbusopenP processing next SIGIO\n"));

          /*
           | If fd_listen selector then event if there is data on the socket then try to accept a talk connection
           | on it and drop through to the talk case.
           */
           if (fd_listen != NULL) {
               int fdt, fdl = fileno(fd_listen);                              /* get the actual fdl descriptor from FILE * */
               Dprintf(("\tbusopenP accepting fd_listen & opening fd_talk\n"));
               if (fdwait(fdl, 0, 10) == 1) {                                 /* if activity on socket try to accept */
                  struct sockaddr_in client;
                  int len = sizeof(client);
                  Dprintf(("\tbusopenP data available on fd_listen\n"));
                  if ((fdt = accept(fdl, &client, &len)) >= 0) {              /* accept the connection, if error drop out */
                      Dprintf(("\tbusopenP connection accepted\n"));
                      fclose(fd_listen);                                      /* got accept don't need listner socket now so close it down */
                      fd_listen = NULL;                                       /* and clear FILE * so we do not enter this code on next liio(-1) */
                      if (addrok(client.sin_addr.s_addr, fd_addr)) {          /* if client address matches mask */
                          fdasync(fdt);                                       /* set up so that on I/O event send signal SIGIO */
                          fd_talk = fdopen(fdt, "r+");                        /* and open a FILE * equivalent to this talk file fd */
                      } else
                          close(fdt);                                         /* shut out offending client! */
                   }
               }
           }

          /*
           | Now try to process the talk socket if there is data on it. If socket is active it means that we
           | have just received an S-expression so read/eval and print it, just absorb any errors that occur.
           | Note that this is similar to an errset evaluation in that we return (list (eval (read fp))) on
           | success and Nil on failure. Note that I am using LIST(1) as an end of file expr.
           */
           if (fd_talk != NULL) {
               struct conscell *in, *out, *libread(), *libwrite();
               struct filecell p; int c, binary;
               extern jmp_buf env;
      again:   Dprintf(("\tbusopenP processing fd_talk\n"));
               if (fdwait(fileno(fd_talk), 0, 10) == 1) {                     /* if activity on talk then read/eval/print to it */
                   Dprintf(("\tbusopenP fd_talk has data\n"));
                   if (!feof(fd_talk) && !ferror(fd_talk) && (c = getc(fd_talk)) != EOF) {
                       Dprintf(("\tbusopenP fd_talk 1st char was %c/%#x\n",c,c));
                       push(in);                                              /* set up for marking */
                       binary = !isascii(c);                                  /* or textual. Binary's have op codes > 127 */
                       ungetc(c, fd_talk);                                    /* put back the first char since we'll need it */
                       if (!setjmp(env)) {                                    /* <-- IERROR TARGET */
                           initerrors();                                      /* initialize exception handlers */
                           p.celltype = FILECELL;
                           p.atom = fd_talk;
                           p.issocket = p.state = 0; p.fname = NULL;
                           if (binary) {                                      /* activity so read binary S-expression from socket */
                               in = libread(&p, 30);                          /* use a 30 second timeout */
                               in = (in == NULL) ? LIST(1) : in->carp;        /* if no valid S-expression result is bad ptr 1 else value */
                           } else
                               in = ReadExpression(&p, LIST(1), 1);           /* activity so read an text S-expression from socket */
                           if (in != LIST(1)) {                               /* if not EOF return(list(eval..)) */
                               Dprintf(("\tbusopenP got <expr> now evaling and writing back\n"));
                               rewind(fd_talk);                               /* must rewind before output */
                               out = enlist(eval(in));                        /* evaluate the expression and return as list (result) */
                               if (binary)                                    /* if working binary then write the S-expression back */
                                  libwrite(out, &p, 30);                      /* as a binary S-expression */
                               else {                                         /* otherwise if working textually then make resunt into */
                                  out = enlist(out);
                                  printlist(fd_talk,out,DELIM_OFF,NULL,NULL); /* and print the result back to the talk socket */
                                  putc('\n', fd_talk);                        /* need a terminator for read */
                               }
                           } else {                                           /* we got and EOF on the read so writer has closed */
                               Dprintf(("\tbusopenP closing fd_talk because LIST(1)\n"));
                               fclose(fd_talk);                               /* the socket so close our end and get ready to */
                               fd_talk = NULL;                                /* go back to the top and reopen it */
                           }
                       } else {
                           Dprintf(("\tbusopenP sending NULL error to fd_talk\n"));
                           if (binary)                                        /* syntax or other error so send a (NULL) back to sender */
                               libwrite(NULL, &p, 30);                        /* binary transmission */
                           else {                                             /* or textual transmission */
                               out = enlist(NULL);
                               printlist(fd_talk,out,DELIM_OFF,NULL,NULL);    /* send the (NULL) error result back to socket */
                               putc('\n', fd_talk);                           /* need a terminator for read */
                           }
                       }
                       xpop(1);                                               /* pop 'in' from the mark stack */
                       deiniterrors();                                        /* restore exception handlers */
                       if (fd_talk) {                                         /* flush and rewind for next cycle if still open */
                           Dprintf(("\tbusopenP flushing & rewinding fd_talk\n"));
                           fflush(fd_talk);                                   /* flush because want a write(2) to occur */
                           rewind(fd_talk);                                   /* and rewind because FILE * won't read properly */
                           goto again;                                        /* go back for more */
                       }
                   } else {
                       Dprintf(("\tbusopenP closing fd_talk (because read EOF)\n"));
                       fclose(fd_talk);                                       /* the socket so close our end and get ready to */
                       fd_talk = NULL;                                        /* go back to the top and reopen it */
                   }
               }
           }

          /*
           | If anything fishy going on on the talk descriptor then shut it down now.
           */
           if (fd_talk && (feof(fd_talk) || ferror(fd_talk))) {
               Dprintf(("\tbusopenP shutting down fd_talk due to EOF or I/O error\n"));
               fclose(fd_talk);
               fd_talk = NULL;
           }

          /*
           | If anything fishy going on on the listen descriptor then shut it down now.
           */
           if (fd_listen && (feof(fd_listen) || ferror(fd_listen))) {
               Dprintf(("\tbusopenP shutting down fd_listen due to EOF or I/O error\n"));
               fclose(fd_listen);
               fd_listen = NULL;
           }

          /*
           | If no talk or listen socket open then we just closed it above for some reason so reopen up a new listener.
           | It is possible that we may get errors opening the socket in which case we will retry for up to 30 seconds
           | at 1 second intervals. This can happen when the frame forks a child process which inherits the open listen
           | file descriptor.
           */
           if ((fd_talk == NULL) && (fd_listen == NULL)) {
               int i;
               for(i = 0; i < 30; i++) {
                   fd_listen = sopen(fd_addr, fd_port, -2, NULL);                                          /* open but don't wait for accept, let SIGIO tell us */
                   Dprintf(("\n\n*** busopenP reopen fd_listen = %x errno = %d ***\n", fd_listen, errno));
                   if (fd_listen != NULL) break;                                                           /* if opened ok the break out of retry loop */
                   sleep(1);                                                                               /* sleep for one second then retry */
               }
           }
       }

       return(0);
}

/*
 | (REP-socketopen [port [addr-mask])
 | This primitive will enable socket level communications on the given port, the previously
 | used port is returned. If no port is provided the READ/EVAL/PRINTing are currently disabled
 | and the previous port is returned. The addr-mask if provided is the internet address in
 | name or number format of the machine(s) that are allowed to make the connection. Normally
 | anybody may connect (INADDR_ANY) but specifying this will allow us to restrict it more.
 */
struct conscell *buREPsopen(form)
       struct conscell *form;
{
       int  isdflt, last_fd_port = fd_port;
       long addr, last_fd_addr = fd_addr;
       long port; void (*f)();

      /*
       | Simple case, no port has been specified so action is to shut down the socket. If no socket
       | is currently in use by REP then just return nil otherwise shut down the socket and return
       | its port value. We use busopenP(1) do do this by passing a known illegal port number, this
       | will force a shut down and an error restarting.
       */
       if (form == NULL) {
           if (last_fd_port <= IPPORT_RESERVED) return(NULL);
           fd_addr = INADDR_ANY;                              /* restore default addr mask to 'anyone' */
           busopenP(1);
           goto ret;
       }

      /*
       | Complex case, get the port parameter which must be a fixnum in the range IPPORT_RESERVED .. and
       | non negative. Also make sure that there is a signal handler for SIGIO's otherwise we are in an
       | application that does not support asynchronous read/eval/print from sockets and must inform user
       | of the fact.
       */
       if (!GetFix(form->carp, &port)) goto er;
       if ((port < 0) || (port <= IPPORT_RESERVED)) goto er1;
       isdflt =  ((f = signal(SIGIO, SIG_DFL)) ==  SIG_DFL);      /* get value of signal handler for SIGIO, if none REP not allowed */
       if (isdflt) goto er2;                                      /* if still set to default then won't work so get out */
       signal(SIGIO, f);                                          /* restore signal handler we only wanted to test its value */

      /*
       | If optional addr-mask is specified take the parameter and ask name2addr to convert it to an internet address
       | for us otherwise use the default 'anybody' mask of INADDR_ANY.
       */
       form = form->cdrp;
       if (form) {
          char *addr_s;
          if (!GetString(form->carp, &addr_s)) goto er4;
          if (form->cdrp) goto er;
          if (!name2addr(addr_s, &addr)) goto er4;
       } else
          addr = INADDR_ANY;

      /*
       | Now, try to open the new port/addr as a REP server. If we suceed all is well, just return the previous
       | port/addr pair.
       */
       fd_addr = addr;
       if (busopenP((int)port) == 0) goto ret;

      /*
       | We failed to open the new port so try to reopen the previous port and return NULL to indicate a
       | failed attempt.
       */
       if (last_fd_port > IPPORT_RESERVED) { fd_addr = last_fd_addr; busopenP(last_fd_port); }
       return(NULL);

      /*
       | Normal exit, new (port/addr-mask) opened, return the previous (port/addr-mask) pair now. If port
       | was not previously opened then return (nil nil).
       */
ret:   form = new(CONSCELL);
       xpush(form);
       form->cdrp = new(CONSCELL);
       if (last_fd_port > IPPORT_RESERVED) {
           form->carp = newintop((long) last_fd_port);
           form->cdrp->carp = LIST(insertstring((last_fd_addr == INADDR_ANY) ? "*" : inet_ntoa(last_fd_addr)));
       }
       xpop(1);
       return(form);

      /*
       | Normal error handling.
       */
  er:  ierror("REP-socketopen");
 er1:  ierror("REP-socketopen: illegal port");
 er2:  ierror("REP-socketopen: no SIGIO handler present");
 er4:  ierror("REP-socketopen: bad address mask");
}

#else  /* dummy out these routines on machines that do not support TCP/IP */

    void busopen()     { printf("TCP/IP not supported on this platform\n"); exit(-1); }
    void busopenP()    { busopen(); }
    struct conscell *buREPsopen() { return NULL; }

#endif
