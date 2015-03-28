
/****************************************************************************
 **         PC-LISP (C) 1986 Peter Ashwood-Smith                           **
 **         MODULE: EVAL.C                                                 **
 **------------------------------------------------------------------------**
 **   This module is the key to the LISP interpreter. The function eval()  **
 ** is the root of all activity. It is called with a list and must return  **
 ** the result of evaluating the list. It splits its work up by calling    **
 ** apply() and evlis(). Apply(f,a) : takes a function and a list of args  **
 ** and returns the result of applying the function to its arguments. The  **
 ** evlis() function takes a list of expressions and evaluates them one    **
 ** by one by calling eval() and returns the resulting list of evaluated   **
 ** results. This is used for evaluating the arguments to a function call. **
 ** There are a variety of different kinds of functions, these are built in**
 ** user defined lambda expressions, user defined nlambda expressions, and **
 ** macros. The built in functions will all have been installed with a call**
 ** to the funcinstall() made in function initeval() located in the module **
 ** bufunc.c. The built in functions may be one of two kinds. The first is **
 ** one in which all parameters are evaled before the built in function is **
 ** called, the second is one in which the parameters are not evaled before**
 ** calling the function. cond, is an example of this second type of built **
 ** in function. All built in functions are passed a form which is just a  **
 ** pointer to a list of arguments. The function is responsible for extract**
 ** ing and verifying its arguments. By convention all built in functions  **
 ** are called buXXX hence bucond is the built in cond function. If the    **
 ** built in function detects an error it must call ierror("name"); This   **
 ** function never returns and will print an error and longjmp to the 'er>'**
 ** loop in function main in module main.c. If the built in function ever  **
 ** calls 'new', either directly or indirectly via one of its other func   **
 ** calls, it must make sure that if garbage collection occurs, any temp-  **
 ** orary results will be marked. This is done by requesting marking of    **
 ** a parameter, or local function using xpush(x) or push(x) respectively. **
 ** When the function returns it must then use fret(x,n) or xret(f,n) where**
 ** n is the number of xpush and push operations that must be undone, and  **
 ** x is a variable to be returned or f is a function whose result we wish **
 ** to return. Example1 :  xpush(form); push(temp);....fret(temp,2)        **
 ** or.        Example2 :  xpush(form); push(temp);....xret(eval(temp),2)  **
 ** Correct marking of local variables and temporary results is absolutely **
 ** CRITICAL to the correct operation of the interpreter after a garbage   **
 ** collection cycle. The obvious solution is to mark every local variable **
 ** and all parameters but this will have a tremendous space&time cost.    **
 ** Practice has shown that some very very careful programming must be done**
 ** in even the simplest functions to minimize the number of pushed mark   **
 ** requests, and to ensure that a path to all new cells exists from the   **
 ** marked variables so that they will not be gathered. Don't underestimate**
 ** the difficulty of this task, it has been the hardest thing for me.     **
 ****************************************************************************/

#include <stdio.h>
#include "lisp.h"

/*************************************************************************
 ** buueval: Users call to eval ie Lisp interpreter can call eval       **
 *************************************************************************/
struct conscell *buueval(form)
struct conscell *form;
{      if (form != NULL)
          return(eval(form->carp));
       ierror("eval");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}


/*************************************************************************
 ** buuapply:Users call to apply ie Lisp interpreter can call apply     **
 *************************************************************************/
struct conscell *buuapply(form)
struct conscell *form;
{      register struct conscell *fn, *largs;
       if (form != NULL) {
           fn = form->carp;
           form = form->cdrp;
           if (form != NULL) {
               largs = form->carp;
               if (((largs != NULL)&&(largs->celltype == CONSCELL))
                   ||(largs == NULL))
                  if (form->cdrp == NULL)
                     return(apply(fn,largs));
           }
       }
       ierror("apply");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}

/*************************************************************************
 ** (funcall func arg1 ... argN) Will apply the function func to args   **
 ** arg1 ... argN. This is quite simply done by calling apply with the  **
 ** first arg and the rest of the args as the arg list.                 **
 *************************************************************************/
struct conscell *bufuncall(form)
struct conscell *form;
{      register struct conscell *func,*args;
       if (form != NULL) {
           func = form->carp;
           args = form->cdrp;
           expush(form);
           exret(apply(func,args),1);
       }
       ierror("funcall");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}

