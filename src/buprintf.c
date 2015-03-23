/* */
/*
 | PC-LISP (C) 1984-1990 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include "lisp.h"

/*
 | Emit characters to '*buff' from '*s' as long as we do not meet a spec '%' or
 | the \0. If we meet a % that is not followed by a % then stop otherwise emit
 | a single % and keep going. When done '*s' points to the first char in the
 | spec to be processed (done by next routine) or '\0' and *buff points to the
 | end of the buffer written (ie it has been advanced and null terminated).
 */
static int emit_to_spec(left, buff, s)
       int  *left;                          /* bytes left in buffer for our use */
       char **buff;                         /* pointer to buffer pointer */
       char **s;                            /* pointer to spec pointer */
{
       register char *t;
       register char *x = *buff;

       for(t = *s; *t != '\0'; t++) {       /* for each char in spec */
           if ((*left)-- <= 0) goto er;     /* if no more room in buffer then get out */
	   if (*t != '%')                   /* if not %d %x ... */
	       *x++ = *t;                   /* char is just emitted to buff */
	   else {                           /* else it is a %d...*/
	       t += 1;                      /* skip % to look ahead by one */
	       if (*t == '\0') goto er;     /* %\0 is an error in spec */
	       if (*t == '%')               /* but make sure not %% */
		  *x++ = '%';               /* because that's just a single % */
	       else {                       /* if not %% then is real spec so */
		  t -= 1;                   /* back up to % and then */
		  break;                    /* stop so next routine can handle */
	       }
	   }
       }
       *s = t;                              /* advance pointer for next routine */
       *x = '\0';                           /* null terminate buffer */
       *buff = x;                           /* advance buffer pointer */
       return(1);                           /* return success */
 er:   return(0);
}

/*
 | Emit the next arg in the arg list '*form' according to the spec '*s' and
 | advance *s to the end of the spec when done. Basically we loop through every
 | character up to the end of a spec string. long parameters in form have specs
 | like %<stuff>{d|x|o|u}. Double parameters in form have specs similar to the
 | long but terminated with {f|e|g}, string are terminated with {s} and characters
 | with {c}. The <stuff> may consist of {#,-,+, ,0-9,.,l} and nothing else. Note
 | that '*' is deliberately not supported! Once we have extracted the full spec
 | and made sure it is reasonably valid and extracted the corresponding parameter
 | for it from 'form' we simply call fprintf with the spec and argument.
 */
static int emit_next_spec(left, buff, s, form)
       int  *left;                                  /* room left in buffer in bytes */
       char **buff;                                 /* pointer to buffer sweep pointer */
       char **s;                                    /* pointer to spec sweep pointer */
       struct conscell **form;                      /* argument list */
{
       register char *t;                            /* 't' is what we sweep through spec */
       long   ival;                                 /* long value if %d ... */
       double fval;                                 /* double value if %f ... */
       char  *sval;                                 /* string value if %s ... */
       int    c;                                    /* temporary char for char swap */
       int    len;                                  /* length of addition to buffer */

      /*
       | We are about to process a spec %... something so there must be at least one
       | argument available.
       */
       if (*form == NULL) goto er;

