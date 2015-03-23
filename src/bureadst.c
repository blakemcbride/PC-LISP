#include        <stdio.h>
#include        <ctype.h>
#include        "lisp.h"

/*
 | readstr(list, pad, result)
 |
 | This primitive will read an expression from the string formed by concaten-
 | ating all the string or atom parameters. It does this by concatenating all
 | the elements of the form list with an optional single space between them.
 |
 | This function makes use of the normal ReadExpression routine to do its
 | work but inorder to get the string into the input stream it uses two
 | special scaner functions called 'ScanFromBuffer' and 'ScanReset'. These
 | just enable and disable forced scanning from a buffer rather than from
 | the requested input stream.
 */
static int readstr(form, pad, result)
struct conscell *form, **result; int pad;
{      char work[2048], *s; struct conscell *p; int worklen, slen;
       work[0] = '\0'; worklen = 0;

      /*
       | Loop through the parameters building up a large buffer of the
       | contents of each string|atom parameter with ' ' between them.
       | Check for overflow of the buffer and throw error if it happens.
       */
       while(form != NULL) {
             if (form->celltype != CONSCELL) goto er;
             p = form->carp;
             if (!GetString(p, &s)) goto er;
             slen = strlen(s);
             if (worklen + slen + 1 >= sizeof(work)) goto er;
             memcpy(&work[worklen], s, slen);
             worklen += slen;
             if (pad) work[worklen++] = ' ';
             form = form->cdrp;
       }

      /*
       | Back up the end of the buffer past any trailing white space.
       | If we manage to back all the way to the beginning of the
       | buffer then just return NULL.
       */
       while(worklen > 0 && isspace(work[worklen-1])) worklen--;
       if (worklen == 0)  {*result = NULL; return(1); }

      /*
       | Tell scanner that we want it to operate out of the following
       | buffer for the time being.
       */
       ScanFromBuffer(work, worklen);

      /*
       | Now read the expression. The port is not important because the
       | scanner will read from the buffer until a ScanReset operation
       | occurs. After reading the expression we reset the scanner which
       | will also return the number of characters not scanned in the
       | buffer. If this is non zero then there were extraneous chars in
       | the buffer that are not part of the S-expression so we return 0
       | to cause an error to be thrown.
       */
       *result = ReadExpression(piporthold,NULL,1);
       if (ScanReset() != 0) goto er;
       return(1);
er:    return(0);
}

/*
 | This primitive will read an expression from the string formed by concaten-
 | ating all the string or atom parameters. It does this by concatenating all
 | the input parmaters with a single space between them and then calling the
 | ReadExpression routine.
 |
 |    -->(readstr "'(a b" "c" ")")
 |    '(a b c)
 */
struct conscell *bureadst(form)
struct conscell *form;
{      struct conscell *result;
       if (form == NULL) return(NULL);
       if (readstr(form, 1, &result))
           return(result);
       ierror("readstr");
}

/*
 | This primitive will read an expression from the string formed by concaten-
 | ating all the string or atoms in its single list parameter. It does this
 | by concatenating all the input parmaters without a single space
 | between them and then calling ReadExpression.
 |
 |    -->(readlist "'(a b" "c" ")")
 |    '(a b c)
 */
struct conscell *bureadli(form)
struct conscell *form;
{      struct conscell *result;
       if (form == NULL) return(NULL);
       if (form->cdrp != NULL) goto er;
       if ((form = form->carp) == NULL) return(NULL);
       if (form->celltype != CONSCELL) goto er;
       if (readstr(form, 0, &result))
           return(result);
  er:  ierror("readlist");
}

