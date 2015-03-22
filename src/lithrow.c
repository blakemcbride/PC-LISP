/* EDITION AC04, APFUN PAS.813 (92/04/10 13:12:44) -- CLOSED */                 

/**************************************************************************
 **      PC-LISP (C) 1986 by Peter Ashwood-Smith                         **
 **      MODULE THROW (very nonstandard flow of control)                 **
 ** -------------------------------------------------------------------- **
 **                                                                      **
 **     This module implements (throw) & (catch) and (errset) & (err).   **
 ** The errors.c module also calls a variant of (err) to allow it to be  **
 ** cleanly interfaced with the general error handling. A description of **
 ** how these work follows however, if you do not know how setjmp/longjmp**
 ** work then read the manual first. I will explain by example:          **
 **                                                                      **
 **     -->(catch (car (cdr (car (throw 'x)))))                          **
 **     x                                                                **
 **                                                                      **
 **     Operates as follows. Catch is called before its arguments are    **
 ** evaluated, it immediately pushes an <environment> onto a stack which **
 ** is built using the valstack of atom 'catchstkhold'. This <environ>   **
 ** has as first element a pointer to a jmp_buf which when passed to the **
 ** longjmp function will return control to the correct catch. Second is **
 ** the value of the eval mark stack top. Finally is a sequence of tags  **
 ** which in this case is just nil. Then catch evaluates its argument.   **
 ** This results in eval descending the list pushing the lists (car...)  **
 ** onto the eval mark stack until it hits (throw x). It then calls throw**
 ** which pops the 'catchstkhold' stack looking for a tag that matches   **
 ** (nil in this case). It finds one on the top so, if converts the 1st  **
 ** element of the poped list into a jmp_buf address and gets the old top**
 ** of the eval mark stack from the 2nd element. It then makes a call to **
 ** UnwindShallowBindings giving it the ranges on the eval mark stack    **
 ** that correspond to the evals that occured between the (catch) and the**
 ** (throw). This function undoes any bindings that eval would have undone*
 ** had the returns occured. Once this is done (throw) then makes a jmp  **
 ** using longjmp up to the environment of the catch. The catch then     **
 ** resumes at the point after the if (!setjmp()) statement, it resets   **
 ** the mark stack tops and returns the value 'flung'. Oh, this value was**
 ** set by (throw) just before the longjmp and corresponds to the arg of **
 ** (throw).                                                             **
 **************************************************************************/

#include <stdio.h>
#include "lisp.h"

/**************************************************************************
 ** Push an expression on the stack of bindings associated with special  **
 ** atom 'stack'. These expressions are of the form:                     **
 **                                                                      **
 **      (fixnum1 fixnum2 symbol1 symbol2 .... symbolN)                  **
 **                                                                      **
 ** Where fixnum1 is a long int representation of the address of the jmp_**
 ** buf assocaited with the catch handler for this set of tags. The 2nd  **
 ** fixnum is the value the top of the eval mark stack (emytop) had when **
 ** the associated (catch) or (errset) was executed. The symbols 1...N   **
 ** are in the case of catch & throw a set of tags and in the case of    **
 ** errset, the value of the second optional parameter to errset which   **
 ** controls the printing of the error message.                          **
 **************************************************************************/
PushCatchStack(stack,x)
struct alphacell *stack;
struct conscell *x;
{   struct conscell *temp;
    temp = new(CONSCELL);
    temp->carp = x;
    temp->cdrp = stack->valstack;
    stack->valstack = temp;
}

/**************************************************************************
 ** Pop an expression off the stack of bindings associated with special  **
 ** atom 'stack'. Must return NULL if the stack is empty.                **
 **************************************************************************/
struct conscell *PopCatchStack(stack)
struct alphacell *stack;
{   struct conscell *r;
    if ((r = stack->valstack) == NULL) return(NULL);
    stack->valstack = stack->valstack->cdrp;
    return(r->carp);
}

/**************************************************************************
 ** 'flung' the value which is flung by throw to catch. A global will do **
 ** to hold this because it can't get clobbered between the throw/catch. **
 **************************************************************************/
static struct conscell *flung;

