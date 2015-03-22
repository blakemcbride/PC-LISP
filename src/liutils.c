/* EDITION AE04, APFUN PAS.796 (92/03/16 11:32:34) -- CLOSED */                 

/****************************************************************************
 **           PC-LISP (C) Peter Ashwood-Smith, 1986                        **
 **           MODULE UTILS                                                 **
 ** ---------------------------------------------------------------------- **
 ** This module contains things that I did not know where else to put. They**
 ** are either special purpose functions called in only one place, or they **
 ** are general purpose functions used all over the place. In general the  **
 ** source organization of PC-LISP is poor this is an attempt to rectify it**
 ****************************************************************************/

#include <signal.h>
#include <stdio.h>
#include <math.h>
#include "lisp.h"

#define ISSPEC(t)  (t==LIST(optionalhold))||(t==LIST(resthold))||(t==LIST(auxhold))

/****************************************************************************
 ** ConstructLetArgList(alist): Given a lambda expressions parameter list  **
 ** alist it will construct a list without &optional,&aux,&rest meta parms **
 ** and default settings. This is pretty simple traverse, extract & build. **
 ****************************************************************************/
struct conscell *ConstructLetArgList(alist)
struct conscell *alist;
{      struct conscell *nalist,*temp1,*temp2,*back;
       push(nalist);                               /* NOTE nalist = NULL now!*/
       for(;alist!=NULL;alist=alist->cdrp)
       {   if ((temp1 = alist->carp) == NULL) ierror("defun|def");
           if (ISSPEC(temp1)) continue;
           if (temp1->celltype == CONSCELL) temp1 = temp1->carp;
           temp2 = new(CONSCELL);
           temp2->carp = temp1;
           if (nalist == NULL)
               nalist = temp2;
           else
               back->cdrp = temp2;
           back = temp2;
       };
       xpop(1);
       return(nalist);
}

/****************************************************************************
 ** ConstructNonAuxFormals: Given a lambda expressions parameter list      **
 ** alist it will return a copy of the list up to but not including the    **
 ** first &aux. This is used to define the parms of the outer lambda.      **
 ****************************************************************************/
struct conscell *ConstructNonAuxFormals(alist)
struct conscell *alist;
{      struct conscell **l,*t,*r;
       for(l = &(alist); *l != NULL; l = &((*l)->cdrp))   /* set l to point to cdr field of cell before &aux */
           if ((*l)->carp == LIST(auxhold)) break;
       if (!(*l)) return(alist);                          /* if no aux in list then entire list is formals */
       if (l == &(alist)) return(NULL);                   /* if &aux at front of list then no non aux formals */
       xpush(t);                                          /* otherwise have &aux in middle somewhere so */
       t = *l;                                            /* temporarily trim the (&aux ...) tail from list */
       *l = NULL;                                         /* copy the head of the alist which will be returned */
       r = topcopy(alist);                                /* and then put the list back together again. Note */
       *l = t;                                            /* that since the list is split temporarily the tail */
       xpop(1);                                           /* must be reachable for garbage collection hence the */
       return(r);                                         /* push pop */
}

/****************************************************************************
 ** ConstructPair(a,b) Will make a list (a b) pretty simple stuff.         **
 ****************************************************************************/
struct conscell *ConstructPair(a,b)
struct conscell *a,*b;
{      struct conscell *r;
       xpush(a); xpush(b); push(r);
       r = new(CONSCELL);
       r->carp = a;
       r->cdrp = new(CONSCELL);
       r->cdrp->carp = b;
       xpop(3);
       return(r);
}

/****************************************************************************
 ** BuildActualExpr(dcl,kind,varnum) : Will build one of the following 4   **
 ** expression forms. 0 --> (arg varnum) 1 --> (arg? varnum (cadr dcl))    **
 ** 2 --> (listify varnum) 3 --> nil or (cadr dcl) depending on dcl. The   **
 ** dcl expression is the parameter which is of kind 'kind'. It is either  **
 ** the name of the parameter or a list whose second element is the default**
 ** value of the list. kind is 0,1,2 or 3 meaning normal, optional, rest   **
 ** and &aux accordingly. Varnum is the number of this argument in the arg **
 ** list of the lexpr. It is counted by the driver of this routine.        **
 ****************************************************************************/