/*************************************************************************
 ** autoloadatom(at) - the atom 'at' is not built in, user defined and  **
 ** has no current binding but it used as a function. We check for the  **
 ** property 'autoload' on the atom and if present we use the value as  **
 ** the name of a file to be loaded. We load the file and if the atom   **
 ** now has a user defined function on it we return otherwise we throw  **
 ** the error indicating that the atom is unbound. We return the body   **
 ** of the atom if we manage to load the function.                      **
 *************************************************************************/
static struct conscell *autoloadatom(at)
struct alphacell *at;
{       extern struct alphacell *autoloadhold; char *str;
        extern struct conscell *getprop();
        register struct conscell *s = getprop(at, autoloadhold);
        if ((s != NULL) && GetString(s, &str)) {
            if ( loadfile(str) ) {
               if ( FN_ISUS(at->fntype) || FN_ISCLISP(at->fntype) )      /* now has an fn expr ? */
                   return(LIST(at->func));
            } else
               printf("file %s could not be read to resolve autoload of atom %s\n", str, at->atom);
        }
        bindingerror(at->atom);               /* throw binding error */
	  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}

/*************************************************************************
 ** evlis : evalutate a list of parameters. We just go through the list **
 ** evaluating each element. We construct a new list consisting of the  **
 ** list of evaluated elements. We must create a new list to avoid doing**
 ** harm to constants like function bodys etc.                          **
 *************************************************************************/
struct conscell *evlis(form)
struct conscell *form;
{      register struct conscell *t,*l;
       struct conscell *r;
       if (form == NULL) return(NULL);
       xpush(form); push(r);
       r = new(CONSCELL);
       r->carp = eval(form->carp);
       form = form->cdrp;
       l = r;                                   /**  SUPER CRITICAL CODE!!  */
       while(form != NULL) {                    /**  FOR CORRECT GC ACROSS  */
             if (form->celltype != CONSCELL)
                 ierror("eval");
             t = new(CONSCELL);                 /**  THE NEW AND EVAL CALLS */
             l->cdrp = t;                       /**  READ CAREFULLY  BEFORE */
             t->carp = eval(form->carp);        /**  ALTERING. NOTE PATH FROM */
             form = form->cdrp;                 /**  PUSHED r VARIABLE TO   */
             l = t;                             /**  ALL  EVALS SO FAR!!!!  */
       }
       fret(r,2);
}

/*************************************************************************
 ** evalallbodys(l) We are passed a pointer to a list of expressions we **
 ** will evaluate them all and return the result of the last evaluation.**
 ** This is used to evaluate all the bodys in a lambda expression with  **
 ** multiple bodies. We do not get here unless there really are more    **
 ** than one body to eval, lest we introduce yet another function call  **
 ** to an already very deeply nested set of corecursive routines.       **
 *************************************************************************/
struct conscell *evalallbodys(l)
struct conscell *l;
{      xpush(l);                        /* save it on mark stack */
       while(l->cdrp != NULL) {         /* do until last one in list */
             eval(l->carp);             /* eval and throw away result */
             l = l->cdrp;               /* advance to next body */
       }
       xret(eval(l->carp),1);           /* were at last so return it evaled */
}

/*************************************************************************
 ** apply : the workhorse of the lisp interpreter. We are to apply the  **
 ** function 'fn' to a list of arguements 'largs'. Bindings in valstack.**
 ** First we check if 'fn' is an atom. If so we check to see if it is a **
 ** built in function. If it is we will call the function with the form **
 ** which is justa list of arguments.  If  not  we  check to see if it  **
 ** is a c{a|d}*r class of function. If so we call bucadar to evaluate  **
 ** it. The atom name is passed so that the 'a' and 'd's can direct the **
 ** evaluation. If it is not a cadar type function we will then recall  **
 ** (eval) to see if we can pick up any bindings etc and try again. if  **
 ** fn is not an atom then we have a lambda or label expression and must**
 ** do a binding of actual parameters to formal parameters and eval the **
 ** body of the expression. If label we must temporarily bind body to   **
 ** name of label for recursion without propertly lists. If we have been**
 ** given an array then we must call the generic arrayaccess function   **
 ** with the the largs which will be used to compute the address in it. **
 ** arrayaccess() is located in module bufunc2.c with rest of array junk**
 *************************************************************************/