      /*
       | Loop through all chars in the spec string from one after the % until we get one of
       | the spec terminating characters which we then process and then drop out through the
       | ok label.
       */
       for(t = (*s) + 1 ; ; t++) {
	   switch(*t) {

              /*
               | We have something of the form %<stuff>{d|x|o|u} so the corresponding argument which
               | should be a fixnum so get one and call the fprintf routine using the exact spec
               | passed and the long as an argument. We must null terminate the spec so we get the
               | next char first so that we can put the char back after the call to fprintf when
               | we are done.
               */
	       case 'd' : case 'x' : case 'o' : case 'u' : case 'X' :
		  if (!GetFix((*form)->carp, &ival)) goto er;
		  t += 1;
		  c = *t;
		  *t = '\0';
		  sprintf(*buff, *s, ival);
		  *t = c;
		  goto ok;

              /*
               | We have something of the form %<stuff>{f|e|g} so the corresponding argument which
               | should be a flonum so get one and call the fprintf routine using the exact spec
               | passed and the double as an argument. We must null terminate the spec so we get the
               | next char first so that we can put the char back after the call to fprintf when
               | we are done.
               */
	       case 'f' : case 'e' : case 'g' :
	       case 'E' : case 'G' :
		  if (!GetFloat((*form)->carp, &fval)) goto er;
		  t += 1;
		  c = *t;
		  *t = '\0';
		  sprintf(*buff, *s, fval);
		  *t = c;
		  goto ok;

              /*
               | We have something of the form %<stuff>{s} so the corresponding argument which
               | should be a string so get one and call the fprintf routine using the exact spec
               | passed and the string as an argument. We must null terminate the spec so we get the
               | next char first so that we can put the char back after the call to fprintf when
               | we are done.
               */
	       case 's' :
		  if (!GetString((*form)->carp, &sval)) goto er;
		  t += 1;
		  c = *t;
		  *t = '\0';
		  sprintf(*buff, *s, sval);
		  *t = c;
		  goto ok;

              /*
               | We have something of the form %<stuff>{c} so the corresponding argument which
               | should be a string so get one and call the fprintf routine using the exact spec
               | passed and the first char as an argument. We must null terminate the spec so we get the
               | next char first so that we can put the char back after the call to fprintf when
               | we are done.
               */
	       case 'c' :
		  if (!GetString((*form)->carp, &sval)) goto er;
		  t += 1;
		  c = *t;
		  *t = '\0';
		  sprintf(*buff, *s, sval[0]);
		  *t = c;
		  goto ok;

              /*
               | We have a <stuff> character so just skipt it. We do not check syntax beyond the
               | fact that only chars in this set may be in the <stuff>. Note deliberate abscence
               | of '*'!
               */
               case '#' : case '-' : case '+' : case ' ' : case '.' : case 'l' :
               case '0' : case '1' : case '2' : case '3' : case '4' : case '5' :
               case '6' : case '7' : case '8' : case '9' :
                  break;

              /*
               | Any other character in <stuff> is not allowed by printf so flag it now.
               */
	       default :
		  goto er;
	   }
       }

 er:   return(0);

 ok:   *s = t;                         /* advance to end of % spec */
       *form = (*form)->cdrp;          /* advance to next argument */
       len = strlen(*buff);            /* need length of expanded spec */
       *buff += len;                   /* advance to next slot in buffer */
       if ((*left)-- <= 0) goto er;    /* if this overflowed the buffer throw error */
       return(1);
}

/*
 | do_fprintf(buff, form) - will do a formatted sprintf of the arguments to the
 | given buffer. The first thing in the form is the format specifier as per normal
 | printf standards, the rest are the arguments to be formatted.
 */
static int do_sprintf(left, buff, form)
       int  *left;
       char *buff;
       struct conscell *form;
{
       char *s;
       char *b;

       if (form == NULL) goto er;
       if ((form->celltype != CONSCELL)||(!GetString(form->carp,&s))) goto er;

       form = form->cdrp;
       for(b = buff ; ; ) {
	   if (!emit_to_spec(left, &b, &s)) goto er;
	   if (*s == '\0') break;
	   if (!emit_next_spec(left, &b, &s, &form)) goto er;
	   if (*s == '\0') break;
       }
       if (form != NULL) goto er;        /* all args must have been used! */

       return(1);
 er:   return(0);
}

/*
 |  t <- (printf spec [arg] [arg] ....)
 |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
struct conscell *buprintf(form)
       struct conscell *form;
{
       char buff[MAXATOMSIZE];
       int  left = sizeof(buff) - 1;
       if (do_sprintf(&left, buff, form)) {
           printf("%s", buff);
	   return(LIST(thold));
       }
er:    ierror("printf");
}

/*
 |  t <- (fprintf port spec [arg] [arg] ....)
 |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 |  If writing to a socket that we were previously reading from must rewind the
 |  socket and reset the isread so that we know we are now writing to it.
 */
struct conscell *bufprintf(form)
       struct conscell *form;
{
       char buff[MAXATOMSIZE]; struct filecell *f;
       int  left = sizeof(buff) - 1;
       if ((form == NULL)||(form->carp == NULL)) goto er;
       f = PORT(form->carp);
       if ((f->celltype != FILECELL) || (f->atom == NULL)) goto er;
       if (do_sprintf(&left, buff, form->cdrp)) {
           if (f->issocket && f->state == 1) rewind(f->atom);     /* if was reading socket rewind before writing */
           f->state = 2;                                          /* set new state to writing */
           fprintf(f->atom, "%s", buff);
	   return(LIST(thold));
       }
er:    ierror("fprintf");
}

/*
 |  str <- (sprintf spec [arg] [arg] ....)
 |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
struct conscell *busprintf(form)
       struct conscell *form;
{
       char buff[MAXATOMSIZE];
       int  left = sizeof(buff) - 1;
       if (form == NULL) goto er;
       if (do_sprintf(&left, buff, form)) {
           if (strlen(buff) >= MAXATOMSIZE) goto er;     /* too big for a string */
	   return(LIST(insertstring(buff)));
       }
er:    ierror("sprintf");
}