struct conscell *BuildActualExpr(dcl,kind,varnum)
struct conscell *dcl; int kind,varnum;
{      struct conscell *t,*r;
       if (dcl == NULL) goto ERR;
       switch (kind)
       {  case 0 :  return(ConstructPair(CreateInternedAtom("arg"),
                           newintop((long)varnum)));
          case 1 :  push(t);
                    t = ConstructPair(CreateInternedAtom("arg?"),
                           newintop((long)varnum));
                    if (dcl->celltype == CONSCELL)
                        t->cdrp->cdrp = dcl->cdrp;
                    else
                    {   r = (t->cdrp->cdrp = new(CONSCELL));
                        r->carp = NULL;
                    };
                    xpop(1);
                    return(t);
          case 2 :  return(ConstructPair(CreateInternedAtom("listify"),
                           newintop((long)varnum)));
          case 3  : if (dcl->celltype == CONSCELL)
                    {   if (dcl->cdrp == NULL) goto ERR;
                        return(dcl->cdrp->carp);
                    }
                    else
                        return(NULL);
          default : goto ERR;
       };
 ERR:  ierror("defun|def");
}

/****************************************************************************
 ** ConstructLetActuals(alist) : alist is the parameter list of the lambda **
 ** expression passed to lexprify(). We take this list and scan through it **
 ** setting kind from 0 to 1 to 2 ...3 as we find each of &optional etc.   **
 ** We dispatch the function BuildActualExpr for the parameter of the kind **
 ** currently being scanned. We string these together and return them. Note**
 ** we check that the sequence &optional, &rest, &aux is met and ierror if **
 ** it is not. As usual must be careful for garbage collection problems.   **
 ****************************************************************************/
struct conscell *ConstructLetActuals(alist)
struct conscell *alist;
{      struct conscell *nalist,*temp1,*temp2,*back;
       int lastkind = -1, kind = 0, varnum = 0;
       push(nalist); push(temp1);                  /* NOTE nalist = NULL now!*/
       for(;alist!=NULL;alist=alist->cdrp)
       {   temp1 = alist->carp;
           if (temp1 == LIST(optionalhold))
           {   lastkind = kind; kind = 1; continue;};
           if (temp1 == LIST(resthold))
           {   lastkind = kind; kind = 2; continue;};
           if (temp1 == LIST(auxhold))
           {   lastkind = kind; kind = 3; continue;};
           if (kind <= lastkind) ierror("defun|def");
           temp1 = BuildActualExpr(temp1,kind,++varnum);
           temp2 = new(CONSCELL);
           temp2->carp = temp1;                    /* watch order for GC */
           if (nalist == NULL)
               nalist = temp2;
           else
               back->cdrp = temp2;
           back = temp2;
       };
       xpop(2);
       return(nalist);
}

/****************************************************************************
 ** lexprify(form) : Take a lambda expression which may have special parms **
 ** like &optional, &rest, &aux and convert it to the appropriate lexpr. We**
 ** use the above routines to get this done. Basically a form like this:   **
 **                                                                        **
 **    (lambda(x &optional (y 98)(z 99) &rest R &aux A (B 2) <bodies>)     **
 **                                                                        **
 ** Is Converted to a form like:                                           **
 **                                                                        **
 **    (lexpr (_N_)                                                        **
 **           ((lambda(x y z R A B) <bodies>)                              **
 **            (arg 1)                               ; parameter a         **
 **            (arg? 2 98)                           ; parm 2 or default 98**
 **            (arg? 3 99)                           ; parm 3 or default 99**
 **            (listify 4)                           ; rest of parms 4...  **
 **            nil                                   ; local A bound nil.  **
 **            2))                                   ; local B start val 2 **
 **                                                                        **
 ** Note that the function (arg? n exp) was added to PC-LISP to simplify   **
 ** the work needed here. It is also more effecient than a cond of _N_ and **
 ** is not subject to context problems. (This is not Franz way). Sorry it  **
 ** is such icky code but I was trying to use common subexpressions as much**
 ** as possible to keep loading from slowing down too much. As a side test **
 ** when scanning the formal parameters, we check that none are t or nil.  **
 ** If an arg is t or nil then the binding of t or nil could be changed!   **
 **                                                                        **
 ** APRIL 3rd 1991 - Add optimization here to handle case of arg lists with**
 ** only &aux arguments. Basically if an argument list has no &opt or &rest**
 ** arg then we can map to a lambda with a lambda within it. This means we **
 ** do not have to evaluate all the (arg...) actual parameters and is hence**
 ** faster. It also compiles to much better code eg:                       **
 **                                                                        **
 **    (lambda(x y &aux z (w 4)) <bodies>)                                 **
 **                                                                        **
 ** Is Converted to a form like:                                           **
 **                                                                        **
 **    (lambda (x y)                                                       **
 **            ((lambda(z w) <bodies>)                                     **
 **             nil                                                        **
 **             4                                                          **
 **            ))                                                          **
 ****************************************************************************/
