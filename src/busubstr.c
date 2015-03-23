

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/***************************************************************************
 ** SubStringCopy(d,s,f,n) Copy a substring in s into buffer d. The string**
 ** copied starts at index 'f' and continues for 'n' bytes. If 'f' is < 0 **
 ** we invert 'f' so that it is 'f' bytes from the end of the string. If  **
 ** 'n' is negative the copy terminates when we hit the end of string '\0'**
 ** We return '1' if the substring is legal and 0 otherwise.              **
 ***************************************************************************/
static int SubStringCopy(d,s,f,n)
     char *d,*s; int f,n;
{    register int slen = strlen(s);
     if (f < 0) f = (slen+f)+1;                 /* -1 -> f = slen */
     f--; s+= f;                                /* shift origin, set source */
     if ((f < 0)||(f >= slen)) return(0);       /* check in range of source */
     if (n < 0) n = slen-f;                     /* n = #chars to copy */
     if ((f + n) > slen) n = slen - f + 1;      /* if would go over end of string adjust */
     while(n--) *d++ = *s++;                    /* copy n chars to dest */
     *d = '\0';                                 /* null terminate dest */
     return(1);                                 /* success */
}

/***************************************************************************
 ** busubstring: (substring str n1 [n2]) Extract a substring of length    **
 ** n2 from str starting at index n1 (from 1). If n2 is not present then  **
 ** length is to end of the string. We call the SubStringCopy function to **
 ** do the actual work for us, then we creat a new string cell and return.**
 ***************************************************************************/
struct conscell *busubstring(form)
struct conscell *form;
{      char *str;
       char work[MAXATOMSIZE];
       long int fr, ct;
       if ((form != NULL)&&(GetString(form->carp,&str))) {
            form = form->cdrp;
            if ((form != NULL)&&(GetFix(form->carp,&fr))) {
                form = form->cdrp;
                ct = -1L;
                if (form != NULL) {
                   if ((!GetFix(form->carp,&ct))||(ct<0L)||(form->cdrp != NULL))
                       goto ERR;
                }
                if (SubStringCopy(work,str,(int)fr,(int)ct))
                    return(LIST(insertstring(work)));
                return(NULL);
            }
       }
  ERR: ierror("substring");
}