/**************************************************************************
 ** (throw exp [ tag ]) will send the value of exp to the nearest catch  **
 ** above us whose tags match. If tag is not present then the first catch**
 ** that has no specific tag set will catch it. We implement this by     **
 ** popping the catchstack whose elements are lists whose first elem is  **
 ** a fixnum that is interpreted as the address of the throw buffer. The **
 ** rest of the elements are tags that are associated with this catch.   **
 ** We then assign the value to return to the static global 'flung' and  **
 ** longjmp back to the catch. The catch will read the value out of the  **
 ** global static 'flung' and return it. Note that PopCatchStack will    **
 ** return NULL when it is empty so we keep popping until we get a NULL  **
 ** if this happens we raise the error "no catch for throw". A value of  **
 ** NULL it a list of tags on the catch stack will match any tag hence   **
 ** a (catch <exp>) or a (catch <exp> '(tag1 ... nil ... tagN)) will     **
 ** catch any throw. Oh, the second element in the lists pushed onto the **
 ** catch stack is the top value of the eval stack 'emytop' as set in the**
 ** (catch) function. We need this value, along with the current eval top**
 ** to provide the upperand lower limits to the UnwindShallowBindings    **
 ** function which will undo all the bindings that were done by the calls**
 ** to eval that are now never going to return.                          **
 **************************************************************************/
struct conscell *buthrow(form)
struct conscell *form;
{      struct conscell *tag,*elem;
       jmp_buf *receiver;
       int oldtop;
       if (form != NULL) {
           flung = form->carp;          /* flung is global see above ^^ */
           form = form->cdrp;
           tag = NULL;
           if (form != NULL) {
               tag = form->carp;
               if ((tag != NULL)&&(tag->celltype != ALPHAATOM)) goto ERR;
               if (form->cdrp != NULL) goto ERR;
           }
           while((elem = PopCatchStack(catchstkhold)) != NULL) {
               lillev = elem->linenum;
               receiver = (jmp_buf *) (FIX(elem->carp)->atom);
               oldtop = (int) FIX((elem = elem->cdrp)->carp)->atom;
               for(elem = elem->cdrp; elem != NULL ;elem = elem->cdrp) {
                  if ((elem->carp == NULL)||(elem->carp == tag)) {
                     UnwindShallowBindings(emytop,oldtop);
                     longjmp(*receiver,1);
                  }
               }
           }
           catcherror(tag == NULL ? "nil" : ALPHA(tag)->atom);
       }
ERR:   ierror("throw");
}

/**************************************************************************
 ** (catch exp [ tag1 | (tag1...tagn)]) Will evaluate exp and catch any  **
 ** throws that are sent to tag1...tagn. We do this by calling setjmp to **
 ** get a jmp_buf. On the first return of setjmp we make a list to push  **
 ** onto the catchstack. This list has a first element that is the addr  **
 ** of the jmp_buff, the other elements are the tag values. We then eval **
 ** the exp and if we return we pop the catch stack and return the value **
 ** of eval(exp). If we don't return then we should return via the second**
 ** setjmp return triggered by 'throw' above. If this happens then we    **
 ** must do two things, first we must unbind all the bindings that were  **
 ** made during the eval call we have an array of all calls that were    **
 ** made sitting on the eval mark stack. We call the function undoeval() **
 ** with the upper and lower limits of the stack. It does the work. We   **
 ** then read the variable 'flung' and return it. A NULL tag matches     **
 ** anything and is the default tag if non is provided. Note that some   **
 ** compilers generate a warning when you do &state where state is type  **
 ** jmp_buf which is probably an array. The error is that the & operator **
 ** is redundant however we do not know if the jmp_buf is implemented as **
 ** a structure in which case we must use the & operator. Leaving out the**
 ** & is therefore not portable even though it shuts up the compiler.    **
 **************************************************************************/