struct conscell *lexprify(form)
struct conscell *form;
{      struct conscell *auxpos,*temp,*temp2,*hold,*l,*Narg;
       int aux = 0, rest = 0, opt = 0;
       if (form == NULL) goto ERR;
       temp = form->cdrp;
       if (temp == NULL) goto ERR;
       for(temp2 = temp = temp->carp; temp != NULL; temp = temp->cdrp) {
           if (!temp->carp || (temp->carp == LIST(thold)))
               gerror("t|nil parameter");
           if (temp->carp == LIST(auxhold))      { aux  += 1; auxpos = temp; continue; }
           if (temp->carp == LIST(resthold))     { rest += 1; continue; }
           if (temp->carp == LIST(optionalhold)) { opt  += 1; continue; }
       }
       if ( ! (aux || rest || opt) ) return(form);
       if ((aux > 1 || rest > 1 || opt > 1)) gerror("bad &aux, &rest or &opt");
       if ( rest || opt ) auxpos = temp2;
       push(l); push(hold);
       l = ConstructLetArgList(auxpos);
       hold = temp = new(CONSCELL);
       temp->carp =  (rest || opt) ? LIST(lexprhold) : LIST(lambdahold);
       temp = (temp->cdrp = new(CONSCELL));
       if ( rest || opt )
          ( temp->carp = new(CONSCELL))->carp = LIST(CreateInternedAtom("_N_"));
       else {
          temp->carp = ConstructNonAuxFormals(temp2);
       }
       temp = (temp->cdrp = new(CONSCELL));
       temp2 = (temp->carp = new(CONSCELL));
       temp = (temp2->carp = new(CONSCELL));
       temp->carp = LIST(lambdahold);
       temp = (temp->cdrp = new(CONSCELL));
       temp->carp = l;
       temp->cdrp = form->cdrp->cdrp;
       temp2->cdrp = ConstructLetActuals(auxpos);
       xpop(2);
       return(hold);
ERR:   ierror("defun|def");
}

/*******************************************************************
 ** function hash(s,n) : Compute Hash function on the bytes in 's'**
 ** for n-1 bytes. Similar to " Compiles, Principles, Techniques  **
 ** and Tools by Aho Setti and Ullman, page 436". It seems to be  **
 ** a reasonable choice. It is a little costly to compute on a PC **
 ** because of the long unsigned math but the distribution  is    **
 ** good which is more important for G/C compaction performance.  **
 *******************************************************************/
int hash(s,n)
char *s; int n;
{   unsigned long h = 0L, g;
    while(n-- > 0) {
        h = (h << 4) + (*s++);
        if (g = h & 0xf0000000) {
            h ^= (g >> 24);
            h ^= g;
        }
    }
    return(h % ALPHATABSIZE);
}

/*************************************************************************
 ** iscadar(s) Will return true '1' or '0' false if the string 's' is   **
 ** a valid c{a|d}*r function name. Ie car cdr cadar caaaadr etc.       **
 *************************************************************************/
int iscadar(s)
char *s;
{    if (*s++ != 'c') return(0);                  /* first a 'c' ? */
     if ((*s != 'd')&&(*s != 'a')) return(0);     /* second 'd' or 'a' ? */
     do s++; while((*s == 'd')||(*s == 'a'));     /* infinite 'd' or 'a' s */
     if (*s++ != 'r') return(0);                  /* terminated with 'r' ?*/
     return(*s == '\0');                          /* null terminated ? */
}

/****************************************************************************
 ** GetFloat(l,where) Extract a double from cell 'l'. If it is a fix or real**
 ** cell we store the real in *where. If not a numberic type return(0).    **
 ****************************************************************************/
int GetFloat(l,where)
struct conscell *l; double *where;
{   if (l != NULL)
    {   if (l->celltype == FIXATOM)
        {   *where = (double) FIX(l)->atom;
            return(1);
        };
        if (l->celltype == REALATOM)
        {   *where = REAL(l)->atom;
            return(1);
        };
    };
    return(0);
}

/****************************************************************************
 ** GetFix(l,where) Extract a fixnum from cell 'l'. If it is a fix or real **
 ** cell we store the fix in *where. If not a numberic type return(0).     **
 ****************************************************************************/
int GetFix(l,where)
struct conscell *l; long int *where;
{   if (l != NULL)
    {   if (l->celltype == FIXATOM)
        {   *where = FIX(l)->atom;
            return(1);
        };
        if (l->celltype == REALATOM)
        {   *where = (long) REAL(l)->atom;
            return(1);
        };
    };
    return(0);
}

/****************************************************************************
 ** GetChar(l,where) Extract a character from cell 'l'. If l is a string   **
 ** or alpha atom the char is the first in the print name. If l is a fixnum**
 ** in the range 0..255 the character is that with this ascii value.       **
 ****************************************************************************/
int GetChar(l,where)
struct conscell *l; char *where;
{   if (l != NULL)
    {   switch(l->celltype)
        {   case FIXATOM:
                 if ((FIX(l)->atom < 0)||(FIX(l)->atom > 255)) return(0);
                 *where = (char) FIX(l)->atom;
                 return(1);
            case ALPHAATOM:
                 *where = *(ALPHA(l)->atom);
                 return(1);
            case STRINGATOM:
                 *where = *(STRING(l)->atom);
                 return(1);
            default:
                 return(0);
        };
    };
    *where = 'n';               /* n as in 'n' 'i' 'l' */
    return(1);
}

