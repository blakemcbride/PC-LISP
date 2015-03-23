

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bucadar(): This is the c{a|d}* r evaluator. It takes two parameters **
 ** one is the caddadar function name. And the other is the form to be  **
 ** evaluated. First we go to end of string and match backwards doing   **
 ** a car or cdr on the form depending on which of 'd' or 'a' we find.  **
 ** Note if (sstatus chainatom) is set then we allow the error of taking**
 ** the car or cdr of an object that is not a cons or nil cell.         **
 *************************************************************************/
struct conscell *bucadar(form,s)
struct conscell *form; char *s;
{      if ((form == NULL)||(form->cdrp!=NULL))
           ierror("c{a|d}+r");
       form = form->carp;
       while(*s) s++;                              /*  s at end of name */
       s--; s--;                                   /*  backup past "r0" */
       while(*s != 'c')                            /*  scan back through*/
       {   if (form == NULL) return(NULL);         /*  name doing car or*/
           if (form->celltype != CONSCELL)         /*  error unless the */
           {   if (GetOption(CHAINATOM))           /* (sstatus chainatom)*/
                  return(NULL);                    /* is set, so NULL */
               ierror("c{a|d}+r");                 /* else an error. */
           };
           if (*s-- == 'd')                        /*  cdr if 'a' or 'd'*/
              form = form->cdrp;                   /*  as long as it is */
           else                                    /*  safe to do so.   */
              form = form->carp;
       };
       return(form);                               /*  reached 'c'..ret */
}