struct conscell *bucatch(form)
struct conscell *form;
{      jmp_buf  state;
       int top1,top2;
       struct conscell *exp,*tag,*temp,**clisp_tos,**getclisptos();
       push(tag); push(temp);                  /* tag = NULL & hold on GC */
       if (form != NULL)
       {   exp = form->carp;
           form = form->cdrp;
           if (form != NULL)
           {   tag = eval(form->carp);
               if (form->cdrp != NULL) goto ERR;
           };
           top1 = mytop;
           top2 = emytop;
           clisp_tos = getclisptos();
           if (!setjmp(state))
           {   if ((tag==NULL)||(tag->celltype!=CONSCELL)) tag = enlist(tag);
               temp = new(CONSCELL);
#              if JMP_BUFISARRAY                              /* no & */
                  temp->carp = LIST(newintop((long)state));
#              else                                           /* need & */
                  temp->carp = LIST(newintop((long)(&state)));
#              endif
               temp->linenum = lillev;                       /* remember lexical level */
               temp->cdrp = new(CONSCELL);
               temp->cdrp->carp = LIST(newintop((long) emytop));
               temp->cdrp->cdrp = tag;
               PushCatchStack(catchstkhold,temp);
               temp = eval(exp);              /* may return to setjmp() */
               PopCatchStack(catchstkhold);
               xpop(2);                       /* pop tag&temp from markstack */
               return(temp);
           };
           mytop = top1;              /* <----- RETURN FROM LONGJMP IN THROW */
           emytop= top2;              /* reset the mark stacks to held tops */
           putclisptos(clisp_tos);    /* and byte coded interpreter stack too! */
           xpop(2);
           return(flung);
       };
ERR:   ierror("catch");
}

/**************************************************************************
 ** (errset exp [exp]) Will evaluate exp and catch any (err 'x) calls    **
 ** that are made. It will also catch any ThrowError calls that are made **
 ** by the internal error routien gerror() in errors.c. The operation is **
 ** almost exactly the same as the 'catch' routine above except that the **
 ** rather than store a tag, we store the second exp or t   on the stack **
 ** errsetstack. When we return we return the flung item. We put the     **
 ** pair (fixnum . exp | t) on the errstkhold atom. Where fixnum is &env.**
 **************************************************************************/
struct conscell *buerrset(form)
struct conscell *form;
{      jmp_buf  state;
       int top1,top2;
       struct conscell *exp,*tag,*temp,**clisp_tos,**getclisptos();
       push(tag); push(temp);                  /* tag = NULL & hold on GC */
       tag = LIST(thold);                      /* default tag */
       if (form != NULL)
       {   exp = form->carp;
           form = form->cdrp;
           if (form != NULL)
           {   tag = eval(form->carp);
               if (form->cdrp != NULL) goto ERR;
           };
           top1 = mytop;
           top2 = emytop;
           clisp_tos = getclisptos();
           if (!setjmp(state))
           {   temp = new(CONSCELL);
#              if JMP_BUFISARRAY                              /* no & */
                  temp->carp = LIST(newintop((long)state));
#              else                                           /* need & */
                  temp->carp = LIST(newintop((long)(&state)));
#              endif
               temp->linenum = lillev;                       /* remember lexical level */
               temp->cdrp = new(CONSCELL);
               temp->cdrp->carp = LIST(newintop((long) emytop));
               temp->cdrp->cdrp = tag;
               PushCatchStack(errstkhold,temp);
               temp = eval(exp);              /* may return to setjmp() */
               temp = enlist(temp);           /* ok return list of value */
               PopCatchStack(errstkhold);
               xpop(2);                       /* pop tag&temp from markstack */
               return(temp);
           }
           mytop = top1;              /* <----- RETURN FROM LONGJMP IN (err) */
           emytop= top2;              /* reset the mark stacks to held tops */
           putclisptos(clisp_tos);
           xpop(2);
           return(flung);
       };
ERR:   ierror("errset");
}

/**************************************************************************
 ** (err exp) Will throw the value of 'exp' up to the closest enclosing  **
 ** errset function (above). We do this by poping the errstkhold atom and**
 ** if it is non null we have an environment to return to so we lonjmp to**
 ** its address. If not we call the normal error message routine with    **
 ** string 'user err'.                                                   **
 **************************************************************************/