/****************************************************************************
 ** GetString(l,where) Extract a pointer to a character string from cell   **
 ** 'l' where 'l' may be an ALPHAATOM  or a STRINGATOM. Store the pointer  **
 ** at *where and return 1 if successful otherwise return 0 for failure.   **
 ** Note that if l is NULL pointer then the string assocaited is "nil".    **
 ****************************************************************************/
int GetString(l,where)
struct conscell *l; char **where;
{   if (l != NULL)
    {   if (l->celltype == ALPHAATOM)
        {   *where = ALPHA(l)->atom;       /* alters callers pointer */
            return(1);
        };
        if (l->celltype == STRINGATOM)
        {   *where = STRING(l)->atom;      /* alters callers pointer */
            return(1);
        };
        return(0);
    };
    *where = "nil";                        /* NULL is nil is "nil" */
    return(1);
}

/*************************************************************************
 ** enlist(x) : Will listify x, this just means making a cons cell whose**
 ** car points to x and whose cdr is nil. This is used by nlambda when  **
 ** it makes an arg list as far as the interpreter is concerned into a  **
 ** list of args as far as the user function is concerned.              **
 *************************************************************************/
struct conscell *enlist(x)
struct conscell *x;
{      register struct conscell *t;
       t = new(CONSCELL);
       t->carp = x;
       return(t);
}

/*************************************************************************
 ** newintop: just returns a new cell with long int value it in .       **
 *************************************************************************/
struct conscell *newintop(val)
long int val;
{      struct fixcell *t;
       t = (struct fixcell *)new(FIXATOM);
       t->atom = val;
       return((struct conscell *)t);
}

/*************************************************************************
 ** newfixfixop: just returns a new fixfixcell with long values a & b   **
 *************************************************************************/
struct conscell *newfixfixop(a,b)
long int a, b;
{      struct fixfixcell *t;
       t = (struct fixfixcell *)new(FIXFIXATOM);
       t->atom1 = a;
       t->atom2 = b;
       return((struct conscell *)t);
}

/*************************************************************************
 ** newrealop: just returns a new real atom cell with value 'val'.      **
 *************************************************************************/
struct conscell *newrealop(val)
double val;
{      struct realcell *t;
       t = (struct realcell *)new(REALATOM);
       t->atom = val;
       return((struct conscell *)t);
}

/*************************************************************************
 ** reverse(l) : will return a copy of the reverse of the list l. We    **
 ** do this simply by traversing the list and creating new cons cells   **
 ** for the reversed list. We return a pointer to the last cons cell.   **
 *************************************************************************/
struct conscell *reverse(l)
struct conscell *l;
{      struct conscell *lasts, *s;
       xpush(l); push(lasts);
       lasts = NULL;
       while( l != NULL ) {
              if (l->celltype != CONSCELL) ierror("reverse");
              s = new(CONSCELL);
              s->carp = l->carp;
              l = l->cdrp;
              s->cdrp = lasts;
              lasts = s;
       }
       fret(lasts,2);
}

/*************************************************************************
 ** nreverse(l) : will destructively reverse the list 'l' and return the**
 ** new head.                                                           **
 *************************************************************************/
struct conscell *nreverse(l)
struct conscell *l;
{      struct conscell *m,*n;
       if (l == NULL) return(NULL);
       m = l->cdrp;
       l->cdrp = NULL;
       for(;;) {
          if (m == NULL) goto ok;
          if (m->celltype != CONSCELL) ierror("nreverse");
          n = m->cdrp;
          m->cdrp = l;
          l = m;
          m = n;
       }
ok:    return(l);
}

/*************************************************************************
 ** copy(l) : Will return a copy of the list l using new cons cells for **
 ** all levels. We must be careful to properly handle dotted pairs!     **
 *************************************************************************/
struct conscell *copy(l)
struct conscell *l;
{      struct conscell *first,*last,*s;
       if ((l == NULL)||(l->celltype != CONSCELL)) return(l);
       xpush(l); push(first); push(s);
       first = new(CONSCELL);
       first->carp = copy(l->carp);
       last = first;
       l = l->cdrp;
       while( l != NULL ) {
              if (l->celltype != CONSCELL) { last->cdrp = l; break; }
              s = new(CONSCELL);
              s->carp = copy(l->carp);
              l = l->cdrp;
              last->cdrp = s;
              last = s;
       }
       fret(first,3);
}

/*************************************************************************
 ** topcopy(l) : Will return a copy of the list l using new cons cells  **
 ** for the top level only. Ie we traverse cdrp's and make carps equal. **
 *************************************************************************/
