

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** CharIndex(s,c) Return the index of 'c' in string 's'. The first char**
 ** in 's' is at index 1 etc. We return 0 if the char is not found.     **
 *************************************************************************/
int CharIndex(s,c)
char *s,c;
{   register int n;
    for (n=1; *s != '\0'; s++,n++)
    {   if (*s == c)
           return(n);
    };
    return(0);
}

/*************************************************************************
 ** buchix: (character-index str ch)   Returns the position of character**
 ** ch   in string (or atom) str. Nil if no ch in str. Also, the second **
 ** parameter may be a fixnum ie (character-index str n) in which case  **
 ** n is interpreted as being the ascii representation of the character.**
 *************************************************************************/
struct conscell *buchix(form)
struct conscell *form;
{      char *str,*ch; int n; long lc;
       if ((form != NULL)&&(GetString(form->carp,&str)))
       {   form = form->cdrp;
           if (form != NULL)
           {   if (!GetFix(form->carp,&lc))
               {   if (!GetString(form->carp,&ch)) goto ERR;
                   lc = *ch;
               };
               if (form->cdrp == NULL)
               {   n = CharIndex(str,(char)lc);
                   if (n > 0)
                       return(newintop((long)n));
                   return(NULL);
               };
           };
       };
  ERR: ierror("character-index");
}
