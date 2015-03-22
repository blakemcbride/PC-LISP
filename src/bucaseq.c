/* EDITION AC01, APFUN PAS.751 (91/09/05 17:41:16) -- CLOSED */                 
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** eqforcase(key,exp) checks to see if case key 'key' is a match with  **
 ** the case-of expression 'exp'. They match if they are (eq) or if the **
 ** key is 't. However since Franz makes small fixnums (eq) if they have**
 ** the same value and PC-LISP does not we must check to see if the args**
 ** are fixnums and test their contents for (=).                        **
 *************************************************************************/
int eqforcaseq(key,exp)
struct conscell *key,*exp;
{     if ((exp == key)||(key == LIST(thold)))
          return(1);
      if ((exp != NULL)&&(key != NULL))
          if ((exp->celltype == FIXATOM)&&(key->celltype == FIXATOM))
              if (FIX(exp)->atom == FIX(key)->atom)
                  return(1);
      return(0);
}

/*************************************************************************
 ** (caseq exp -"(key -exps-)"-) is like a case statement. It evaluates **
 ** the expression 'exp' then looks through the keys until a match is   **
 ** found. When a match is found the exps that correspond are evaled one**
 ** by one and the result of the last eval is returned. The key may be  **
 ** an atom or fixnum or list of atoms or fixnums to indicate a set. The**
 ** special key 't matches any expression. Nil is returned if no match. **
 ** In the code, exp is the value that the key must match to cause a hit**
 ** 'mb' is the match body (key exp1 exp2...expn). mbk is the key from  **
 ** this body. It may be a list of keys. The second goto hit comes from **
 ** the multi key case. The first goto hit comes from the single key    **
 ** case. In either case we test for equality to thold the 't' atom ptr.**
 ** We use eqforcaseq(x,y) to test for a match. Fixnums must appear eq. **
 *************************************************************************/
struct conscell *bucaseq(form)
struct conscell *form;
{      register struct conscell *exp,*mb,*mbk,*t;
       if (form == NULL)
           gerror("caseq : no match exp given");
       exp = eval(form->carp);
       for(form=form->cdrp; form != NULL; form = form->cdrp)
       {   mb = form->carp;
           if ((mb == NULL)||(mb->celltype != CONSCELL))
               gerror("caseq : bad match body");
           mbk = mb->carp;
           if (eqforcaseq(mbk,exp))
              goto hit;
           if ((mbk != NULL)&&(mbk->celltype == CONSCELL))
              for( ; mbk != NULL; mbk = mbk->cdrp)
                 if (eqforcaseq(mbk->carp,exp))
                    goto hit;
       }
       return(NULL);
hit:   for(mb = mb->cdrp; mb != NULL; mb = mb->cdrp) {
           if ((exp = eval(mb->carp)) && (exp->celltype == CONSCELL))
              if (((t = exp->carp) == LIST(returnhold)) || (t == LIST(gohold)))
                 break;
       }
       return(exp);
}