struct conscell *topcopy(l)
struct conscell *l;
{      struct conscell *first,*last,*s;
       if (l == NULL) return(NULL);
       xpush(l); push(first);
       first = new(CONSCELL);
       first->carp = l->carp;
       last = first;
       l = l->cdrp;
       while( l != NULL)
       {      s = new(CONSCELL);
              s->carp = l->carp;
              l = l->cdrp;
              last->cdrp = s;
              last = s;
       };
       fret(first,2);
}

/*************************************************************************
 ** hunkequal(h1,h2) Returns 1 (TRUE) if hunk1 and hunk2 are equal. Else**
 ** it returns 0 (FALSE). Note that two hunks are 'equal' if they have  **
 ** the same size and each of their elements are 'equal'. We traverse   **
 ** the hunks and compare using the funcion 'equal' to compare each elem**
 ** ent. Note that 'equal' in bufunc.c will most likely be our caller.  **
 *************************************************************************/
int hunkequal(h1,h2)
struct hunkcell *h1,*h2;
{      register int n;
       if (h1 == h2) return(1);
       if ((h1 == NULL)||(h2 == NULL)) return(0);
       if ((n = h1->size) != h2->size) return(0);
       while(n--)
       {  if (!equal(*GetHunkIndex(h1,n),*GetHunkIndex(h2,n)))
               return(0);
       };
       return(1);
}

/*************************************************************************
 ** clispequal(c1,c2) Returns 1 (TRUE) if compiled lisp object c1 is the**
 ** same structurally as compiled lisp object c2. We must compare the   **
 ** literals and code individually. The literals must be individually   **
 ** 'equal' to each other and the code arrays must be the same length & **
 ** with identical contents. This is mostly used after compilation and  **
 ** writing to a file to read the file back and make sure that the      **
 ** binary I/O routines libio.c have properly read/written the clisp    **
 ** this is why we check the self pointers for validity!                **
 *************************************************************************/
int clispequal(c1, c2)
struct clispcell *c1, *c2;
{      struct conscell **l1, **l2; int n1, n2;
       l1 = c1->literal; l2 = c2->literal;
       n1 = *(((int *) l1 ) - 1);
       n2 = *(((int *) l2 ) - 1);
       if (n1 != n2) return(0);                       /* same literal count ? */
       if ((*l1++ != LIST(c1)) || (*l2++ != LIST(c2)))
          return(0);                                  /* lit[0] = self for both objs? (should always be true) */
       n1 = (n1 / sizeof(struct conscell)) - 1;       /* convert to real literal count skipping literal[0] */
       while(--n1 >= 0)                               /* recursively compare all literals */
          if (!equal(*l1++, *l2++))
              return(0);
       n1 = *(((int *) c1->code ) - 1);               /* get machine code counts */
       n2 = *(((int *) c2->code ) - 1);
       if (n1 != n2) return(0);                       /* must have same amount of code each */
       return(memcmp(c1->code, c2->code, n1) == 0);   /* and code must be identical */
}

/*************************************************************************
 ** eq(e1,e2): will return 1 iff expression e1 is 'eq' to expression e2 **
 ** according to the Franz definition of 'eq'. Two expressions are 'eq' **
 ** iff they are the same object. Note that we treat fixnums with the   **
 ** same value as being the same object as per Franz.                   **
 *************************************************************************/
int eq(e1,e2)
struct conscell *e1, *e2;
{      if (e1 == e2) return(1);
       if ((e1 == NULL) || (e2 == NULL)) return(0);
       if (e1->celltype != e2->celltype) return(0);
       if (e1->celltype != FIXATOM) return(0);
       return(FIX(e1)->atom == FIX(e2)->atom);
}

/*************************************************************************
 ** int equal(x,y) Will return true '1' if the lists x and y 'look' the **
 ** same, ie have the same structure. So realatoms are equal if they are**
 ** the same number, alphas are equal if there print names match, file  **
 ** atoms are the same if there file pointers are the same, finally the **
 ** cons cells are equal if their car and cdr pointers are equal.       **
 *************************************************************************/
int equal(x,y)
struct  conscell *x,*y;
{       if (x == y) return(1);
        if ((x == NULL)||(y == NULL)) return(0);
        if (x->celltype != y->celltype) return(0);
        switch(x->celltype)
        {       case CONSCELL:
                        return(equal(x->carp,y->carp)&&
                               equal(x->cdrp,y->cdrp));
                case ALPHAATOM:
                        return(strcmp(ALPHA(x)->atom,ALPHA(y)->atom) == 0);
                case FIXATOM:
                        return(FIX(x)->atom == FIX(y)->atom);
                case STRINGATOM:
                        return(strcmp(STRING(x)->atom, STRING(y)->atom)==0);
                case REALATOM:
                        return(REAL(x)->atom == REAL(y)->atom);
                case FILECELL:
                        return(PORT(x)->atom == PORT(y)->atom);
                case HUNKATOM:
                        return(hunkequal(x,y));
                case ARRAYATOM:
                        return(equal(ARRAY(x)->info,ARRAY(y)->info) &&
                               hunkequal(ARRAY(x)->base,ARRAY(y)->base));
                case CLISPCELL:
                        return(clispequal(x,y));
                case FIXFIXATOM:
                        return((FIXFIX(x)->atom1 == FIXFIX(y)->atom1) &&
                               (FIXFIX(x)->atom2 == FIXFIX(y)->atom2));
        };
        return(0);
}