struct conscell *apply(fn,largs)
struct conscell *fn,*largs;
{      register struct conscell *temp, *temp2;
       xpush(fn); xpush(largs);
       if (fn == NULL) ierror("apply");
       switch(fn->celltype)
       {   case ALPHAATOM:
                if (FN_ISBU(ALPHA(fn)->fntype)) {
                    if (ALPHA(fn)->tracebit == TRACE_ON) {
                        EnterTrace(fn,largs);
                        temp = (*ALPHA(fn)->func)(largs);
                        ExitTrace(fn,temp);
                    } else
                        temp = (*ALPHA(fn)->func)(largs);
#                   if WANTERRNOTESTING
                        if (errno) syserror();
#                   endif
                    fret(temp,2);
                }
                if (FN_ISCLISP(ALPHA(fn)->fntype)) {
                    fn = LIST(ALPHA(fn)->func);
                    xret(evalclisp(CLISP(fn)->code,CLISP(fn)->literal,largs), 2);
                }
                if (ALPHA(fn)->iscadar)
                    xret(bucadar(largs,ALPHA(fn)->atom),2);
                if (FN_ISPRED(ALPHA(fn)->fntype)) {             /* binary predicate? */
                    if (largs != NULL) ierror(ALPHA(fn)->atom); /* must have no args */
                    fret(((((*ALPHA(fn)->func)(ALPHA(fn)->atom)) == 0) ? NULL : LIST(thold)), 2);
                }
                if (ALPHA(fn)->tracebit == TRACE_ON)
                {   EnterTrace(fn,largs);
                    temp = apply(eval(fn),largs);
                    ExitTrace(fn,temp);
                    fret(temp,2);
                }
                xret(apply(eval(fn),largs),2);
           case CONSCELL :
                if (fn->carp== LIST(lambdahold)) {           /* simple lambda expr*/
                    fn = fn->cdrp;                           /* bind actual to  */
                    pushvariables(fn->carp,largs);           /* formal parms and */
                    if ((temp2 = fn->cdrp)->cdrp  != NULL)   /* if multi body lambda */
                        temp = evalallbodys(temp2);          /* ret last body eval*/
                    else                                     /* save stack space & */
                        temp = eval(temp2->carp);            /* only one to eval */
                    popvariables(fn->carp);                  /* then undo bindings*/
                    fret(temp,2);                            /* and return result */
                }
                if ((fn->carp==LIST(nlambdahold))||(fn->carp == LIST(macrohold))) {
                    fn = fn->cdrp;                           /* MACRO or NLAMBDA  */
                    largs = enlist(largs);                   /* CRITICAL! hold for GC! */
                    pushvariables(fn->carp,largs);           /* push listified args */
                    if ((temp2 = fn->cdrp)->cdrp != NULL)    /* if multi body nlambda */
                        temp = evalallbodys(temp2);          /* ret last body eval*/
                    else                                     /* save stack space & */
                        temp = eval(temp2->carp);            /* only one to eval */
                    popvariables(fn->carp);                  /* then undo bindings*/
                    fret(temp,2);                            /* and return result */
                }
                if (fn->carp == LIST(lexprhold)) {           /* lexprs are done */
                    fn = fn->cdrp;                           /* by special push */
                    pushlexpr(temp2=fn->carp->carp,largs);   /* and poplexpr funcs */
                    if (fn->cdrp->cdrp != NULL)
                        temp = evalallbodys(fn->cdrp);
                    else
                        temp = eval(fn->cdrp->carp);
                    poplexpr(temp2);
                    fret(temp,2);
                }
                if (fn->carp == LIST(labelhold)) {           /* label - so temp */
                    fn = fn->cdrp;                           /* bind name to body*/
                    bindvar(fn->carp,temp2=fn->cdrp->carp);  /* apply to args */
                    temp = apply(temp2,largs);               /* then undo the */
                    unbindvar(fn->carp);                     /* binding and ret */
                    fret(temp,2);                            /* the result */
                }
                break;
           case ARRAYATOM:
                xret(arrayaccess(ARRAY(fn)->info,ARRAY(fn)->base,largs),2);
           case CLISPCELL:
                xret(evalclisp(CLISP(fn)->code,CLISP(fn)->literal,largs),2);
       }
       ierror("apply");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}