struct conscell *buerr(form)
struct conscell *form;
{      jmp_buf *receiver;
       int oldtop;
       struct conscell *elem;
       if ((form != NULL)&&(form->cdrp == NULL)) {
            flung = form->carp;
            elem = PopCatchStack(errstkhold);
            if (elem == NULL) gerror("user err");
            lillev = elem->linenum;
            receiver = (jmp_buf *) (FIX(elem->carp)->atom);
            oldtop = (int) FIX(elem->cdrp->carp)->atom;
            UnwindShallowBindings(emytop,oldtop);
            longjmp(*receiver,1);
       }
       ierror("err");
}

/**************************************************************************
 ** ThrowError(s) The internal version of (err) except that a NULL is    **
 ** returned to an errset via the 'flung' global variable. If there is no**
 ** errset to trap the error we print the error and return to the gerror **
 ** routine in errors.c which called us. Before we longjmp back to the   **
 ** errset routine we check the cdr of the poped element of the errstk   **
 ** and if it is non NULL we print the error string. The errset will have**
 ** set this value to NULL or not accordingly to indicate if it wants the**
 ** error to be printed or not.                                          **
 **************************************************************************/
ThrowError(s)
char *s;
{      struct conscell *elem;
       jmp_buf *receiver;
       int oldtop;
       elem = PopCatchStack(errstkhold);        /* get an errset handler */
       if (elem == NULL) {                      /* none? print err & ret */
          PutErrString(s);
          lillev = 0;
          return;
       }
       lillev = elem->linenum;                 /* restore lexical leval at call time */
       receiver = (jmp_buf *) (FIX(elem->carp)->atom);
       oldtop = (int) FIX((elem = elem->cdrp)->carp)->atom;
       if (elem->cdrp != NULL)
           PutErrString(s);
       flung = NULL;                            /* errset returns NULL */
       UnwindShallowBindings(emytop,oldtop);
       longjmp(*receiver,1);
}

/**************************************************************************
 ** Unwind the Shallow Bindings that were made by eval during 'from' to  **
 ** 'to' pushes onto the eval mark stack. These will be several cases.   **
 **                                                                      **
 **      CASE        EXAMPLE            ACTION                           **
 **      ~~~~        ~~~~~~~            ~~~~~~                           **
 ** eval built in:  (car <exps>)        Nothing no bindings were made.   **
 ** eval prog       (prog (x y z) l..)  Unbind x y z and atoms in l..    **
 ** eval foreach    (foreach obj l..)   Unbind obj and atoms in l..      **
 ** eval repeat     (repeat N l..)      Unbind labels in l..             **
 ** eval while      (while expr .. l..) Unbind labels 'l'..              **
 ** eval array ref: (A 1 2 3 <exp>)     Nothing no bindings were made.   **
 ** eval lambda:    ((lambda(x y z)..)  Pop each of x y & z once.        **
 ** eval nlambda:   ((nlambda(l)...)    Pop the single argument 'l' once.**
 ** eval macro:     ((macro(l)...)      Pop the single argument 'l' once.**
 ** eval lexpr:     ((lexpr(n)...)      Pop 'n and lexprhold once.       **
 ** eval usr func   (func <exps>)       eval func then do case above.    **
 **                                                                      **
 **  Note that while we are looping through the mark stack looking at the**
 ** eval trace, we check that each stack element is indeed a CONSCELL. If**
 ** not we cause a fatalerror because something is wrong. There is a minor*
 ** problem with just unwinding every function named in the trace. Think **
 ** about a trace like (f (f (f (throw 'x)))) where f is a lambda form   **
 ** or has a lambda for associated with it. The trace will contain the   **
 ** (throw 'x) at the top, then (f (throw 'x)) then (f (f (throw 'x)))   **
 ** etc but 'f has not been entered 3 times, in fact 'f has not been     **
 ** entered at all because it is a lambda expression and its formals are **
 ** evaled before being bound to the actuals. The solution here is to    **
 ** check if the previous expression is an argument to the current expr  **
 ** and if so we do not uneval lambda's or lexpr's because their args are**
 ** evaled before they are called. However, macro,nlambda and progs are  **
 ** still unevaled because they do not eval their args before binding.   **
 **************************************************************************/