/*************************************************************************
 ** macroexpand(l): Will expand all the macros in list 'l'. We construct**
 ** the list 'l' after destructively replacing the arm1...parmn). First **
 ** we must extract the function name called 'fnat' then we get the body**
 ** from the atoms func field and make sure it is FN_USMACRO form, then **
 ** we apply the function to the entire form (macroname parm1...parmn). **
 ** Recursively: Base cases is null,non list returns itself. Induction  **
 ** A list which starts with a macro name returns the application of the**
 ** macro. A list which starts with the function 'quote' returns itself **
 ** ie macroexpand does not work inside quote. Any other type of list is**
 ** made by calling macroexpand twice recursively for the car and cdr.  **
 *************************************************************************/
 struct conscell *macroexpand(l)
 struct conscell *l;
 {      struct alphacell *fnat; struct conscell *fn;
        if ((l == NULL)||(l->celltype != CONSCELL)) return(l);
        fnat = ALPHA(l->carp);
        if ((fnat != NULL)&&(fnat->celltype == ALPHAATOM))
        {   if (fnat->fntype == FN_USMACRO)                 /** (macro S) **/
            {   fn = LIST(fnat->func);
                return(apply(fn,l));
            };
            if (fnat == quotehold)                          /** (quote S) **/
                return(l);
        };
        push(fn);
        fn = new(CONSCELL);
        fn->carp = macroexpand(l->carp);
        fn->cdrp = macroexpand(l->cdrp);
        xpop(1);
        return(fn);
}

/*************************************************************************
 ** putprop : this function will add a new element to the property list **
 ** for the atom 'atm', with indicator atom 'indic'. This is done by    **
 ** first checking for a pair with indicator 'indic' if this pair exists**
 ** we put put the prop directly into the list, otherwise we tack a new **
 ** pair onto the front of the property list.                           **
 *************************************************************************/
struct conscell *putprop(atm,indic,prop)
struct alphacell *atm; struct conscell *indic,*prop;
{      struct conscell *t1,*t2;
       for(t1 = atm->proplist; t1 != NULL ; t1 = t1->cdrp)
       {   if (t1->carp != NULL)
           {  if (equal(t1->carp->carp, indic))
              {   t1->carp->cdrp = prop;
                  return(prop);
              }
           }
       }
       push(t1);                      /* don't collect t1 please! */
       t1 = new(CONSCELL);
       t2 = new(CONSCELL);
       xpop(1);
       t1->carp = t2;
       t1->cdrp = atm->proplist;
       t2->carp = indic;
       t2->cdrp = prop;
       atm->proplist = t1;
       return(prop);
}

/*************************************************************************
 ** getprop : this function will search the property list associated    **
 ** with atom 'atm' looking for indicator 'indic'. When indic is found  **
 ** we return a pointer to this property list. If not found we return   **
 ** just placing the indic,prop pair onto the front of the propertly    **
 ** NULL.                                                               **
 *************************************************************************/
struct conscell *getprop(atm,indic)
struct conscell *atm,*indic;
{      register struct conscell *l, *t;
       l = ALPHA(atm)->proplist;
       while(l != NULL) {
             t = l->carp;
             if ( t && equal(t->carp, indic) ) return(t->cdrp);
             l = l->cdrp;
       }
       return(NULL);
}

/*************************************************************************
 ** assoc : Will search the list 'alist' for a key value that is equal  **
 ** to 'var'. If none is found we return NULL. Call 'equal' to do a     **
 ** comparisson of the list structures for equality.                    **
 *************************************************************************/
struct conscell *assoc(var,alist)
struct conscell *var,*alist;
{      extern int equal();
       register struct conscell *t;
       for( ; alist && (alist->celltype == CONSCELL) ; alist = alist->cdrp ) {
             t = alist->carp;
             if (t &&(t->celltype == CONSCELL))
                if (equal(var, t->carp))
                    return(t);
       }
       return(NULL);
}

/*************************************************************************
 ** pairlist : the inverse of assoc, will match a list of vars with a   **
 ** list of values to then append the alist and return a new alist. If  **
 ** the vals list runs out we tack associate NULL with the corresponding**
 ** remaining atoms in the vars list.                                   **
 *************************************************************************/