/*************************************************************************
 ** eval : The function of a lisp interpreter. Evaluating an atom is just*
 ** the binding of the atom. A real cannot be evaluated so we just return*
 ** the real. (ie reals do not have to be quoted) Evaluating a list is  **
 ** just a matter of applying the function to the arguments in the curr **
 ** ent context.If this function is built in we evaluate it with a call **
 ** If it has a lambda expression as property we apply the lambda       **
 ** expression to the evaluated list of arguments. If it is neither then**
 ** the body must be a lambda or label expression so we call 'apply' to **
 ** finish the job. Nlambdas are handled specially by testing to see if **
 ** the first atom in the expression is 'nlambda', if so we do not eval **
 ** its argument list, instead we listify the argument list and call    **
 ** apply, which will threat it just like any lambda expression with one**
 ** argument and a quoted list of parameters. Macro expansion is handled**
 ** by calling apply, then by calling CopyCellIf..to do the work, then  **
 ** by reevauating the expanded form. The macro may or may not replace  **
 ** code in the form list. It will replace code only if the cell type   **
 ** returned by the macro is copyable over the celltype in 'form' this  **
 ** decision is made by CopyCellIf, a function located in mman.c because**
 ** it needs to know physical cell size restrictions, not sizeof info.  **
 *************************************************************************/
struct conscell *eval(form)
struct conscell *form;
{      register struct conscell *temp,*fn,*largs;
       register struct alphacell *fnat;
       TEST_BREAK();                                                     /* exit on break */
       if (form == NULL) return(form);                                   /* No Work Case  */
       switch (form->celltype) {
          case ALPHAATOM:
               if ((temp = ALPHA(form)->valstack) != NULL)
                     return(temp->carp);
               if (FN_ISUS(ALPHA(form)->fntype))                         /* has fn expr ? */
                     return(LIST(ALPHA(form)->func));
               if (FN_ISCLISP(ALPHA(form)->fntype))                      /* has clisp expr ? */
                     return(LIST(ALPHA(form)->func));
               return(autoloadatom(form));                               /* try to autoload it */
          case CONSCELL  :
               expush(form);
               fnat = ALPHA(form->carp);
               if (fnat->celltype == ALPHAATOM) {
                   if (fnat->fntype == FN_BUNEVAL)
                        exret((*fnat->func)(form->cdrp),1);
                   if ((fnat->fntype == FN_CLISP) && (CLISP(fnat->func)->eval == 0))
                        exret( evalclisp(CLISP(fnat->func)->code, CLISP(fnat->func)->literal, form->cdrp), 1);
                   if ((fnat->fntype == FN_NONE) && (!fnat->iscadar) &&
                       (fnat->valstack == NULL))                         /* need autoload ? */
                        autoloadatom(fnat);
                   if (fnat->fntype == FN_USEXPR) {
                       fn = LIST(fnat->func);
                       if (fnat->tracebit == TRACE_ON) {
                           if (fn->carp == LIST(nlambdahold)) {
                               EnterTrace(fnat,form->cdrp);
                               temp = apply(fn,form->cdrp);
                               ExitTrace(fnat,temp);
                               efret(temp,1);
                           }
                           largs = evlis(form->cdrp);
                           EnterTrace(fnat,largs);
                           temp = apply(fn,largs);
                           ExitTrace(fnat,temp);
                           efret(temp,1);
                       }
                       if (fn->carp == LIST(nlambdahold))
                           exret(apply(fn,form->cdrp),1);
                       largs = evlis(form->cdrp);
                       temp = apply(fn, largs);
                       lifreelist(largs);
                       efret(temp, 1);
                   }
                   if (fnat->fntype == FN_USMACRO) {            /** MACRO ?**/
                        fn = LIST(fnat->func);
                        temp = apply(fn,form);
                        CopyCellIfPossible(form,temp);          /* memcpy(form,temp,N) */
                        exret(eval(temp),1);
                   }
               }
               if (LIST(fnat)->carp != LIST(nlambdahold)) {     /* nlambda is not evaluated */
                  largs = evlis(form->cdrp);
                  temp  = apply(LIST(fnat), largs);
                  lifreelist(largs);
                  efret(temp, 1);
               }
               exret(apply(LIST(fnat),form->cdrp),1);

          case CLISPCELL:
               xpush(form);
               xret(evalclisp(CLISP(form)->code, CLISP(form)->literal, NULL), 1);
       }
       return(form);
}