static UnwindShallowBindings(f,t)
int f,t;
{   int i,IsArg;
    struct conscell *last,*curr,*temp;
    last = *mystack[f+1];               /* temp = (throw <exp>) */
    for(i=f+1;i<=t;i++) {
        curr = *mystack[i];
        if (curr == NULL) goto er;
        switch(curr->celltype) {

               case CONSCELL:
                    for(IsArg = 0,temp=curr;temp != NULL; temp = temp->cdrp) {
                        if (temp->carp == last) {
                            IsArg = 1;
                            break;
                        }
                    }
#                   if DEBUG                                 /* useful debugging */
                       printf("$$ NEXT UNEVAL TO DO $$\n");
                       printlist(stdout,*mystack[i],DELIM_OFF,NULL,NULL);
                       if (IsArg) printf("  <--- above is arg");
                       putchar('\n');
#                   endif
                    UnEval(curr,IsArg);
                    break;

               case FIXFIXATOM:
                    UnZpush(mystack[i], curr);
                    break;

               default:
                    goto er;
        }
        last = curr;
    }
    return;
er: fatalerror("UnwindShallowBindings");
}

/**************************************************************************
 ** Un Evaluate 'l'. In other words, undo the binding side effects that  **
 ** an evaluation of 'l' would have. This is just the cases as described **
 ** in  comments for UnwindShallowBindings above. Basically we are just  **
 ** analysing the situation here and dispatching the popvariables routine**
 ** on the correct parameter lists. The only difficulty is in unbinding  **
 ** prog labels. We dispatch the routien UnProgBody to scan the body of  **
 ** a prog and unbind any symbols found there. The flag 'IsArg' is used  **
 ** to control the UnEval'ing of functions whose arguments are evaluated **
 ** before they are called. These functions say 'f' will leave traces    **
 ** like (f (throw 'x)) on the eval mark stack but because (throw 'x) is **
 ** a parameter to 'f it was evaluated before any bindings were done and **
 ** so 'f' does not need to be undone. In general we can determine if a  **
 ** function needs to be unbound by looking to see if the previous trace **
 ** is an argument to the current trace, if so then no unwinding is done **
 ** for lambda, or lexprs because they were never entered. Macros and    **
 ** nlambda's on the other hand called eval directly and must be unwound.**
 ** Note that if the form (f <args>) is on the eval mark stack then it   **
 ** is possible that the form of f is bound to f not (putd'ed). To check **
 ** for this we must look it f's binding and tail recurse (loop). This   **
 ** following of the binding chain may continue infinitely if something  **
 ** is wrong. To allow for this case a TEST_BREAK is placed in the loop. **
 **************************************************************************/
static UnEval(l,IsArg)
struct conscell *l; int IsArg;
{    struct conscell *car,*cdr;
     car = l->carp;
     for(;;)
     {   if (car == NULL) return;                     /*  ( nil ...) silly! */
         if (car->celltype != ALPHAATOM) break;       /*  ( func <args> )   */
         if (strcmp(ALPHA(car)->atom,"prog")==0)      /*  ( prog (l)...))   */
         {   cdr = l->cdrp;
             popvariables(cdr->carp);
             UnProgBody(cdr->cdrp);
             return;
         }
         if (strcmp(ALPHA(car)->atom,"foreach")==0)   /*  ( foreach atom list <progbody> ... ) */
         {   cdr = l->cdrp;
             unbindvar(cdr->carp);                    /* unbind 'atom' */
             cdr = cdr->cdrp;                         /* advance to 'list' */
             if (cdr != NULL)                         /* if we have ('list'...) */
                 UnProgBody(cdr->cdrp);               /*    treat like regular prog body */
             return;
         }
         if (strcmp(ALPHA(car)->atom,"while")==0)     /*  ( while expr <progbody> ) */
         {   cdr = l->cdrp;
             UnProgBody(cdr->cdrp);
             return;
         }
         if (strcmp(ALPHA(car)->atom,"repeat")==0)     /*  (repeat N <progbody> ) */
         {   cdr = l->cdrp;
             UnProgBody(cdr->cdrp);
             return;
         }
         if (FN_ISUS(ALPHA(car)->fntype))             /* (user func/macro)  */
         {   car = LIST(ALPHA(car)->func);            /*  car is func code. */
             break;                                   /*  drop to CONS case */
         }
         car = ALPHA(car)->valstack;                  /* no func, get the   */
         car = (car != NULL) ? car->carp : NULL;      /* binding and repeat */
     };
     if (car->celltype == CONSCELL)                   /* ( (...) <args> )  */
     {   cdr = car->cdrp;
         car = car->carp;
         if (!IsArg && (car == LIST(lambdahold)))     /* ((lambda (x..z).) */
             popvariables(cdr->carp);
         else
         if (car == LIST(nlambdahold))                /* ((nlambda(l)....) */
             popvariables(cdr->carp);
         else
         if (car == LIST(macrohold))                  /* ((macro(l)......) */
             popvariables(cdr->carp);
         else
         if (!IsArg && (car == LIST(lexprhold)))      /* ((lexpr(n)......) */
         {   popvariables(cdr->carp);
             unbindvar(blexprhold);                   /* the stack of args */
         }
         return;
     }
ERR: fatalerror("UnEval");
}