struct conscell *pairlis(vars,vals,alist)
struct conscell *vars,*vals,*alist;
{      struct conscell *r,*t,*l,*last;
       xpush(vars); xpush(vals); xpush(alist);
       push(r);
       r = NULL;
       while(vars != NULL)
       {      t = new(CONSCELL);
              if (r==NULL)
                 {r=t; last = t; }
              else
                 {last->cdrp = t; last = t; };
              l = new(CONSCELL);
              l->carp = vars->carp;
              if (vals != NULL)
              {   l->cdrp = vals->carp;
                  vals = vals->cdrp;
              };
              t->carp = l;
              vars = vars->cdrp;
       };
       if (r != NULL)
           t->cdrp = alist;
       fret(r,4);
}

/*************************************************************************
 ** pushlexpr(at,args) Will handle entry to an lexpr body. This is done **
 ** by binding the args list to the global lexpr holding atom this atom **
 ** will be used to access the arguments via the (arg) function. Next   **
 ** the single parameter to the lexpr is bound to the number or args.   **
 ** In order to speed things up the arg list has the length tacked onto **
 ** the front. This allows (arg) (listify) and (setarg) to run faster.  **
 *************************************************************************/
pushlexpr(at,args)
struct conscell *at, *args;
{      register int i;
       struct conscell *temp;
       push(temp);
       temp = new(CONSCELL);
       temp->cdrp = args = topcopy(args);
       for(i=0; args != NULL; i++)
           args = args->cdrp;
       bindvar(at,temp->carp = newintop((long)i));
       bindvar(blexprhold,temp);
       xpop(1);
}

/*************************************************************************
 ** poplexpr(at) Will handle the exit from an lexpr body. This is done  **
 ** by simply unbinding the global (arg) function holding atom, and then**
 ** we unbind the single atom parameter to the lexpr body.              **
 *************************************************************************/
poplexpr(at)
struct conscell *at;
{      unbindvar(blexprhold);
       unbindvar(at);
}

/*************************************************************************
 ** ListToHunk(l) Will take a list 'l' and return a hunk with elements  **
 ** of the list. First we compute the length of the list, then we ask   **
 ** for an empty hunk with this many elements, then we fill each element**
 ** one by one from the list using GetHunkIndex to get the address of   **
 ** the element pointer for each index.                                 **
 *************************************************************************/
struct hunkcell *ListToHunk(l)
struct conscell *l;
{      struct hunkcell *h; register int n,i;
       struct conscell *c;
       for(c=l,n=0;c!=NULL;n++,c=c->cdrp);      /* compute length of list */
       if ((h = inserthunk(n)) == NULL)         /* get a nilxn hunk */
           return(NULL);                        /* no luck! */
       for(c=l,i=0;i<n;i++,c=c->cdrp)
         *GetHunkIndex(h,i) = c->carp;
       return(h);
}

/*************************************************************************
 ** HunkToList(h) Will construct a list from the elements of h. We do   **
 ** his backwards to avoid stacking too much stuff on the mark stack.   **
 *************************************************************************/
struct conscell *HunkToList(h)
struct hunkcell *h;
{      struct conscell *l,*n; register int i;
       push(l);                                         /* l is visible */
       for(i = h->size-1,l = NULL; i >= 0; i--)
       {   n = new(CONSCELL);
           n->carp = *GetHunkIndex(h,i);
           n->cdrp = l;
           l = n;
       };
       xpop(1);
       return(l);
}

#define         SMARTSLASH           0          /* Get/Set Options flags */
#define         AUTORESET            1
#define         CHAINATOM            2
#define         IGNOREEOF            3
                                                /* DEFAULT SETTINGS */
#define         SMARTSLASHDEFAULT    1          /* do \n interpretation by default */
#define         AUTORESETDEFAULT     0          /* do break level on ERR */
#define         CHAINATOMDEFAULT     0          /* ERR when (ca|dr atom) */
#define         IGNOREEOFDEFAULT     0          /* exit on EOF at top level */

/***************************************************************************
 ** GetOption(o) Will return the setting of global PC-LISP option 'o'. The**
 ** the setting may be changed with SetOption(o,v). This just provides a  **
 ** clean way to get at the PC-LISP options that are not set by changing  **
 ** the value of a lisp global variable. This could be done using an array**
 ** but it would mean removing the constants from the LISP.H file because **
 ** they would depend on the organization of the array. I prefer abstract **
 ***************************************************************************/
                                                           /**/
static int SmartSlashOption = SMARTSLASHDEFAULT;
static int AutoResetOption  = AUTORESETDEFAULT;
static int ChainAtomOption  = CHAINATOMDEFAULT;
static int IgnoreEofOption  = IGNOREEOFDEFAULT;
                                                           /**/
