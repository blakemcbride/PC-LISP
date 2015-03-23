/* */
#define _NO_PROTO
#include <stdio.h>
#include <ctype.h>
#include "lisp.h"

/*
 | This function when passed a string containing a scanf spec like "<stuff...> .. %% ... %<stuff>d ..."
 | will return the first full spec in 'specout' and the type of the spec as the return value. For the
 | above string this function will return specout='<stuff...> .. %%' and return value of -1 to indicate
 | a simple string to match against input. If we call it again with '<stuff> ... %10d ..' this function
 | will return specout='<stuff> ... %10d' and a return valud of FIXNUM to indicate the %10d argument
 | is a FIXNUM. If an illegal spec is encountered this function returns -2.
 */
static int next_spec(specin, specout)
       char *specin, *specout;
{
       register char *s = specout;
       register char *t = specin;

      /*
       | Walk through specin until we get to a '%' or the end of string. If we get to the end
       | of string then just return -1 to indicate a simple matched string. Note that as we check
       | we copy.
       */
       for( ; *t != '%'; *s++ = *t++) {
           if (*t == '\0') {
               *s = '\0';
               return(-1);
           }
       }

      /*
       | We reached a '%' in the spec string so copy it to the specou and advance both pointers.
       */
       *s++ = *t++;

      /*
       | Now enter the validation loop to find the end of the %<stuff>{d...} string and establish
       | its type.
       */
       for( ; *s++ = *t ; *t++) {
	   switch(*t) {
               case '%' :
                  *s = '\0';
                  return(-1);
	       case 'd' : case 'x' : case 'o' : case 'u' :
                  *s = '\0';
                  return(FIXATOM);
               case 'c':
                  *s = '\0';
                  return(ALPHAATOM);
	       case 'f' : case 'e' : case 'g' :
                  *s = '\0';
                  return(REALATOM);
               case 's' :
                  *s = '\0';
                  return(STRINGATOM);
               case '[' :                           /* it is %[....] form so grab everything */
                  t += 1;
                  while(*t != ']') {
                      if (*t == '\0') return(-2);
                      *s++ = *t++;
                  }
                  *s++ = ']'; *s = '\0';
                  return(STRINGATOM);

              /*
               | We have a <stuff> character so just skipt it. We do not check syntax beyond the
               | fact that only chars in this set may be in the <stuff>. Note deliberate abscence
               | of '*'!
               */
               case '#' : case '-' : case '+' : case ' ' : case '.' :
               case '0' : case '1' : case '2' : case '3' : case '4' : case '5' :
               case '6' : case '7' : case '8' : case '9' :
                  break;

              /*
               | Any other character in <stuff> is not allowed by printf so flag it now.
               */
	       default :
		  return(-2);
	   }
       }
       return(-2);
}

/*
 | This function will iterate through each of the specs in 'spec' calling (*func)(arg1,"%....",&thing);
 | and accululate a list of the resulting things.
 */
static struct conscell *do_scanf(f, arg1, spec)
       int    (*f)();
       char   *arg1, *spec;
{
       char   subspec[MAXATOMSIZE];
       char   str[MAXATOMSIZE];
       char   *s;
       int    fix, rc;
       float  flo;
       struct conscell *r, *n, *t, *nreverse();
       push(r); push(n);
       for(s = spec; *s; s += strlen(subspec) ) {              /* iterate through each subspec in the string of specs */
           switch( next_spec(s, subspec) ) {                   /* get 'subspec' and its type */
               case -1:                                        /* if subspec is just a string to be match, match string */
                    (*f)(arg1, subspec);
                    continue;                                  /* nothing to append, so back to top of loop */
               case FIXATOM:                                   /* if 'subspec' represents an integer then scan */
                    rc = (*f)(arg1, subspec, &fix);            /* get the fix from string/port and create a new FIXATOM */
                    if (rc != 1) goto x;                       /* cell for it */
                    n = newintop((long)fix);
                    break;
               case REALATOM:                                  /* DITTO real atom */
                    rc = (*f)(arg1, subspec, &flo);
                    if (rc != 1) goto x;
                    n = newrealop((double)flo);
                    break;
               case ALPHAATOM:                                 /* DITTO for single char */
                    rc = (*f)(arg1, subspec, &str[0]);
                    if (rc != 1) goto x;
                    str[1] = '\0';
                    n = LIST(insertstring(str));
                    break;
               case STRINGATOM:                                /* DITTO string atom */
                    rc = (*f)(arg1, subspec, str);
                    if (rc != 1) goto x;
                    n = LIST(insertstring(str));
                    break;
               default:
                    goto x;
           }
           t = new(CONSCELL);                                  /* append the just scanned item to the output list */
           t->carp = n;                                        /* note the list is built up backwards and then */
           t->cdrp = r;                                        /* nreversed before being returned */
           r = t;
       }
   x:  xpop(2);
       return(nreverse(r));
}

/*
 |  (val val ..) <- (scanf spec)
 |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
struct conscell *buscanf(form)
       struct conscell *form;
{
       char *s, spec[MAXATOMSIZE];
       if ((form == NULL) || (!GetString(form->carp, &s))) goto er;
       strcpy(spec, s);
       if (form->cdrp) goto er;
       zapee = stdin;
       return(do_scanf(fscanf,stdin,spec));
er:    ierror("scanf");
}

/*
 |  (val val ...) <- (fscanf port spec)
 |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
struct conscell *bufscanf(form)
       struct conscell *form;
{
       char *s, spec[MAXATOMSIZE]; struct filecell *p;
       if ((form == NULL) || ((p = PORT(form->carp)) == NULL)) goto er;
       if ((p->celltype != FILECELL) || (p->atom == NULL)) goto er;
       if (!(form = form->cdrp) || (!GetString(form->carp, &s))) goto er;
       strcpy(spec, s);
       if (form->cdrp) goto er;
       if (p->issocket && p->state == 2) rewind(p->atom);         /* if was writing socket rewind before reading */
       p->state = 1;                                              /* set new state to reading */
       zapee = p->atom;
       return(do_scanf(fscanf,p->atom,spec));
er:    ierror("fscanf");
}