/**************************************************************************
 ** Un Prog Body- will run through a list (which happens to be the body  **
 ** of a prog) and look for symbols. Every symbol (ALPHAATOM) is a label **
 ** so we unbind the symbol. If an element is not an ALPHAATOM it is an  **
 ** expression that was evaluated so we skip it.                         **
 **************************************************************************/
static UnProgBody(l)
struct conscell *l;
{   while(l != NULL) {
         if ((l->carp != NULL)&&(l->carp->celltype == ALPHAATOM))
             unbindvar(l->carp);
         l = l->cdrp;
    }
}

/**************************************************************************
 ** Un ZPUSH will invert the SPUSHNIL SPUSHWARG etc. instructions that   **
 ** follow the current ZPUSH instruction's literal. The literal's ptr    **
 ** literal_p actaully points into the literal array so to get the clisp **
 ** we must back-up to literal[0] which contains the clisp, from their   **
 ** we add the offset to get the instruction following the ZPUSH and just**
 ** decode forwards until we find something that is not a SPUSH* instr.  **
 ** each time we pop the SPUSH'ed <literal>'s atom's shallow stack by 1. **
 **************************************************************************/
static UnZpush(literal_p, literal)
struct conscell **literal_p; struct fixfixcell *literal;
{
       register char *ip; register int tmp;
       register struct clispcell *clisp;
       register struct conscell *atm;
       literal_p -= literal->atom1;                                     /* back-up to literal[0] */
       clisp = CLISP(*literal_p);                                       /* follow literal[0] to clisp */
       if ((clisp == NULL) || (clisp->celltype != CLISPCELL)) goto er;  /* SANITY: make sure we got to a clisp */
       if  (clisp->literal != literal_p) goto er;                       /* and that the clisp points back to us */
       for(ip = clisp->code + literal->atom2; ; ip += 2) {
           tmp = *ip++;
           if ((tmp != OP_SPUSHARGS) && (tmp != OP_SPUSHLEX) &&
               (tmp != OP_SPUSHNIL)  && (tmp != OP_SPUSHWARG)) break;
           tmp = XSHORT(ip);
           atm = literal_p[tmp];
#          if DEBUG                                                     /* useful debugging */
              printf("$$ NEXT UnZpush about to unbind for (lit#=%ld, offset=%ld) <lit>=%d atom:$$\n",
                     literal->atom1, literal->atom2, tmp);
              printlist(stdout,atm,DELIM_OFF,NULL,NULL);
              putchar('\n');
#          endif
           if ((atm->celltype != ALPHAATOM) || (ALPHA(atm)->valstack == NULL)) goto er;
           unbindvar(atm);
       }
       return;
 er:   fatalerror("UnZpush");
}

/**************************************************************************
 ** Put an error to stdout. If it is the "Interrupt" error string then   **
 ** put out a new line to help when interrupting I/O. Otherwise no newline*
 ** is used to keep from putting too many blanks on the screen. This is  **
 ** just a question of personal taste.                                   **
 **************************************************************************/
PutErrString(s)
char *s;
{    if (strcmp(s,INTERRUPT)==0) putchar('\n');
     printf("--- %s ---\n",s);
}