int GetOption(o)
int o;
{   switch (o)
    {  case SMARTSLASH : return(SmartSlashOption); break;
       case AUTORESET  : return(AutoResetOption);  break;
       case CHAINATOM  : return(ChainAtomOption);  break;
       case IGNOREEOF  : return(IgnoreEofOption);  break;
       default         : fatalerror("GetOption");
    };
}
                                                           /**/
SetOption(o,v)
int o,v;
{   switch (o)
    {  case SMARTSLASH : SmartSlashOption = v; break;
       case AUTORESET  : AutoResetOption = v;  break;
       case CHAINATOM  : ChainAtomOption = v;  break;
       case IGNOREEOF  : IgnoreEofOption = v;  break;
       default         : fatalerror("SetOption");
    };
}

/*
 | This function is a useful debugging aid, it allows us to dump an expression from
 | the debugger.
 */
liudump(e)
       struct conscell *e;
{      struct conscell f;
       printf("DUMPING ...\n");
       f.celltype = CONSCELL; f.cdrp = NULL;
       f.carp = e;
       return(buppform(&f));
}

/*
 | Return the length of a list.
 */
int liulength(l)
       struct conscell *l;
{
       register int n;
       for(n = 0; (l != NULL) && (l->celltype == CONSCELL); n++, l = l->cdrp);
       return(n);
}

/*
 | Hash an S-expression into an integer. Just like regular hashing but it
 | works on List structures too. Note that the choice of algorithm is quite
 | random, I do not know how well it really distributes most normal LISP
 | expressions. Note that in the leaf cases we call the regular 'hash' used
 | by the oblist.
 */
int liushash(e)
       struct conscell *e;
{      int i,n;
       if (e == NULL) return(1);
       n = e->celltype;
       switch(n) {
	  case ALPHAATOM : return(n+3  + hash(ALPHA(e)->atom,   strlen(ALPHA(e)->atom)));
	  case REALATOM  : return(n+5  + hash(&(REAL(e)->atom), sizeof(REAL(e)->atom)));
	  case FIXATOM   : return(n+7  + hash(&(FIX(e)->atom),  sizeof(FIX(e)->atom)));
	  case FIXFIXATOM: return(n+19 + hash(&(FIXFIX(e)->atom1), sizeof(FIXFIX(e)->atom1)) +
                                         hash(&(FIXFIX(e)->atom2), sizeof(FIXFIX(e)->atom2)));
	  case STRINGATOM: return(n+9  + hash(STRING(e)->atom, strlen(STRING(e)->atom)));
	  case FILECELL  : return(n+11 + liushash(PORT(e)->fname));
	  case ARRAYATOM : return(n+13 + liushash(ARRAY(e)->base) + liushash(ARRAY(e)->info));
	  case HUNKATOM  : n += 15;
			   for(i = HUNK(e)->size - 1; i >= 0; i--)
			      n += liushash(*GetHunkIndex(e,i));
			   return(n+1);
	  case CONSCELL  : for(n += 17;(e != NULL)&&(e->celltype == CONSCELL);e=e->cdrp)
			      n += liushash(e->carp);
			   return(n+1);
          case CLISPCELL : i = *(((int *) CLISP(e)->code) - 1);
                           return(n + 18 + i + hash(CLISP(e)->code, i));
	  default:         return(0);
       }
}

/*
 | Given the address of a CLISP cells code, try to find the atom on which
 | this clisp was PUTD'd. This means a complete traversal of all the atomtable
 | and each overflow chain until we either find it, or exhaust the search.
 | We do not really care about the cost here because this function is only
 | used in tracebacks involving CLISPs.
 */
char *liuclnam(code)
     char *code;
{    struct conscell  *w;
     extern struct conscell *atomtable[];
     struct alphacell *a; int i;
     for(i = 0; i < ALPHATABSIZE; i++)  {
         w = atomtable[i];
         while(w != NULL)  {
              a = ALPHA(w->carp);
              if ((a != NULL) && (a->celltype == ALPHAATOM))
                 if (FN_ISCLISP(a->fntype) && (CLISP(a->func)->code == code))
                     return(a->atom);
              w = w->cdrp;
         }
     }
     return(NULL);
}

/*
 | Given a FIXFIX cell found on a traceback, try to find the CLISP which uses
 | this FIXFIX and return the name of the atom on which it is bound.
 */
char *liuffnam(ff)
     struct fixfixcell *ff;
{    struct conscell  *w;
     extern struct conscell *atomtable[];
     struct alphacell *a; int i;
     for(i = 0; i < ALPHATABSIZE; i++)  {
         w = atomtable[i];
         while(w != NULL)  {
              a = ALPHA(w->carp);
              if ((a != NULL) && (a->celltype == ALPHAATOM))
                 if (FN_ISCLISP(a->fntype) && (CLISP(a->func)->literal[1] == LIST(ff)))
                     return(a->atom);
              w = w->cdrp;
         }
     }
     return(NULL);
}



