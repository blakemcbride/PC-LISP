

/*
 | PC-LISP (C) 1984-1990 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include "lisp.h"

/*
   This is the (compile <expr>) function. It returns the byte coded instructions
   equivalent to <expr> and a literal reference list.

   Then there is the (assemble <expr>) function which takes the output of the
   (compile <expr>) function and will convert the instructions to real binary.

   Here is a list of all instructions supported by the byte coded interpreter
   There position in the table is their byte code starting with ATOM having byte
   code 01. <16> means a 16 bit fields follows hence the CALL instruction uses
   <16>+<16>+<8> bytes total.

  ARGS    INST                    DESCRIPTION
-------------------------------------------------------------------------------------------
          ARG                     | pops top element for index then push arg with this index
          ARG?                    | N and E are popped and either (arg N) is pushed or E if no arg N
          ATOM                    | pops top element pushes T if it was an atom
 <16><16> CALL     <n> <lit>      | call the function <lit> with <n> args
 <16><16> CALLNF    <n> <lit>     | identical to CALL but does not free arg lists (used for nlambda calls)
          CAR                     | pops stack and pushes its car
          CDR                     | pops stack and pushes its cdr
          CONS                    | pops stack twice allocs a cons and pushes it
          COPYFIX                 | pops a fixnum creates a copy and pushes back
          DDEC                    | destructive decrement of fixnum on stack
          DEC                     | constructive decrement of fixnum on stack
          DUP                     | push a copy of current stack top
          EQ                      | pops top two elements pushes T if eq else NIL
          FIXP                    | pops top element pushes T if it was a fixnum
          FLOATP                  | pops top element pushes T if it was a float
          HUNKP                   | pops top element pushes T if it was a hunk
          INC                     | increment the fixnum on stack
 <16>     JEQ      <label>        | pop stack twice, if eq or '==' jump to <label>
 <16>     JNIL     <label>        | pop stack, if nil jump to <label>
 <16>     JNNIL    <label>        | pop stack, if non nil jump to <label>
 <16>     JUMP     <label>        | unconditionally branch to <label>
 <16>     JZERO    <label>        | pop stack, if fixnum 0 jump to <label>
          LENGTH                  | pop stack and push fixnum length of list on stack
          LISTIFY                 | pop stack for count then push list of args indexed from it
          LISTP                   | pops top element pushes T if it was a list
          NULL                    | pops top element pushes T if is NIL else NIL
          NUMBP                   | pops top element pushes T if it was a fix or flo
          POP                     | pop the stack once
 <16>     PUSHL    <lit>          | push a literal on stack
 <16>     PUSHLV   <lit>          | push value of atom ref by <lit>
          PUSHNIL                 | pushes a 'nil' on the stack
          PUSHT                   | pushes a 't' on the stack
          PUTCLISP <lit> <lit>    | like putd ie on <lit1> atom put clisp body <lit>
          RCALL    <n>            | make a recursive call to ourseves with <n> parms
          RETURN                  | pop stack and return it to caller
 <16>     SETQ     <lit>          | pop stack and bind atom <lit> to it
 <16>     SPOP     <lit>          | pops scope on atom ref by <lit>
 <16>     SPUSHLEX <lit>          | pushes length of args list as scope of <lit>
 <16>     SPUSHNIL <lit>          | pushes new nil scope on atom ref by <lit>
 <16>     SPUSHWARG<lit>          | push next arg value on atom ref by <lit> and wind in arg list by one
 <16>     STORR    -N             | pop stack and overwrite value -n below with it
          STRINGP                 | pops top element pushes T if it was a string
          ZEROP                   | pops top element pushes T if 0 else NIL
 <16>     ZPOP     <lit>          | pop current literal# & ip offset as FIXFIX on eval markstack
 <16>     ZPUSH    <lit>          | inverse of above, used for unwinding shallow bindings during throw.
*/

typedef void (*fptr)();


static long bu_litref();
static void bu_compile_list();
static void bu_compile_func_args();
static void bu_compile_literal();
static int  bu_compile_arg_list();
static void bu_compile_func_list();
static void bu_compile_lambda_body();
static void bu_compile_nlambda_body();
static void bu_compile_lexpr_body();
static void bu_compile_cadr();
static fptr bu_lookup_compile_func();


/*
 | Number of timese user has pressed CONTROL-C (SIGINT) while we are compiling. If
 | ever this goes positive the compiler skips compiling expressions, and just before it
 | exits it generates an ierror(INTERRUPT). The compiler actually traps all errors so
 | as to properly exit/reset.
 */
static int breaks_while_compiling = 0;

/*
 | When an expression fails to compile we jump up to this handler this allows us
 | to generate a backwards dump of what we were compiling at the time of the error.
 */
static jmp_buf *compile_expr_jmp_buf = NULL;

/*
 | The current express list is a list of expression we are currently compiling from
 | bottom to top. It is a list of conscells stored locally in the compile_expr frames.
 */
static struct conscell *curr_expr = NULL;

/*
 | This is a list of all errors for the current compile. It is eventually added as the
 | second argument of the returned ($$clisp$$ <erlist> <lits> <code>) object returned
 | by the compile primitive.
 */
static struct conscell *errlist = NULL;

/*
 | This function is called when an error occurs compiling something. It generates a list consisting
 | of a (reason loc <expr> ...) which is then placed on the front of the errlist which will
 | eventually make its way onto into the returned $$clisp expression as the second element.
 */
static void cerror(msg, loc)
       char *msg;
       struct conscell *loc;
{
       struct conscell *head, *n;
       push(head);

      /*
       | Make a list of (loc <expr> <expr> ...) where loc is where the error was reported and
       | <expr> .. is the trackeback of expressions being compiled by compile_expr.
       */
       head = new(CONSCELL);
       head->carp = loc;
       head->cdrp = topcopy(curr_expr);

      /*
       | Put the reason on the front of the list.
       */
       n = new(CONSCELL);
       n->cdrp = head;
       head = n;
       head->carp = LIST(insertstring(msg));

      /*
       | Now put this reason on the front of the errlist
       */
       n = new(CONSCELL);
       n->cdrp = errlist;
       errlist = n;
       n->carp = head;

      /*
       | Now get out of here and back to the closes enclosing compile_expr so that compilation
       | can continue.
       */
       xpop(1);
       if (compile_expr_jmp_buf) longjmp(*compile_expr_jmp_buf, 1);
}

/*
 | A bucompile() has just finished on a lower level expression. The ($$clisp$$ <erlist> <lits> <code>)
 | expression has just been passed to us. It is our job to set the <erlist> to nil and append this list
 | of errors destructively to it. The reason for this is that we want the top level bucompil call to
 | contain as the second element of its returned list a list of all the errors in all subcompilations
 | below it.
 */
static void bu_hoist_errors(clisp)
       struct conscell *clisp;
{
       register struct conscell *l;
       if (!clisp || (clisp->celltype != CONSCELL)) return;
       if (!(clisp = clisp->cdrp) || (clisp->celltype != CONSCELL)) return;
       if (errlist == NULL)
           errlist = clisp->carp;
       else {
           for(l = errlist; l->cdrp != NULL; l = l->cdrp);
           l->cdrp = clisp->carp;
       }
       clisp->carp = NULL;
}

/*
 | We use the symtab functions from C as they are extremely useful for
 | working with labels and literals.
 */
extern struct conscell *busymtcreate(), *busymtmember(), *busymtadd(), *busymtlist();

/*
 | Reverse utility used for destructive expression list generation.
 */
extern struct conscell *nreverse();

/*
 | The compiler is used recursively so we must declare it forward.
 */
struct conscell *bucompile();

/*
 | This is a pointer to the ATOM which when calls are detected to will cause compile
 | to generate (RCALL <n>) instructions. When the bucompile function is called it
 | remembers the current value of this global and assigs the passed parameter to this
 | global which takes effect for the duration of the compile call. Then, when the
 | compile is done it restores the old value of this global so that (compile) can
 | be used recursively. Currently compile is not used recursively.
 */
static struct alphacell *rcall_atom = NULL;

/*
 | Emit list and the EMIT{1,2,..} macros which will add an element to the front of
 | the emit list, this is just like emitting instructions only in reverse. As result
 | when the list is complete, we reverse it to get the proper order.
 |
 |                            eml
 |                             |         { all the EMIT1*'s add to eml }
 |                             v
 |         all -->   (    (PUSHNIL)      { and EMITEND finishes an instruction }
 |                        (POP)          { and then appends it to 'all' }
 |                          :
 |                          :
 |                   )
 */
static struct conscell *all, *eml;

/*
 | EMIT1ATM - will add to the current output instruction the atom 'e'.
 */
static EMIT1ATM(s)
       char *s;
{      struct conscell *t = new(CONSCELL);
       t->cdrp = eml;
       eml = t;
       t->carp = LIST(CreateInternedAtom(s));
}

/*
 | EMIT1FIX - will add to the current output instruction the fixnum 'e'.
 */
static EMIT1FIX(e)
       long e;
{      struct conscell *t = new(CONSCELL);
       t->cdrp = eml;
       eml = t;
       t->carp = LIST(newintop(e));
}

/*
 | EMIT1ATM - will add to the current output instruction the compiler label 'e'
 */
static EMIT1LAB(d)
       int d;
{
       char name[32];
       sprintf(name,"l%d",d);
       EMIT1ATM(name);
}

/*
 | EMIT1ATM - will output a complete compiler label prefixed with 'l'.
 */
static EMITLABEL(e)
       int e;
{      struct conscell *t = new(CONSCELL);
       char name[32];
       t->cdrp = all;
       all = t;
       sprintf(name,"l%d",e);
       t->carp = LIST(CreateInternedAtom(name));
       eml = NULL;
}

/*
 | EMIT1ATM - will end the current instruction and append to list of all so far.
 */
static EMITEND()
{      struct conscell *t = new(CONSCELL);
       t->cdrp = all;
       all = t;
       t->carp = nreverse(eml);
       eml = NULL;
}

/*
 | Obviously the set of labels.
 */
static int nextLabel = 1;               /* must start at 1 as we use -ves */
#define NEWLABEL() nextLabel++;

/*
 | The goto scope. When we enter compile_prog_bodies this increases, when we exit
 | it decreases. While compiling a list of bodies the goto scope is bound to
 | each label in the list of bodies and any (gotos) will check the binding
 | before allowing the goto. This is to control cross scope jumping which we
 | do not allow.
 */
static int gotoScope = 0;

/*
 | This is the list of literals used by a complete function. The first allocated
 | literal is '1' because we always use literal[0] as a pointer to ourself.
 */
static struct conscell *litset = NULL;
static int nextLit = 0;

/*
 | emitZPUSH - will generate a new FIXFIX cell for use as a literal by the assembled
 | code. These are the literals used by ZPUSH <lit> and contain the literal # and
 | offset into the code. Since we do not know the offset it must be populated by the
 | assembler later. The code is ZPUSH <lit> where lit is FIXFIX(lit#, 0).
 */
static emitZPUSH()
{
       EMIT1ATM("ZPUSH");
       EMIT1FIX(bu_litref(newfixfixop((long)nextLit + 1L, 0L)));
       EMITEND();
}

/*
 | This is the stack of return target labels. When a (repeat, while ...) is
 | entered the return label is pushed on the stack so that when the (return)
 | function is compiled we can compile a jump to the appropriate label. To
 | save static space the real stack is allocated on the frame for the compile
 | function and this is just a pointer to it.
 */
#define MAX_RETURN_STACK 32             /* at most 32 nested while/foreach/prog etc */
static int *return_stack_top = NULL;
static int  return_stack_free = MAX_RETURN_STACK;

/*
 | Push label onto the return stack pushing a 0 means that in the current context
 | return is not allowed.
 */
static return_push(label)
       int label;
{      if (return_stack_free <= 0)
           cerror("return: not allowed in this context", NULL);
       *return_stack_top++ = label;
       return_stack_free -= 1;
}

/*
 | Pop label from the return stack.
 */
static return_pop()
{      if (return_stack_free >= MAX_RETURN_STACK)
           cerror("return: compiler return stack limit exceeded", NULL);
       return_stack_top--;
       return_stack_free += 1;
}

/*
 | Return 1 if the top label on the return stack of labels has been processed
 | by a return statement. It is set negative when processed so just return true
 | if the top label on the stack is negative. (see next function for details).
 */
static int return_called()
{      if (return_stack_free >= MAX_RETURN_STACK)
           cerror("return: compiler return stack limit exceeded", NULL);
       return( *(return_stack_top - 1) < 0 );
}

/*
 | Compile the return which is simply a jump to the current return label
 | on the return stack. We set the label negative to indicate that it has
 | been used so that the while etc code can be adjusted to have this label
 | for processing returns. This is necessary because special code is often
 | required to pop off the stack the while or repeat control expressions.
 | If no args are provided we must return NIL from expression being compiled
 | otherwise the argument has already been compiled.
 */
static void bu_compile_return(args)
       struct conscell *args;
{      int *top; int label;
       if (return_stack_free >= MAX_RETURN_STACK) goto er;
       if (args == NULL) { EMIT1ATM("PUSHNIL"); EMITEND(); }
       top = return_stack_top-1;
       if (*top == 0) goto er;
       label = *top;
       if (label < 0) label = -label; else *top = -label;
       if (args && (bu_compile_arg_list(args) != 1)) goto er;
       EMIT1ATM("JUMP"); EMIT1LAB(label); EMITEND();
       return;
er:    cerror("return: illegal; not allowed in this context",NULL);
}

/*
 | Look up and enter if necessary into the symbol table the literal lit and retun its
 | reference number 'i'. We start at 0 and work up. We use a symtab to keep track of
 | the objects and build a static list of up to three elements to create the 'focms'
 | to call busymtab* functions. Note that often this function is called with:
 |
 |       bu_litref(new<something>cell())
 |
 | hence we must xpush the argument to protect it against G.C.
 */
static long bu_litref(lit)
       struct conscell *lit;
{
       struct conscell c1, c2, c3, *pair; long ret;
       xpush(lit);                                         /* must mark for GC */
       c1.celltype = c2.celltype = c3.celltype = CONSCELL;
       c1.carp = c2.cdrp = NULL;
       c1.cdrp = &c2;
       c2.carp = LIST(thold);
       if (litset == NULL) litset = busymtcreate(&c1);     /* (symtab-create NULL 't) { we want 'eq' collisions} */
       c1.carp = litset;
       c2.carp = lit;
       if (pair = busymtmember(&c1))
           ret = FIX(pair->cdrp)->atom;
       else {
           nextLit += 1;
           c3.carp = newintop((long) nextLit);
           xpush(c3.carp);
           c3.cdrp = NULL;
           c2.cdrp = &c3;
           busymtadd(&c1);
           xpop(1);
           ret = nextLit;
       }
       xpop(1);
       return(ret);
}

/*
 | This is the symbol table of (symbol x type) values. It is unique throughout a compile
 | and all recursive compiles within that compile. The bu_declare function is used to
 | add to and check clashes against it.
 */
static struct conscell *typeset = NULL;

/*
 | This function returns -1 if the given 't' is not of the form "lambdaN" otherwise if the
 | atom 't' is of the form "lambdaN" it returns the value of N. These forms of type are used
 | to indicate a lambda of N arguments. eg (defun fred(x) ..) has (fred . lambda1) as an entry
 | in the typeset table.
 */
static int bu_lambda_n_type(t)
       struct conscell *t;
{
       int n;
       if (t == NULL) return(-1);
       if (t->celltype != ALPHAATOM) return(-1);
       if (sscanf(ALPHA(t)->atom, "lambda%d", &n) != 1) return(-1);
       return(n);
}

/*
 | This function will return 1 if there was a type clash and 0 if there was no type clash
 | against type 't1' and the list containing type 'typel'. The 't1' is what is being
 | declared and 'typel' is what is currently known about the type in the symbol table. Types
 | are described below:
 |
 |     nlambda     => declared as variable # of args non evaluated
 |     lambda0..N  => declared as taking N args and evaluated.
 |     lexpr       => declared as variable # of args evalauted.
 |     0...M       => symbol called with M args declaration not seen yet.
 |     macro       => symbol is a macro.
 |     NULL        => symbol's type not yet known.
 |
 | The 0..M types are called pseudo-types, that is we do not know what the type is yet but
 | we have already compiled then with M arguments. This is used to compare past limited type
 | information with an upcomming declaration if any. If information is gathered that allows
 | us to update the table stored type we do so by modifying destructively the car of typel.
 | The entry stored in the symtab is (symbol (type . t | NULL)) where the type is as described
 | above and a 't' cdr indicates that the symbol is NOT to be compiled. NULL indicates that
 | compile is to occur.
 */
static int bu_type_clash(t1, typel)
       struct conscell *t1, *typel;
{
       struct conscell *t2; long m1, m2;
       if ((typel == NULL)||(typel->celltype != CONSCELL)) return(0);     /* should not happen */
       t2 = typel->carp;
       if (equal(t1, t2)) return(0);                                      /* trivial case diagonal of truth table */
       if (t2 == NULL) { typel->carp = t1; return(0); }                   /* NULL type is ok */
       if (t1->celltype == FIXATOM) {                                     /* seen with m1 arguments */
           m1 = FIX(t1)->atom;
           if (t2->celltype == FIXATOM) {                                 /* in past seen with m2 arguments */
               m2 = FIX(t2)->atom;
               if (m1 != m2) typel->carp = LIST(lexprhold);               /* if seen with diff # of args assume lexpr */
               return(0);                                                 /* ok in either case since compile before declare ok */
           }
           if (t2 == LIST(lexprhold)) return(0);                          /* just seen with m1 args and declared as lexpr is ok */
           if (t2 == LIST(nlambdahold)) return(0);                        /* just seen with m1 args and declared as nlambda is ok */
           if (t2 == LIST(macrohold)) return(0);
           m2 = bu_lambda_n_type(t2);                                     /* if declared as lambdaM2 then clash if m1 != m2 */
           return(m1 != m2);
       }
       if (t2->celltype == FIXATOM) {                                     /* seen in past with t2 (ie m1) arguments but no decl yet */
           m2 = FIX(t2)->atom;
           if (t1 == LIST(lexprhold)) {
               typel->carp = LIST(lexprhold);
               return(0);                                                 /* new declaration as lexpr is fine so update the decl */
           }
           m1 = bu_lambda_n_type(t1);                                     /* if new lambdaM1 decl found then update decl and clash if */
           if (m1 >= 0) {                                                 /* previously seen with different number of args */
               typel->carp = t1;
               return(m1 != m2);
           }
       }
       typel->carp = t1;                                                  /* update table with new type for this symbol */
       return(1);                                                         /* default case, a type clash occurred */
}

/*
 | Declare a symbol as occuring on a particular line number in the typeset table. We simply create the
 | typeset if it does not exist, check to see if the symbol is in the typeset, if not we create it with
 | entry (symbol (NULL)) entry. This will later expand to (symbol (lambdaN . T | NULL)) later where the
 | lambdaN represents the function type and the T | NULL is a flag indicating if compilation is desired
 | or not.
 */
static struct conscell *bu_declare_symbol(symbol, linenum)
       struct conscell *symbol; int linenum;
{
       struct conscell c1, c2, c3, *pair;
       c1.celltype = CONSCELL;
       c1.carp = c1.cdrp = NULL;
       if (typeset == NULL) typeset = busymtcreate(&c1);
       c1.carp = typeset;
       c1.cdrp = &c2;
       c2.celltype = CONSCELL;
       c2.carp = symbol;
       c2.cdrp = NULL;
       if ((pair = busymtmember(&c1)) == NULL) {
           c3.celltype = CONSCELL;
           c3.carp = enlist(NULL);
           xpush(c3.carp);
           c3.carp->linenum = linenum;    /* remember line number it came from for clash errors below */
           c3.cdrp = NULL;
           c2.cdrp = &c3;
           busymtadd(&c1);
           xpop(1);
           return(c3.carp);
       }
       return(pair->cdrp);
}

/*
 | Declare the fact that symbol is of the given type. If symbol already has a declaration
 | of a different type then generate an error. The type is stored as a list containing the
 | type, this is so that any line # info on the type is not lost and allows us to generate
 | good clash messages with the original line # of the declaration and the clashing line
 | number. This function returns the real type so that if fexpr's are called the argument
 | compilation will be suppressed.
 */
static struct conscell *bu_declare(symbol, type, linenum)
       struct conscell *symbol, *type; int linenum;
{
       struct conscell *typel;
       if ((symbol == NULL)||(symbol->celltype != ALPHAATOM)) return NULL;
       xpush(symbol);
       xpush(type);
       typel = bu_declare_symbol(symbol, linenum);
       if (typel->carp == NULL)
           typel->carp = type;
       else {
           if (bu_type_clash(type, typel)) {
               char msg[MAXATOMSIZE + 80];
               sprintf(msg, "function type clash on symbol '%s' (first declared on line# %d)",
                             ALPHA(symbol)->atom, typel->linenum);
               cerror( msg, type);
           }
       }
       xpop(2);
       return(typel->carp);            /* return the actual type */
}

/*
 | An symbol is being compiled as a function type. Look at the 'func' type and determine what type to add
 | to the symbol table for this function body.
 */
static void bu_declare_func(symbol, func)
       struct conscell *symbol, *func;
{
       struct conscell *type;
       if ((func == NULL) || (func->celltype != CONSCELL)) return;
       type = func->carp;
       if ((type == NULL) || (type->celltype != ALPHAATOM)) return;
       if (type == LIST(lambdahold)) {
           int n; char name[32];
           if ((func->cdrp == NULL) || (func->cdrp->celltype != CONSCELL)) return;
           n = liulength(func->cdrp->carp);
           sprintf(name, "lambda%d", n);
           type = LIST(CreateInternedAtom(name));
       }
       bu_declare(symbol, type, func->linenum);
}

/*
 | A symbol is being declared as nocompile. This causes the cdr of the type list entry to be set to
 | T. We lookup this symbol and if it exists we set the nocompile flat to T. If it does not exist we
 | enter it and set its type to NULL.
 */
static void bu_declare_nocompile(symbol, linenum)
       struct conscell *symbol;
{
       struct conscell *typel;
       if ((symbol == NULL)||(symbol->celltype != ALPHAATOM)) return;
       typel = bu_declare_symbol(symbol, linenum);
       typel->cdrp = LIST(thold);
}

/*
 | Preset the types for certain built in functions to be 'nlambda' so that compiled calls to
 | them do not compile their arguments.
 */
static void bu_pretype()
{
       bu_declare(LIST(CreateInternedAtom("sstatus")), LIST(nlambdahold), 0);
       bu_declare(LIST(CreateInternedAtom("def"))    , LIST(nlambdahold), 0);
       bu_declare(LIST(CreateInternedAtom("array"))  , LIST(nlambdahold), 0);
       bu_declare(LIST(CreateInternedAtom("exec"))   , LIST(nlambdahold), 0);
       bu_declare(LIST(CreateInternedAtom("trace"))  , LIST(nlambdahold), 0);
       bu_declare(LIST(CreateInternedAtom("untrace")), LIST(nlambdahold), 0);
}

/*
 | Test a symbol against the typeset to see if it supposed to be compiled or not. If it has
 | the nocompile option we return 0, otherwise we return 1. The nocompile option is set if
 | the cdr of the type is of the form (nlambda . t) otherwise if it is of the form (nlambda)
 | then the nocompile option is not set.
 */
static int bu_want_compile(symbol)
       struct conscell *symbol;
{
       struct conscell c1, c2, *pair;
       if (typeset == NULL) return(1);
       c1.celltype = c2.celltype = CONSCELL;
       c1.carp = typeset;
       c1.cdrp = &c2;
       c2.carp = symbol;
       c2.cdrp = NULL;
       if ((pair = busymtmember(&c1)) == NULL) return(1);
       return(pair->cdrp->cdrp == NULL);
}

/*
 | An atom reference is being made so compile the fixnum portion of the instruction
 | that is the literal reference to the atom. At the same time store the atom in the
 | literals set under the given literal number.
 */
static bu_compile_atom(a)
       struct alphacell *a;
{
       EMIT1FIX((long)bu_litref(a));
}

/*
 | Sorted table of all functions which are built in and side effect free, this means that we can fold expressions
 | involving them. For example (length (append (list 'a 'b 'c) '(d e f))) will fold to the fixnum 6.
 */
static char *bu_sifs[] = { "*", "+", "-", "/", "1+", "1-", "<", "=", ">", "abs", "acos", "add", "add1", "alphalessp", "and",
                           "append", "ascii", "asin", "assoc", "atan", "atom", "car", "cdr", "character-index", "cons",
                           "cos", "diff", "difference", "eq", "equal", "evenp", "exp", "explode", "exploden", "expt",
                           "fact", "fix", "fixp", "flatc", "flatsize", "flatten", "float", "floatp", "greaterp", "last",
                           "ldiff", "length", "lessp", "list", "listp", "log", "log10", "lsh", "max", "member", "memq",
                           "min", "minusp", "mod", "not", "nth", "nthcdr", "nthchar", "null", "numberp", "numbp", "oddp", "or",
                           "pairlis", "plus", "plusp", "product", "quotient", "reverse", "sin", "sizeof", "sprintf", "sqrt",
                           "strcomp", "stringp", "strlen",  "strpad", "strtrim", "subst", "substring", "sum",
                           "times", "tolower", "toupper", "zerop"
                         };

/*
 | Given the name of a function, return true if it is a built in function that is side effect
 | free. These are all functions in the sifs[] table above. Any expression involving simple
 | composition of the above functions will be (eval'ed) to fold it to its minimum form before
 | compiling.
 */
static int bu_is_sef_bu(l)
       struct conscell *l;
{
       register char *satom;
       register int mid, r, lo, hi;
       if ((l == NULL) || (l->celltype != ALPHAATOM)) goto no;
       if (ALPHA(l)->fntype != FN_BUEVAL)             goto no;
       satom = ALPHA(l)->atom;
       lo = 0; hi = (sizeof(bu_sifs)/sizeof(bu_sifs[0])) - 1;
       while(lo <= hi) {
           mid = (lo + hi)/2;
           r = strcmp(satom, bu_sifs[mid]);
           if (r == 0) goto yes;
           if (r < 0) hi = mid-1; else lo = mid+1;
       }
no:    return(0);
yes:   return(1);
}

/*
 | Given an expression return true if it is a constant expression.
 | Constant function expressions MUST have at least 1 argument otherwise
 | they have side effects (ie read etc.)
 */
static int bu_is_a_constant(l)
       struct conscell *l;
{
       if ((l == NULL) || (l == LIST(thold))) goto yes;
       switch(l->celltype) {
          case FIXATOM: case REALATOM: case STRINGATOM:
               goto yes;
          case CONSCELL:
               if (l->carp == LIST(quotehold)) goto yes;
               if (l->cdrp == NULL) goto no;
               if (!bu_is_sef_bu(l->carp)) goto no;
               for(l = l->cdrp; l != NULL; l = l->cdrp) {
                   if (l->celltype != CONSCELL) goto no;
                   if (!bu_is_a_constant(l->carp)) goto no;
               }
               goto yes;
       }
no:    return(0);
yes:   return(1);
}

/*
 | Given an expression fold it if it is constant but don't try to fold (quote ...)
 | this is compiled as a literal directly. After folding the expression may require
 | quoting so add (quote <expr>) if not a trivial leaf.
 */
static struct conscell *bu_fold_constant_expr(l)
       struct conscell *l;
{
       struct conscell *o, *p;
       if ((l == NULL) || (l->celltype != CONSCELL)) goto no;
       if (l->carp == LIST(quotehold)) goto no;
       if (!bu_is_a_constant(l)) goto no;
       o = eval(l);
       if (!o ||(o == LIST(thold))||(o->celltype == STRINGATOM)||(o->celltype == FIXATOM)||(o->celltype == REALATOM))
           return(o);
       xpush(o);
       o = enlist(o);
       p = new(CONSCELL);
       p->cdrp = o;
       p->carp = LIST(quotehold);
       xpop(1);
       return(p);
 no:   return(l);
}

/*
 | Compile a single expression. For nil, t or atom we generate code to push
 | nil, t or the atoms value on the stack for a list we compile the list with
 | a sub function, for fixnums and flonums we generate code to push the fixnum
 | or flonum on the stack respectively.
 */
static void bu_compile_expr(l)
       struct conscell *l;
{
       struct conscell cc;

      /*
       | Error handling, set up a global 'compile_expr_jmp_buf' to which the
       | lower level routines can longjmp when an error occurs.
       */
       int curtop, ecurtop;
       jmp_buf env;
       jmp_buf *save_jmp_buf;

      /*
       | If SIGINT has been received then get the heck out of here now this will
       | force the (compile) to end pretty quickly since no actual work will be
       | done.
       */
       if (breaks_while_compiling > 0) return;

      /*
       | Set up jump buffer and save previous jump buffer.
       */
       save_jmp_buf = compile_expr_jmp_buf;
#      if JMP_BUFISARRAY
          compile_expr_jmp_buf = (jmp_buf *) env;
#      else
          compile_expr_jmp_buf = &env;
#      endif
       if (setjmp(env)) goto cleanup;

      /*
       | Remember stack tops so that cleanup can restore it properly.
       */
       curtop = mytop;
       ecurtop = emytop;

      /*
       | put this expression on the head of the list of expressions being compiled that
       | threads through the cons cells 'cc' in the stack frame.
       */
       cc.celltype = CONSCELL;
       cc.carp = l;
       cc.cdrp = curr_expr;
       curr_expr = &cc;

      /*
       | Check for a user interrupt. Log location interrupt occured just like any other error since
       | it is useful for debugging asynchronous SIGINT problems.
       */
       if (bkhitcount > 0) { breaks_while_compiling += 1; bkhitcount = 0; cerror(INTERRUPT, l); }

      /*
       | Try to fold constant top level expression. This will call (eval) if
       | possible.
       */
       l = bu_fold_constant_expr(l);
       xpush(l);                              /* no need for xpop(1) later I use curtop! */

      /*
       | Now actually compile the expression.
       */
       if (l == NULL) {
           EMIT1ATM("PUSHNIL"); EMITEND();
       } else if (l == LIST(thold)) {
           EMIT1ATM("PUSHT");    EMITEND();
       } else  {
           switch(l->celltype) {
               case ALPHAATOM :
                    EMIT1ATM("PUSHLV");
                    bu_compile_atom(l);
                    EMITEND();
                    break;
               case CONSCELL :
                    bu_compile_list(l);
                    break;
               default :
                    bu_compile_literal(l);
                    break;
           }
       }

      /*
       | Return but first restore the current global compile_expr jump buffer.
       | and unwind the cc static current expression backward threaded list.
       | Also restore stack top, don't need an xpop because we assign direclty.
       */
cleanup:
       mytop = curtop;
       emytop = ecurtop;
       compile_expr_jmp_buf = save_jmp_buf;
       curr_expr = curr_expr->cdrp;
       return;
}

/*
 | Compile a list of the form (func args .....) we split into two cases here
 | the first case handles functions that are atoms and the second case handles
 | functions that are expressions eg ( (lambda (a b c) ...) 1 2 3). We handle
 | the special case of a macro by first expanding the macro and then compiling
 | the resulting expression.
 */
static void bu_compile_list(l)
       struct conscell *l;
{
       struct conscell *func;

      /*
       | Make sure this list expression makes sense and catch (t ...) (nil ..) forms
       | which often come up when parenthesis nesting is wrong in (cond ...) forms.
       */
       if (l == NULL) return;
       func = l->carp;
       if (func == NULL) goto er1;
       if (func == LIST(thold)) goto er2;

      /*
       | Compile depending on type.
       */
       switch(func->celltype) {
           case ALPHAATOM :
                if (ALPHA(func)->fntype == FN_USMACRO) {
                    struct conscell *temp; push(temp);
                    temp = macroexpand(l);
                    bu_compile_expr(temp);
                    xpop(1);
                } else
                    if (ALPHA(func) == lambdahold)
                       bu_compile_lambda_body(l->cdrp);
                    else
                       if (ALPHA(func) == nlambdahold)
                          bu_compile_nlambda_body(l->cdrp);
                       else if (ALPHA(func) == lexprhold)
                                bu_compile_lexpr_body(l->cdrp);
                            else
                                bu_compile_func_args(func, l->cdrp, l->linenum, l);
                break;
           case CONSCELL:
                bu_compile_func_list(func, l->cdrp);
                break;
       }
       return;
er1:   cerror("(nil ..): nil is not a function", l);
       return;
er2:   cerror("(t ..): t is not a function", l);
       return;
}

/*
 | Compile a (func args) call where func is an atom. We handle the case of
 | a built in function here. We split into two cases, the first is the case
 | where the arguments are to be evaluated in which case we recursively
 | compile the arguments first then generate the operation, the  2nd
 | case is the compile of non evaluated functions like foreach, prog setq
 | etc which are handled individually. Note that if the function is equal to
 | the special rcall_atom (ie the optional parameter passed to the (compile)
 | function then this call is recursive and the (RCALL <n>) instruction is
 | generated. When we actually generate a generic CALL instruction we declare
 | the function as being of pseudo type N where N is the number of arguments.
 | Type checking will then be performed where possible on the type and number
 | of arguments. When a call is made to type nlambda the arguments become one
 | big literal which is pushed and a call is made with one argument.
 */
static void bu_compile_func_args(func, args, linenum, funcargs)
       struct alphacell *func;
       struct conscell *args;
       int linenum;
       struct conscell *funcargs;
{
       fptr fa = bu_lookup_compile_func(func->atom);
       char *call = "CALL";
       if (fa != NULL)
           (*fa)(args, func);
       else if (iscadar(func->atom))
           bu_compile_cadr(func->atom, args);
       else {
           long int n = liulength(args);
           struct conscell *type = bu_declare(func, LIST(newintop(n)), linenum);
           if (type == LIST(nlambdahold)) {
               struct conscell *arg;
               call = "CALLNF";                                  /* use non freeing call for NLAMBDA */
               for(arg = args; arg != NULL; arg = arg->cdrp) {
                   if (arg->celltype != CONSCELL) goto er1;
                   if (arg->carp == NULL) {
                       EMIT1ATM("PUSHNIL"); EMITEND();
                   } else if (arg->carp == LIST(thold)) {
                       EMIT1ATM("PUSHT"); EMITEND();
                   } else {
                       EMIT1ATM("PUSHL");
                       EMIT1FIX((long)bu_litref(arg->carp));
                       EMITEND();
                   }
               }
           } else {
               if ((type == LIST(macrohold)) && (ALPHA(func)->fntype != FN_USMACRO)) {  /* if is macro but don't have a body */
                    EMIT1ATM("PUSHL");                                                  /* expansion must be done at run time */
                    EMIT1FIX((long) bu_litref(funcargs));                               /* so generate (apply 'macro '(macro arg ..) */
                    EMITEND();
                    EMIT1ATM(call);
                    EMIT1FIX((long)1);
                    EMIT1FIX((long)bu_litref(LIST(CreateInternedAtom("eval"))));
                    EMITEND();
                    return;
               } else
                   bu_compile_arg_list(args);
           }
           if ((ALPHA(func) != rcall_atom) || (type == LIST(nlambdahold))) {
               EMIT1ATM(call);
               EMIT1FIX((long)n);
               EMIT1FIX((long)bu_litref(func));
           } else {
               EMIT1ATM("RCALL");             /* rcall faster for lambda but not allwed for nlambda */
               EMIT1FIX((long)n);             /* because it free's its arguments */
           }
           EMITEND();
       }
       return;
er1:   cerror("dotted pair not allowed as arg to nlambda", args);
       return;
}

/*
 | Compile a literal object ie a string or list such as '(a (((b c d)))) or "xy"
 | because the compiled code does not build these, they are build when the code
 | is loaded we simply generate a reference to a literal using a PUSHL to push
 | a literal on the stack. Each unique literal is referenced by a number so we
 | generate PUSHL <n> where <n> is the literals reference.
 */
static void bu_compile_literal(lit)
       struct conscell *lit;
{
       long litref;
       if (!lit) goto er;
       litref = bu_litref(lit);
       EMIT1ATM("PUSHL");
       EMIT1FIX(litref);
       EMITEND();
       return;
er:    cerror("literal: a literal is required but not found", lit);
}

/*
 | Compile an function calls argument list:
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | ( arg1 arg2 arg3 ... )                       <compile arg1 expr>
 |                                              <compile arg2 expr>
 |                                                      :
 | Compiling an argument list to a function that evaluates its arguments is
 | simply a matter of compiling each argument as an expression and allowing
 | the resulting S-expression to remain on the stack, this results in a stack
 | with N arguments pushed on it. This function will return the number of
 | arguments on the stack so that the caller can use it for reference.
 */
static int bu_compile_arg_list(l)
       struct conscell *l;
{
       int n = 0;
       while(l != NULL) {
           n += 1;
           if (l->celltype != CONSCELL) break;
           bu_compile_expr(l->carp);
           l = l->cdrp;
       }
       return(n);
}

/*
 | Compile a list of bodies without labels
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | ( (setq x 10)                                  <setq1>
 |   (setq y 20)                                   POP
 |     :                                          <setq2>
 |     :                                           POP
 |   (setq z 10)                                  <setq3>
 | )
 |
 | This list of bodies is just like compiling a list of expression arguments
 | except that the result of each is to be thrown away except the last one.
 | To accomplish this we pop the stack after each body but the last. Labels
 | are not processed in the bodies and when a simple atom will compile to a
 | push of its value.
 */
static void bu_compile_bodies(l)
       struct conscell *l;
{
       if (l == NULL) { EMIT1ATM("PUSHNIL"); EMITEND(); }
       while(l) {
           if (l->celltype != CONSCELL) goto er;
           bu_compile_expr(l->carp);
           if (l = l->cdrp) { EMIT1ATM("POP"); EMITEND(); }
       }
       return;
er:    cerror("list of statements: may not contain a dotted pair", l);
}

/*
 | Compile a list of bodies with labels:
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | ( (setq x 10)                                  <setq1>
 |   (setq y 20)                                   POP
 | top :                                          <setq2>
 |     :                                            POP
 |     :                                    Utop  { rest ... }
 |   (goto top)                                   { no POP after last body }
 |     :                                            JUMP Utop
 | )
 |
 | This list of bodies is just like compiling a list of expression arguments
 | except that the result of each is to be thrown away except the last one.
 | To accomplish this we pop the stack after each body but the last. We first
 | scan the list of bodies for labels and when we find them we bind them to
 | the current recursive level (scope and target label). Then when a (go) is
 | processed it will check the binding of the label which must be the proper
 | scope level or the go is not allowed, ie we do not allow (go)'s outside or
 | into another list of bodies.
 */
static void bu_compile_prog_bodies(bods)
       struct conscell *bods;
{
       struct conscell *e,*l,*scope; long label;
       push(scope);
       gotoScope += 1;
       if (bods == NULL) { EMIT1ATM("PUSHNIL"); EMITEND(); }
       for(l = bods; l != NULL; l = l->cdrp) {
           if (l->celltype != CONSCELL) goto er;
           if (!(e = l->carp)) continue;
           if (e->celltype == ALPHAATOM) {
               label = NEWLABEL()
               scope = newintop((long)((((long)gotoScope)<<16) | label));
               bindvar(e, scope);
           }
       }
       for(l = bods; l != NULL; l = l->cdrp) {
           if ((e = l->carp) && (e->celltype == ALPHAATOM)) {
               if (GetFix(ALPHA(e)->valstack->carp,&label))
                   EMITLABEL(label & 0xffff);
           } else {
               bu_compile_expr(e);
               if (l->cdrp) { EMIT1ATM("POP"); EMITEND(); }
           }
       }
       for(l = bods; l != NULL; l = l->cdrp) {
           if (!(e = l->carp)) continue;
           if (e->celltype == ALPHAATOM)
               unbindvar(e);
       }
       gotoScope -= 1;
       xpop(1);
       return;
er:    cerror("prog: bodies may not contain dotted pair", bods);
}

/*
 | Compile the "cond" expression code in and code out are as follows:
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | (cond  ( expr1 body1/1 body1/2 ... )           <expr1>
 |        ( expr2 body2/1 body2/2 ... )           JNIL L<expr1>
 |        :                                       <body1/1> <body1/2> ...
 | )                                              JUMP L<cond>
 |                                          L<expr1>
 |                                                <expr2>
 |                                                JNIL L<expr2>
 |                                                <body2/1> <body2/2> ...
 |                                                JUMP L<cond>
 |                                          L<expr2>
 |						  PUSHNIL       ; no cond matched return a nil
 |                                          L<cond>
 |
 | Special case when no body is present for a case, we duplicate the expr
 | value on the stack before testing and POP it if test fails, otherwise the
 | jump to exit causes the top of stack to be returned.
 |
 | (cond  ( expr1  )                              <expr1>
 |           ...                                  DUP
 |        )                                       JNIL L<expr1>
 | )                                              JUMP L<cond>
 |                                          L<expr1>
 |                                                POP
 |                                                ...
 */
static void bu_compile_cond(args)
       struct conscell *args;
{
       int special, elt_exit, cond_exit = NEWLABEL()
       struct conscell *elt;
       while(args != NULL) {
           if (args->celltype != CONSCELL) goto er1;
           elt = args->carp;
           if (!elt) goto er2;
           if (elt->celltype != CONSCELL) goto er3;
           bu_compile_expr(elt->carp);                          /* code to compute guard value */
           if (special = (elt->cdrp == NULL)) {                 /* if no body must DUP expr value as return value */
               EMIT1ATM("DUP"); EMITEND();                      /* of cond if value is non nil */
           }
           elt_exit = NEWLABEL()
           EMIT1ATM("JNIL"); EMIT1LAB(elt_exit); EMITEND();     /* test guard skip body if nil */
           if (!special) bu_compile_bodies(elt->cdrp);          /* if non special generate code to compute bodies */
           args = args->cdrp;
           EMIT1ATM("JUMP"); EMIT1LAB(cond_exit); EMITEND();    /* then after bodies evaled skip to cond end */
           EMITLABEL(elt_exit);
           if (special) { EMIT1ATM("POP"); EMITEND(); }
       }
       EMIT1ATM("PUSHNIL"); EMITEND();                          /* no cond matched case returns a nil */
       EMITLABEL(cond_exit);
       return;
er1:   cerror("(cond <case> '.' <case> ): dotted pair in arg list not permitted", args);
       return;
er2:   cerror("(cond ... nil ...): nil case not permitted", args);
       return;
er3:   cerror("(cond ... ?? ...): non list case not permitted", args);
       return;
}

/*
 | Compile an disembodied lambda expression like ( (lambda (a b c) ... ) 10 20 30)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | ( (lambda (x y) (+ x y))                       SPUSHNIL x
 |   10                                           SPUSHNIL y
 |   20                                           PUSH 10
 | )                                              SETQ x
 |                                                POP
 |                                                PUSH 20
 |                                                SETQ y
 |                                                POP
 |                                                PUSH x
 |                                                PUSH y
 |                                                ADD
 |                                                SPOP x
 |                                                SPOP y
 |
 */
static void bu_compile_disembodied_lambda(func, args)
       struct conscell *func, *args;
{
       int nfargs, naargs;
       struct conscell *s;
       return_push(0);                          /* inhibit (return's) */
       func = func->cdrp;
       if (func == NULL) goto er1;
       nfargs = liulength(args);
       naargs = liulength(func->carp);
       if (naargs != nfargs) goto er2;
       if (func->carp) emitZPUSH();
       for(s = func->carp; s != NULL; s = s->cdrp) {
           if ((s->carp == NULL) || (s->carp == LIST(thold)) || (s->carp->celltype != ALPHAATOM)) goto er3;
           EMIT1ATM("SPUSHNIL");
           bu_compile_atom(s->carp);
           EMITEND();
       }
       for(s = func->carp; s != NULL; s = s->cdrp) {
           bu_compile_expr(args->carp);
           EMIT1ATM("SETQ");
           bu_compile_atom(s->carp);
           EMITEND();
           EMIT1ATM("POP"); EMITEND();
           args = args->cdrp;
       }
       bu_compile_bodies(func->cdrp);
       if (func->carp) { EMIT1ATM("ZPOP"); EMITEND(); }
       for(s = func->carp; s != NULL; s = s->cdrp) {
           EMIT1ATM("SPOP");
           bu_compile_atom(s->carp);
           EMITEND();
       }
       return_pop();
       return;
er1:   cerror("( (lambda(f1 f2 ..) ) arg1 arg2 ..): badly formed lambda being applied", args);
       return;
er2:   cerror("( (lambda(f1 f2 ..) ) arg1 arg2 ..): incorrect number of args provided", args);
       return;
er3:   cerror("( (lambda(nil/t/etc.) ) arg1 arg2 ..): invalid formal argument", s);
       return;
}

/*
 | Compile a disembodied nlambda expression like ( (nlambda(l) <bod>..) <expr> <expr> ... )
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | ( (nlambda (l) (length l))                     PUSHL (+ b c)     ; arguments are not compiled!
 |   (+ b c)                                      PUSHL (* d e)     ; ditto
 |   (* d e)                                      CALL <2>, <list>
 | )                                              ZPUSH
 |                                                SPUSHNIL <l>
 |                                                SETQ <l>
 |                                                POP
 |                                                PUSHLV <l>
 |                                                CALL <1>, <length>
 |                                                ZPOP
 |                                                SPOP <l>
 */
static void bu_compile_disembodied_nlambda(func, args)
       struct conscell *func, *args;
{
       int naargs = 0;
       struct conscell *work;
       return_push(0);                                /* inhibit (return's) */
       func = func->cdrp;
       if (func == NULL) goto er1;
       if (liulength(func->carp) != 1) goto er2;
       for(work = args; work != NULL; work = work->cdrp) {
           if (work->carp == NULL) {
               EMIT1ATM("PUSHNIL"); EMITEND();
           } else if (work->carp == LIST(thold)) {
               EMIT1ATM("PUSHT");   EMITEND();
           } else
               bu_compile_literal(work->carp);
           naargs += 1;
       }
       EMIT1ATM("CALL"); EMIT1FIX(naargs); bu_compile_atom(LIST(CreateInternedAtom("list"))); EMITEND();
       work = func->carp;
       if ((work == NULL) || (work->celltype != CONSCELL)) goto er3;
       work = work->carp;
       if ((work == NULL) || (work == LIST(thold)) || (work->celltype != ALPHAATOM)) goto er3;
       emitZPUSH();
       EMIT1ATM("SPUSHNIL");
       bu_compile_atom(work);
       EMITEND();
       EMIT1ATM("SETQ");
       bu_compile_atom(work);
       EMITEND();
       EMIT1ATM("POP"); EMITEND();
       bu_compile_bodies(func->cdrp);
       EMIT1ATM("ZPOP"); EMITEND();
       EMIT1ATM("SPOP");
       bu_compile_atom(work);
       EMITEND();
       return_pop();
       return;
er1:   cerror("( (nlambda) arg1 arg2 ..): badly formed nlambda being applied", func);
       return;
er2:   cerror("( (nlambda(l .. ) ) arg1 arg2 ..): nlambda with more than/less than 1 formal argument", func);
       return;
er3:   cerror("( (nlambda(nil/t/etc.) ) arg1 arg2 ..): invalid formal argument", work);
       return;
}

/*
 | Compile a disembodied lexpr expression like ( (lexpr(N) <bod>..) <expr> <expr> ... )
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | ( (lexpr(n) <bodies> ...)                      PUSHL <clisp-of-lexpr>
 |   <arg1>                                       <arg1>
 |   <arg2>                                       <arg2>
 |   ...                                          ...
 |   <argN>                                       <argN>
 | )                                              CALL <N>,<list>
 |                                                CALL <2>,<apply>
 |
 | Unlike the other disembodied types an lexpr is difficult to compile inline because
 | of the (arg/listify..etc.) functions that must work in the context. We compile this
 | unusual form into an (apply <clisp> (arg1 ... argN)) and simply call the bucompile
 | function recursively to produce the <clisp> literal for the lexpr form.
 */
static void bu_compile_disembodied_lexpr(func, args)
       struct conscell *func, *args;
{
       int naargs; struct conscell c1, *clisp;
       if (func->cdrp == NULL) goto er1;
       c1.celltype = CONSCELL;
       c1.carp = func;
       c1.cdrp = NULL;
       clisp = bucompile(&c1);
       bu_hoist_errors(clisp);                  /* remove errors from lower level and append to errlist */
       bu_compile_literal(clisp);               /* generate the PUSHL <lexpr-form-clisp-form> */
       naargs = bu_compile_arg_list(args);
       EMIT1ATM("CALL"); EMIT1FIX(naargs); bu_compile_atom(LIST(CreateInternedAtom("list"))); EMITEND();
       EMIT1ATM("CALL"); EMIT1FIX(2); bu_compile_atom(LIST(CreateInternedAtom("apply"))); EMITEND();
       return;
er1:   cerror("( (lexpr) arg1 arg2 ..): badly formed lexpr being applied", func);
       return;
}

/*
 | Compile an disembodied function call expression like ( <list> arguments )
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | i.e. ( (lambda (x y) (+ x y))  10 20)
 |      ( (nlambda(l) ...) )
 */
static void bu_compile_func_list(func, args)
       struct conscell *func, *args;
{
       struct alphacell *at = ALPHA(func->carp);
       if (at == lambdahold)
           bu_compile_disembodied_lambda(func, args);
       else if (at == nlambdahold)
           bu_compile_disembodied_nlambda(func, args);
       else if (at == lexprhold)
           bu_compile_disembodied_lexpr(func, args);
       else
           goto er;
       return;
er:    cerror("not a valid disembodied function type", func);
       return;
}

/*
 | Compile an expression like (lambda (a b c) ... )
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 |
 | (lambda (x y z)
 |     .. body ..)                                SPUSHWARG <lit>
 | )                                              SPUSHWARG <lit>
 |                                                SPUSHWARG <lit>
 |                                                .. body ..
 |                                                SPOP <lit>
 |                                                SPOP <lit>
 |                                                SPOP <lit>
 */
static void bu_compile_lambda_body(args)
       struct conscell *args;
{
       struct conscell *l = args;
       if (l == NULL) goto er;
       if (args->carp) emitZPUSH();
       for(l = args->carp; l != NULL; l = l->cdrp) {
           if (l->celltype != CONSCELL) goto er;
           if (l->carp == NULL) goto er;
           if (l->carp->celltype != ALPHAATOM) goto er;
           if (l->carp == LIST(thold)) goto er2;
           EMIT1ATM("SPUSHWARG");
           bu_compile_atom(l->carp);
           EMITEND();
       }
       bu_compile_bodies(args->cdrp);
       if (args->carp) { EMIT1ATM("ZPOP"); EMITEND(); }
       for(l = args->carp; l != NULL; l = l->cdrp) {
           EMIT1ATM("SPOP");
           bu_compile_atom(l->carp);
           EMITEND();
       }
       return;
er:    cerror("(lambda()..): bad formal argument; must be atom", l);
       return;
er2:   cerror("(lambda()..): t cannot be rebound by using as formal argument", l);
       return;
}

/*
 | Compile an expression like (nlambda (l) ... )
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 |
 | (nlambda (l)
 |     .. body ..                                 SPUSHARGS <lit>
 | )                                              .. body ..
 |                                                SPOP <lit>
 */
static void bu_compile_nlambda_body(args)
       struct conscell *args;
{
       struct conscell *l = args;
       if (l == NULL) goto er;
       l = l->carp;
       if (l) {
           if (l->carp == NULL) goto er;
           if (l->cdrp != NULL) goto er;
           if (l->carp->celltype != ALPHAATOM) goto er;
           if (l->carp == LIST(thold)) goto er2;
           emitZPUSH();
           EMIT1ATM("SPUSHARGS");
           bu_compile_atom(l->carp);
           EMITEND();
       }
       bu_compile_bodies(args->cdrp);
       if (l) {
           EMIT1ATM("ZPOP"); EMITEND();
           EMIT1ATM("SPOP");
           bu_compile_atom(l->carp);
           EMITEND();
       }
       return;
er:    cerror("(nlambda()..): bad formal argument; must be atom", l);
       return;
er2:   cerror("(nlambda()..): t cannot be rebound by using as formal argument", l);
       return;
}

/*
 | Compile an expression like (lexpr (_N_) <body> ...)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 |
 |  (lexpr(_N_)                                   SPUSHLEX  _N_    ; bind the arg count argument
 |     <body> ...                                 <body> ...       ; can use SPUSHWARG now for (arg N) etc.
 |                                                SPOP      _N_    ; pop the binding of _N_ and return top of stack
 |  )
 */
static void bu_compile_lexpr_body(args)
       struct conscell *args;
{
       struct conscell *l = args;
       if (l == NULL) goto er;
       l = l->carp;
       if (l) {
           if (l->carp == NULL) goto er;
           if (l->cdrp != NULL) goto er;
           if (l->carp->celltype != ALPHAATOM) goto er;
           if (l->carp == LIST(thold)) goto er2;
           emitZPUSH();
           EMIT1ATM("SPUSHLEX");
           bu_compile_atom(l->carp);
           EMITEND();
       }
       bu_compile_bodies(args->cdrp);
       if (l) {
           EMIT1ATM("ZPOP"); EMITEND();
           EMIT1ATM("SPOP");
           bu_compile_atom(l->carp);
           EMITEND();
       }
       return;
er:    cerror("(lexpr()..): bad formal argument; must be atom", l);
       return;
er2:   cerror("(lexpr()..): t cannot be rebound by using as formal argument", l);
       return;
}

/*
 | Compile an expression like (setq a <expr> b <expr> ...)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | (setq a 10 b 20)                               PUSHFIX 10
 |                                                SETQ a
 |                                                POP
 |                                                PUSHFIX 20
 |                                                SETQ b
 */
static void bu_compile_setq(args)
       struct conscell *args;
{
       struct conscell *var;
       if (args == NULL) goto er1;
       while(args != NULL) {
            var = args->carp;
            if (var == NULL) goto er4;
            if (var->celltype != ALPHAATOM) goto er2;
            if (var == LIST(thold)) goto er3;
            args = args->cdrp;
            if (args == NULL) goto er1;
            bu_compile_expr(args->carp);
            EMIT1ATM("SETQ");
            bu_compile_atom(var);
            EMITEND();
            if ((args = args->cdrp) != NULL) {
                EMIT1ATM("POP"); EMITEND();
            }
       }
       return;
er1:   cerror("setq: wrong # of args", args);
       return;
er2:   cerror("setq: target of assignment not an atom", var);
       return;
er3:   cerror("setq: t may not be rebound", args);
       return;
er4:   cerror("setq: nil may not be rebound", args);
       return;
}

/*
 | Compile an expression like (foreach a '(a b c) ....)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 |                            PUSHNIL           ; return value for foreach
 | (foreach a '(a b c)        SPUSHNIL a        ; new scope for variable
 |   ...                      PUSHL '(a b c)    ; push list to iterate through
 | )                 L1:      DUP               ; top of loop, duplicate remaining list
 |                            JNIL L2:          ; if nothing left jump to exit
 |                            DUP               ; dup again so as to leave lower copy
 |                            CAR               ; get first element and assign
 |                            SETQ a            ; to loop index variable
 |                            POP               ; pop setq value exposing remaining list
 |                            CDR               ; replace with cdr of list
 |                            .............     ; do bodies
 |                            STORR -2          ; last body return value stored in return spot
 |                            JUMP L1:          ; back for more
 |                   L3:      STORR -2          ; return target if there is one
 |                   L2:      POP               ; pop the null list on stack
 |                            SPOP a            ; unwind scope of a and exit with return value
 */
static void bu_compile_foreach(args)
       struct conscell *args;
{
       struct conscell *lvar;
       int elt_top  = NEWLABEL()
       int elt_exit = NEWLABEL()
       int elt_retn = NEWLABEL()
       return_push(elt_retn);
       if (args == NULL) goto er1;
       lvar = args->carp;
       if (lvar == NULL) goto er2;
       if (lvar->celltype != ALPHAATOM) goto er3;
       if (lvar == LIST(thold)) goto er4;
       EMIT1ATM("PUSHNIL"); EMITEND();
       emitZPUSH();
       EMIT1ATM("SPUSHNIL"); bu_compile_atom(lvar); EMITEND();
       args = args->cdrp;
       if (args == NULL) goto er5;
       bu_compile_expr(args->carp);
       EMITLABEL(elt_top);
       EMIT1ATM("DUP"); EMITEND();
       EMIT1ATM("JNIL"); EMIT1LAB(elt_exit); EMITEND();
       EMIT1ATM("DUP"); EMITEND();
       EMIT1ATM("CAR"); EMITEND();
       EMIT1ATM("SETQ"); EMIT1FIX((long)bu_litref(lvar)); EMITEND();
       EMIT1ATM("POP"); EMITEND();
       EMIT1ATM("CDR"); EMITEND();
       if (args = args->cdrp) {
           bu_compile_prog_bodies(args);
           EMIT1ATM("STORR"); EMIT1FIX(-2L); EMITEND();
       }
       EMIT1ATM("JUMP"); EMIT1LAB(elt_top); EMITEND();
       if (return_called()) {
           EMITLABEL(elt_retn);
           EMIT1ATM("STORR"); EMIT1FIX(-2L); EMITEND();
       }
       EMITLABEL(elt_exit);
       EMIT1ATM("POP"); EMITEND();
       EMIT1ATM("ZPOP"); EMITEND();
       EMIT1ATM("SPOP"); EMIT1FIX((long)bu_litref(lvar)); EMITEND();
       return_pop();
       return;
er1:   cerror("foreach: wrong # of args", args);
       return;
er2:   cerror("foreach: nil cannot be rebound", args);
       return;
er3:   cerror("foreach: requires an atom as first argument", args);
       return;
er4:   cerror("foreach: t may not be rebound", lvar);
       return;
er5:   cerror("foreach: badly structured, requires a 2nd argument", args);
       return;
}

/*
 | Compile an expression like (while (< i 20) (setq i (1+ i)) ...)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 |                            PUSHNIL   ; the while's return value is nil so far
 | (while <expr>     L1:      <expr>    ; top of loop test expression
 |   ....                     JNIL l2:  ; if nil get out with tos == while return value
 | )                          ...       ; bodies ..
 |                            ...
 |                            STORR -1  ; store last body return value for while return
 |                            JUMP L1:  ; back to top of loop.
 |                   L3:      STORR -1  ; return target if return called in while
 |                   L2:
 |
 */
static void bu_compile_while(args)
       struct conscell *args;
{
       int elt_top  = NEWLABEL()
       int elt_exit = NEWLABEL()
       int elt_retn = NEWLABEL()
       return_push(elt_retn);
       if (args == NULL) goto er;
       EMIT1ATM("PUSHNIL"); EMITEND();
       EMITLABEL(elt_top);
       bu_compile_expr(args->carp);
       EMIT1ATM("JNIL"); EMIT1LAB(elt_exit); EMITEND();
       if (args = args->cdrp) {
           bu_compile_prog_bodies(args);
           EMIT1ATM("STORR"); EMIT1FIX(-1L); EMITEND();
       }
       EMIT1ATM("JUMP"); EMIT1LAB(elt_top); EMITEND();
       if (return_called()) {
           EMITLABEL(elt_retn);
           EMIT1ATM("STORR"); EMIT1FIX(-1L); EMITEND();
       }
       EMITLABEL(elt_exit);
       return_pop();
       return;
er:    cerror("while: wrong # of args", args);
       return;
}

/*
 | Compile an expression like   (prog (a b c) ...................)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | (prog (a b)                                    SPUSHNIL a
 |   ....                                         SPUSHNIL b
 | )                                              ...
 |                                                ...
 |                                       L1:      SPOP a
 |                                                SPOP b
 |
 */
static void bu_compile_prog(args)
       struct conscell *args;
{
       int elt_retn = NEWLABEL()
       struct conscell *s;
       return_push(elt_retn);
       if (args == NULL) goto er1;
       if (args->carp) emitZPUSH();
       for(s = args->carp; s != NULL; s = s->cdrp) {
           if (s->celltype != CONSCELL) goto er5;
           if (s->carp == NULL) goto er2;
           if (s->carp->celltype != ALPHAATOM) goto er3;
           if (s->carp == LIST(thold)) goto er4;
           EMIT1ATM("SPUSHNIL");
           bu_compile_atom(s->carp);
           EMITEND();
       }
       if (args->cdrp)
           bu_compile_prog_bodies(args->cdrp);
       else {
           EMIT1ATM("PUSHNIL");
           EMITEND();
       }
       EMITLABEL(elt_retn);
       if (args->carp) { EMIT1ATM("ZPOP"); EMITEND(); }
       for(s = args->carp; s != NULL; s = s->cdrp) {
           EMIT1ATM("SPOP");
           bu_compile_atom(s->carp);
           EMITEND();
       }
       return_pop();
       return;
er1:   cerror("prog: wrong # of args", args);
       return;
er2:   cerror("prog: nil not allowed as a prog local", args->carp);
       return;
er3:   cerror("prog: only atoms are legal prog locals", args->carp);
       return;
er4:   cerror("prog: t not allowed as a prog local", args->carp);
       return;
er5:   cerror("prog: second argument must be a list of atoms", s);
       return;
}

/*
 | Compile an expression like   ($file-prog$ <path> body body ...)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | Since returns ang go's are not legal in a file-prog just compile
 | the bodies and POP the results as we go except for the last one.
 | A file prog is used as the top level wrapper around all statements
 | in a file and is only there so that the error handling mechanisms
 | of liszt.l can figure out which file the source came from.
 */
static void bu_compile_file_prog(args)
       struct conscell *args;
{
       if (args == NULL) goto er1;
       args = args->cdrp;
       bu_compile_bodies(args);
       return;
er1:   cerror("$file-prog$: wrong # of args", args);
       return;
}

/*
 | Compile an expression like (repeat <expr> .........)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 |                            PUSHNIL           ; push repeat's return value
 | (repeat 10                 PUSHL 10          ; generated <expr> code
 |     ......                 COPYFIX           ; replace with new copy of cell
 |     ......        L1:      DUP               ; and duplicate top so that
 | )                          JZERO L2:         ; test leaves once copy
 |                            .............
 |                            STORR -2          ; last body value stored for return
 |                            DDEC              ; destructive decrement of loop count
 |                            JUMP L1:          ; back to top.
 |                   L3:      STORR -2          ; return target if called
 |                   L2:      POP               ; pop the loop count exposing return value
 */
static void bu_compile_repeat(args)
       struct conscell *args;
{
       int elt_top  = NEWLABEL()
       int elt_exit = NEWLABEL()
       int elt_retn = NEWLABEL()
       if (args == NULL) goto er1;
       return_push(elt_retn);                   /* where to go on return */
       EMIT1ATM("PUSHNIL"); EMITEND();
       bu_compile_expr(args->carp);
       EMIT1ATM("COPYFIX"); EMITEND();
       EMITLABEL(elt_top);
       EMIT1ATM("DUP"); EMITEND();
       EMIT1ATM("JZERO"); EMIT1LAB(elt_exit); EMITEND();
       if (args->cdrp) {
           bu_compile_prog_bodies(args->cdrp);
           EMIT1ATM("STORR"); EMIT1FIX(-2L); EMITEND();
       }
       EMIT1ATM("DDEC"); EMITEND();
       EMIT1ATM("JUMP"); EMIT1LAB(elt_top); EMITEND();
       if (return_called()) {
           EMITLABEL(elt_retn);
           EMIT1ATM("STORR"); EMIT1FIX(-2L); EMITEND();
       }
       EMITLABEL(elt_exit);
       EMIT1ATM("POP"); EMITEND();
       return_pop();
       return;
er1:   cerror("repeat: wrong # of args", args);
       return;
}

/*
 | Compile the values in an expression like (PAR-setq a <expr> b <expr> ...)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | (PAR-setq a 10 b 20 c 30)        PUSHFIX 30
 |                                  PUSHFIX 20
 |                                  PUSHFIX 10
 */
static void bu_compile_parsetq_values(args)
       struct conscell *args;
{
       if (args == NULL) return;
       if ((args = args->cdrp) == NULL) return;
       bu_compile_parsetq_values(args->cdrp);
       bu_compile_expr(args->carp);
}

/*
 | Compile an expression like (PAR-setq a <expr> b <expr> ...)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | (PAR-setq a 10 b 20 c 30)        PUSHFIX 30
 |                                  PUSHFIX 20
 |                                  PUSHFIX 10
 |                                  SETQ a
 |                                  POP
 |                                  SETQ b
 |                                  POP
 |                                  SETQ c
 */
static void bu_compile_parsetq(args)
       struct conscell *args;
{
       struct conscell *var;

      /*
       | PAR-setq must have even number of arguments.
       */
       if (liulength(args) % 2 != 0) goto er1;

      /*
       | Recursively generate the parsetq value expressions in reverse order.
       | Using a recursive function to create the reverse output code.
       */
       bu_compile_parsetq_values(args);

      /*
       | Now generate the setq's in the order they occur in the input expression.
       | Only the last one is not popped forming the return value for the PAR-setq.
       */
       while(args != NULL) {
            var = args->carp;
            args = args->cdrp;
            if (args == NULL) goto er1;
            if (var == NULL) goto er2;
            if (var->celltype != ALPHAATOM) goto er3;
            if (var == LIST(thold)) goto er4;
            EMIT1ATM("SETQ");
            bu_compile_atom(var);
            EMITEND();
            if ((args = args->cdrp) != NULL) {
                EMIT1ATM("POP"); EMITEND();
            }
       }
       return;
er1:   cerror("PAR-setq/do: wrong # of args", args);
       return;
er2:   cerror("PAR-setq/do: nil may not be rebound", args);
       return;
er3:   cerror("PAR-setq/do: only atoms my be bound", args);
       return;
er4:   cerror("PAR-setq/do: t may not be rebound", args);
       return;
}

/*
 | Compile an expression like (and <expr> <expr> ............)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | (and <expr1> <expr2> <expr3>)    <expr1>
 |                                  JNIL L1:
 |                                  <expr2>
 |                                  JNIL L1:
 |                                  <expr3>
 |                                  JUMP L2:
 |                              L1: PUSHNIL
 |                              L2:
 */
static void bu_compile_and(args)
       struct conscell *args;
{
       struct conscell *cdrp;
       if (!args) {
           EMIT1ATM("PUSHT"); EMITEND();
       } else {
           int l1 = NEWLABEL()
           int l2 = NEWLABEL()
           while(args != NULL) {
                bu_compile_expr(args->carp);
                if (cdrp = args->cdrp) {
                    EMIT1ATM("JNIL"); EMIT1LAB(l1); EMITEND();
                    args = cdrp;
                } else {
                    EMIT1ATM("JUMP"); EMIT1LAB(l2); EMITEND();
                    EMITLABEL(l1);
                    EMIT1ATM("PUSHNIL"); EMITEND();
                    EMITLABEL(l2);
                    break;
                }
           }
       }
}

/*
 | Compile an expression like (or <expr> <expr> ............)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | (or)                             PUSHT
 |
 | (or <expr1> <expr2> <expr3>)     <expr1>
 |                                  DUP
 |                                  JNNIL L1:
 |                                  POP
 |                                  <expr2>
 |                                  DUP
 |                                  JNNIL L1:
 |                                  POP
 |                                  <expr3>
 |                                  DUP
 |                                  JNNIL l1:
 |                              L1:
 */
static void bu_compile_or(args)
       struct conscell *args;
{
       struct conscell *cdrp;
       if (!args) {
           EMIT1ATM("PUSHT"); EMITEND();
       } else {
           int l1 = NEWLABEL()
           while(args != NULL) {
                bu_compile_expr(args->carp);
                EMIT1ATM("DUP"); EMITEND();
                EMIT1ATM("JNNIL"); EMIT1LAB(l1); EMITEND();
                if (cdrp = args->cdrp) {
                    EMIT1ATM("POP"); EMITEND();
                    args = cdrp;
                } else {
                    EMITLABEL(l1);
                    break;
                }
           }
       }
}

/*
 | Compile an expression like (caseq <key>  ................)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | (caseq <key>                         <key>          ; push value of <key> expr
 |     ( <lit1>                         DUP            ; dup for first jeq
 |       <body1_1> ..... )              <lit1>         ; push <lit1> value
 |     ( (<lit2> <lit3>)                JEQ   L0       ; if equal do body1_1 ...
 |       <body2_1 .....  )              DUP            ; else dup <key> again for next
 |     ( t                              <lit2>
 |       <body3_1 ....   )              JEQ   L1
 | )                                    DUP
 |                                      <lit3>
 |                                      JEQ   L1
 |                                      JUMP  L2       ; and jump to default bodies
 |                                 L0   POP            ; pop the key is is redundant
 |					<body1_1> ...  ; these are the bodies one by one
 |                                      JUMP  L3
 |                                 L1   POP            ; pop the key it is redundant
 |					<body2_1> ...
 |                                      JUMP L3
 |                                 L2   POP            ; pop the key it is redundant
 |					<body3_1> ...  ; last body just drops into exit code
 |                                 L3   :
 |
 | NOTE: a special case occurs when one of the literals is NIL, in this case instead of
 | generating code that does <lit>, DUP, JEQ <label> we generate DUP, JNIL <label>
 | which is slightly more effecient.
 | NOTE: a special case occurs when there is no (t <body>) default case. When this occurs
 | we generate a default case of (t nil).
 */
static void bu_compile_caseq(aargs)
       struct conscell *aargs;
{
       int i, has_t_case, nbod, origLabel;
       struct conscell *lits, *next, *args = aargs;
       if (!args) goto er1;
       bu_compile_expr(args->carp);
       args = args->cdrp;
       has_t_case = 0;
       for(nbod = 0; args != NULL; args = args->cdrp) {
           if (args->celltype != CONSCELL) goto er4;
           next = args->carp; if (!next) goto er2;
           lits = next->carp;
           if (lits && (lits->celltype == CONSCELL)) {
               for( ; lits != NULL; lits = lits->cdrp ) {
                   if (lits->celltype != CONSCELL) goto er5;
                   EMIT1ATM("DUP"); EMITEND();
                   if (lits->carp != NULL) {
                       bu_compile_literal(lits->carp);
                       EMIT1ATM("JEQ");
                   } else
                       EMIT1ATM("JNIL");
                   EMIT1LAB(nextLabel+nbod); EMITEND();
               }
            } else {
               if (lits != LIST(thold)) {
                   EMIT1ATM("DUP"); EMITEND();
                   if (lits) {
                       bu_compile_literal(lits);
                       EMIT1ATM("JEQ");
                   } else
                       EMIT1ATM("JNIL");
                   EMIT1LAB(nextLabel+nbod); EMITEND();
               } else {
                   EMIT1ATM("JUMP"); EMIT1LAB(nextLabel+nbod); EMITEND();
                   if (args->cdrp != NULL) goto er3;
                   has_t_case = 1;
               }
           }
           nbod += 1;
       }
       origLabel = nextLabel;
       nextLabel += nbod + 1;
       if (!has_t_case) {
           EMIT1ATM("POP"); EMITEND();
           EMIT1ATM("PUSHNIL"); EMITEND();
           EMIT1ATM("JUMP"); EMIT1LAB(origLabel + nbod); EMITEND();
       }
       i = 0;
       for(args = aargs->cdrp; args != NULL; args = args->cdrp) {
           EMITLABEL(origLabel+i);
           EMIT1ATM("POP"); EMITEND();
           bu_compile_bodies(args->carp->cdrp);
           if (args->cdrp) { EMIT1ATM("JUMP"); EMIT1LAB(origLabel + nbod); EMITEND(); }
           i += 1;
       }
       EMITLABEL(origLabel + nbod);
       return;
er1:   cerror("caseq: wrong # of args", args);
       return;
er2:   cerror("caseq: badly formed; nil case encountered", args);
       return;
er3:   cerror("caseq: badly formed; (t ..) case not last", args);
       return;
er4:   cerror("caseq: dotted pair not allowed in arg list", args);
       return;
er5:   cerror("caseq: dotted pair not allowed in tag list", lits);
       return;
}

/*
 | Compile the c{a|d}+r expression the list is already on top of stack
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | (caddr <expr>)                   CDR
 |                                  CDR
 |                                  CAR
 */
static void bu_compile_cadr(name, args)
       char *name;
       struct conscell *args;
{      char *s; char msg[50];
       int n = strlen(name);
       if (bu_compile_arg_list(args) != 1) goto er;
       for(s = name + n - 2; *s != 'c'; s--) {
           if (*s == 'd') EMIT1ATM("CDR"); else EMIT1ATM("CAR");
           EMITEND();
       }
       return;
er:    if (n + 20 >= sizeof(msg)) name = "c{a|d}+r";
       sprintf(msg,"%s: wrong # of args", name);
       cerror(msg, args);
       return;
}

/*
 | Compile an expression like (goto <label>)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | (go <atom>)               JUMP <label>
 |
 | Note that the atom <label> is bound to an integer representing the
 | scope level at which it was bound it is an error to try to jump to
 | a label out of the current scope. The upper 16 bits of the binding
 | is the scope level and the lower 16 bits is the real label that
 | corresponds to the user label. See the compile_prog_bodies code for more
 | details.
 */
static void bu_compile_go(args)
       struct conscell *args;
{
       struct alphacell *label; long scope;
       if ((args == NULL) || (args->cdrp != NULL)) goto er1;
       label = ALPHA(args->carp);
       if ((label == NULL) || (label->celltype != ALPHAATOM)) goto er2;
       if (label->valstack == NULL) goto er3;
       if (!GetFix(label->valstack->carp, &scope)) goto er3;
       if ((scope>>16) != gotoScope) goto er3;
       EMIT1ATM("JUMP"); EMIT1LAB(scope & 0xffff); EMITEND();
       return;
er1:   cerror("go: wrong # of args", args);
       return;
er2:   cerror("go: target not an atom", label);
       return;
er3:   cerror("go: target out of scope of closest enclosing prog, while, foreach or repeat", label);
       return;
}

/*
 | Compile (car <expr>)
 | ~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_car(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 1) goto er;
       EMIT1ATM("CAR"); EMITEND();
       return;
er:    cerror("car: wrong # of args", args);
}

/*
 | Compile (cdr <expr>)
 | ~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_cdr(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 1) goto er;
       EMIT1ATM("CDR"); EMITEND();
       return;
er:    cerror("cdr: wrong # of args", args);
}

/*
 | Compile (cons <expr>)
 | ~~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_cons(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 2) goto er;
       EMIT1ATM("CONS"); EMITEND();
       return;
er:    cerror("cons: wrong # of args", args);
}

/*
 | Compile (eq <expr> <expr>)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_eq(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 2) goto er;
       EMIT1ATM("EQ"); EMITEND();
       return;
er:    cerror("eq: wrong # of args", args);
}

/*
 | Compile (zerop <expr>)
 | ~~~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_zerop(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 1) goto er;
       EMIT1ATM("ZEROP"); EMITEND();
       return;
er:    cerror("zerop: wrong # of args", args);
}

/*
 | Compile (1+ <expr>)
 | ~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_inc(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 1) goto er;
       EMIT1ATM("INC"); EMITEND();
       return;
er:    cerror("1+: wrong # of args", args);
}

/*
 | Compile (length <expr>)
 | ~~~~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_length(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 1) goto er;
       EMIT1ATM("LENGTH"); EMITEND();
       return;
er:    cerror("length: wrong # of args", args);
}

/*
 | Compile (1- <expr>)
 | ~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_dec(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 1) goto er;
       EMIT1ATM("DEC"); EMITEND();
       return;
er:    cerror("1-: wrong # of args", args);
}

/*
 | Compile (null <expr>)
 | ~~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_null(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 1) goto er;
       EMIT1ATM("NULL"); EMITEND();
       return;
er:    cerror("null: wrong # of args", args);
}

/*
 | Compile (not <expr>)
 | ~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_not(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 1) goto er;
       EMIT1ATM("NULL"); EMITEND();    /* NULL is equivalent to NOT */
       return;
er:    cerror("not: wrong # of args", args);
}

/*
 | Compile (listp <expr>)
 | ~~~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_listp(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 1) goto er;
       EMIT1ATM("LISTP"); EMITEND();
       return;
er:    cerror("listp: wrong # of args", args);
}

/*
 | Compile (fixp <expr>)
 | ~~~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_fixp(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 1) goto er;
       EMIT1ATM("FIXP"); EMITEND();
       return;
er:    cerror("fixp: wrong # of args", args);
}

/*
 | Compile (hunkp <expr>)
 | ~~~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_hunkp(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 1) goto er;
       EMIT1ATM("HUNKP"); EMITEND();
       return;
er:    cerror("hunkp: wrong # of args", args);
}

/*
 | Compile (atom <expr>)
 | ~~~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_atomp(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 1) goto er;
       EMIT1ATM("ATOM"); EMITEND();
       return;
er:    cerror("atom: wrong # of args", args);
}

/*
 | Compile (numbp <expr>)
 | ~~~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_numbp(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 1) goto er;
       EMIT1ATM("NUMBP"); EMITEND();
       return;
er:    cerror("numbp: wrong # of args", args);
}

/*
 | Compile (floatp <expr>)
 | ~~~~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_floatp(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 1) goto er;
       EMIT1ATM("FLOATP"); EMITEND();
       return;
er:    cerror("floatp: wrong # of args", args);
}

/*
 | Compile (stringp <expr>)
 | ~~~~~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_stringp(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 1) goto er;
       EMIT1ATM("STRINGP"); EMITEND();
       return;
er:    cerror("stringp: wrong # of args", args);
}

/*
 | Compile (arg <expr>)
 | ~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_arg(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 1) goto er;
       EMIT1ATM("ARG"); EMITEND();
       return;
er:    cerror("arg: wrong # of args", args);
}

/*
 | Compile (arg? <expr> <expr>)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_argQ(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 2) goto er;
       EMIT1ATM("ARG?"); EMITEND();
       return;
er:    cerror("arg?: wrong # of args", args);
}

/*
 | Compile (listify <expr>)
 | ~~~~~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_listify(args)
       struct conscell *args;
{
       int n = bu_compile_arg_list(args);
       if (n != 1) goto er;
       EMIT1ATM("LISTIFY"); EMITEND();
       return;
er:    cerror("listify: wrong # of args", args);
}

/*
 | Compile (quote <expr>)
 | ~~~~~~~~~~~~~~~~~~~~~~
 */
static void bu_compile_quote(args)
       struct conscell *args;
{
       if (!args || args->cdrp) goto er;
       if (args->carp == NULL) {
           EMIT1ATM("PUSHNIL"); EMITEND();
       } else if (args->carp == LIST(thold)) {
           EMIT1ATM("PUSHT"); EMITEND();
       } else
           bu_compile_literal(args->carp);
       return;
er:    cerror("quote: wrong # of args", args);
}

/*
 | Compile (errset|catch <expr1> <expr2>)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 |     PUSHL <compiled_expr1_literal>    or   PUSHL   <compiled_expr1_literal>
 |     <expr2>                                CALL 1, <errset>|<catch>
 |     CALL 2, <errset>|<catch>
 */
static void bu_compile_errset_or_catch(args, self)
       struct conscell  *args;
       struct alphacell *self;
{
       struct conscell c1, *clisp; int n;
       n = liulength(args);
       if ((n < 1) || (n > 2)) goto er;
       c1.celltype = CONSCELL;
       c1.carp = args->carp;
       c1.cdrp = NULL;
       clisp = bucompile(&c1);
       bu_hoist_errors(clisp);                  /* remove errors from lower level and append to errlist */
       bu_compile_literal(clisp);
       if (n == 2) {
          args = args->cdrp->carp;
          if (args == NULL) {
              EMIT1ATM("PUSHNIL"); EMITEND();
          } else if (args == LIST(thold)) {
              EMIT1ATM("PUSHT"); EMITEND();
          } else {                              /* must quote it because errset will eval */
              bu_compile_literal(quotehold);
              bu_compile_expr(args);            /* and it has already been compiled. */
              EMIT1ATM("PUSHNIL"); EMITEND();
              EMIT1ATM("CONS"); EMITEND();
              EMIT1ATM("CONS"); EMITEND();      /* tos now has (quote <expr>) as 2nd arg to errset */
          }
       }
       EMIT1ATM("CALL");
       EMIT1FIX((long) n);
       EMIT1FIX((long) bu_litref(self));
       EMITEND();
       return;
er:    cerror("errset/catch: wrong # of args", args);
}

/*
 | Compile (time-eval <expr>)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 |     PUSHL <compiled_expr_literal>
 |     CALL 1, <time-eval>
 */
static void bu_compile_time_eval(args, self)
       struct conscell  *args;
       struct alphacell *self;
{
       struct conscell c1, *clisp;
       if (liulength(args) != 1) goto er;
       c1.celltype = CONSCELL;
       c1.carp = args->carp;
       c1.cdrp = NULL;
       clisp = bucompile(&c1);
       bu_hoist_errors(clisp);                           /* remove errors from clisp and append to errlist */
       bu_compile_literal(clisp);
       EMIT1ATM("CALL");
       EMIT1FIX(1L);
       EMIT1FIX((long) bu_litref(self));
       EMITEND();
       return;
er:    cerror("time-eval: wrong # of args", args);
}

/*
 | Compile (declare (type symbol ...) (type symbol ...) (nocompile symbol ...))
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | This is a declaration to the compiler that a certain symbol is is of given type
 | so we generate calls to bu_declare(symbol, type) repeatedly for each symbol of
 | each type. bu_declare will actually store/check the reference and if there is
 | an error will generate the appropriate errors. If a nocompile call then we call
 | the declare_nocompile to set the appropriate flags in the symbol table.
 */
static void bu_compile_declare(args, self)
       struct conscell  *args;
       struct alphacell *self;
{      struct conscell  *arg, *type, *symbol;
       for( ; args != NULL; args = args->cdrp) {
            if (args->celltype != CONSCELL) goto er1;
            arg = args->carp;
            if ((arg == NULL) || (arg->celltype != CONSCELL)) goto er1;
            type = arg->carp;
            for(arg = arg->cdrp; arg != NULL; arg = arg->cdrp) {
                if (arg->celltype != CONSCELL) goto er2;
                symbol = arg->carp;
                if ((symbol == NULL) || (symbol->celltype != ALPHAATOM)) goto er3;
                if ((type != NULL) && (type->celltype == ALPHAATOM) && (strcmp(ALPHA(type)->atom,"nocompile") == 0))
                     bu_declare_nocompile(symbol, arg->linenum);
                else
                     bu_declare(symbol, type, arg->linenum);
            }
       }
       EMIT1ATM("PUSHT"); EMITEND();      /* imitate interpreted one by returning a T */
       return;
er1:   cerror("declare: dotted pair not allowed in arg list", args);
er2:   cerror("declare: dotted pair not allowed in (type symbol) expr", args);
er3:   cerror("declare: expecting an atom found", symbol);
}

/*
 | Compile (defun func (args...) <expr> ....)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | When a defun is found in some other context is compiled into a
 | PUTCLISP func# <clisp># where the <clisp> is the equivalent compiled
 | version of the body. The <clisp> then becomes a literal to the
 | current compiled function. We call the real budefun to do the dirty
 | work and then compile the expression that actually gets put on the
 | atom. We must save the original body first though so that we can
 | restore it later.
 */
static void bu_compile_defun(args, self)
       struct conscell *args, *self;
{
       struct conscell c1, c2, *form, *func_hold;
       struct alphacell *atm;
       int n, atmref, bodref, fntype_hold, want_compile;
       if (!args) goto er;
       atm = ALPHA(args->carp);
       if (!atm || (atm->celltype != ALPHAATOM)) goto er2;

       want_compile = bu_want_compile(atm);                            /* ask if we are to compile this or not */
       push(form);
       n = 1;
       fntype_hold = atm->fntype;
       func_hold = LIST(atm->func);                                    /* remember the original function body and */
       if (FN_ISUS(fntype_hold) || FN_ISCLISP(fntype_hold)) {          /* its type. Now if body is S-expression */
           xpush(func_hold);                                           /* push so that held value is not GC'ed */
           n += 1;
       }
       budefun(args);
       atmref = bu_litref(atm);                                        /* allocate a reference# for this atom */
       if ( atm->fntype == FN_USEXPR ) {                               /* if user expression compile it, if macro we're done */
           c1.celltype = c2.celltype = CONSCELL;                       /* now create ( (lambda(xx)...) atm ) for pass to compile */
           c2.carp = LIST(atm); c2.cdrp = NULL;                        /* using stack based conscell's c1 & c2 to reduce GC requirements */
           c1.carp = LIST(atm->func); c1.cdrp = &c2;
           bu_declare_func(atm, atm->func);                            /* declare the type for future type checking */
           if (want_compile) {
               form = bucompile(&c1);                                  /* compile it for insertion into literals */
               bu_hoist_errors(form);                                  /* remove errors from clisp and append to errlist */
               bodref = bu_litref(form);                               /* allocate a reference# for this literal */
               EMIT1ATM("PUTCLISP");
               EMIT1FIX(atmref);
               EMIT1FIX(bodref);
               EMITEND();                                              /* generate the PUTCLISP of the code body */
           } else {                                                    /* if not compiling generate an (apply 'defun '(args...)) */
               EMIT1ATM("PUSHL");
                  bu_compile_atom(self);                               /* PUSHL <defun> */
               EMITEND();                                              /* PUSHL <args> */
               EMIT1ATM("PUSHL");
                  EMIT1FIX(bu_litref(args));
               EMITEND();
               EMIT1ATM("CALL");                                       /* CALL 2, <apply> */
                  EMIT1FIX(2);
                  bu_compile_atom(LIST(CreateInternedAtom("apply")));
               EMITEND();
           }
           atm->func = ((struct conscell *(*)())(func_hold));          /* restore the original function binding */
           atm->fntype = fntype_hold;                                  /* and its type */
       } else {
           if (atm->fntype == FN_USMACRO) {                            /* a MACRO was defun'ed so we will compile */
               EMIT1ATM("PUSHL");                                      /* code that PUTD's this macro at run time */
               bu_compile_atom(LIST(atm));                             /* PUSHL <atom> */
               EMITEND();
               EMIT1ATM("PUSHL");
               EMIT1FIX(bu_litref(atm->func));                         /* PUSHL <func body> */
               EMITEND();
               EMIT1ATM("CALL");                                       /* CALL 2, <putd> */
               EMIT1FIX(2);
               bu_compile_atom(LIST(CreateInternedAtom("putd")));
               EMITEND();
               want_compile = 0;                                       /* putd will return atom so code below not */
           }                                                           /* required following this phrase */
       }
       if (want_compile) {
           EMIT1ATM("PUSHL");
           EMIT1FIX(atmref);
           EMITEND();                                                  /* a compiled defun always returns the atom defun'ed regardless of type */
       }
       xpop(n);
       return;
er:    cerror("defun: wrong # of args", args);
       return;
er2:   cerror("defun: first argument must be an atom", args);
       return;
}

/*
 | Compile (defmacro func(args...) <expr> ....)
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | When a defmacro call is made the macro 'func' is defined and 'func'
 | is returned. We need to know that 'func' is a macro so that it can
 | be expaned and properly compiled in the future. We compile this
 | expression normally so that a call to 'defmacro' is made at run
 | time but at the same time we interpret it now so that the macro is
 | properly defined for compile time expansion.
 */
static void bu_compile_defmacro(args, self)
       struct conscell *args, *self;
{
       struct alphacell *atm;
       if (!args) goto er;
       atm = ALPHA(args->carp);
       if (!atm || (atm->celltype != ALPHAATOM)) goto er2;
       apply(self, args);
       bu_declare_func(atm, atm->func);                         /* declare the type for future type checking */
       EMIT1ATM("PUSHL");
       bu_compile_atom(LIST(self));                             /* PUSHL <defmacro> */
       EMITEND();
       EMIT1ATM("PUSHL");
       EMIT1FIX(bu_litref(args));                               /* PUSHL <args> */
       EMITEND();
       EMIT1ATM("CALL");                                        /* CALL 2, <apply> */
       EMIT1FIX(2);
       bu_compile_atom(LIST(CreateInternedAtom("apply")));
       EMITEND();
       return;
er:    cerror("defmacro: wrong # of args", args);
       return;
er2:   cerror("defmacro: first argument must be an atom", args);
       return;
}

/*
 | This is the sorted array of name x function pairs that is used to map the
 | name of a function to the C function that compiles it. It is sorted so that
 | a binary search can be applied to it.
 */

static struct { char *name; fptr func; } bu_ftable[] = {
       {"$file-prog$", bu_compile_file_prog},
       {"1+"         , bu_compile_inc},
       {"1-"         , bu_compile_dec},
       {"PAR-setq"   , bu_compile_parsetq},
       {"and"        , bu_compile_and},
       {"arg"        , bu_compile_arg},
       {"arg?"       , bu_compile_argQ},
       {"atom"       , bu_compile_atomp},
       {"car"        , bu_compile_car},
       {"caseq"      , bu_compile_caseq},
       {"catch"      , bu_compile_errset_or_catch},
       {"cdr"        , bu_compile_cdr},
       {"cond"       , bu_compile_cond},
       {"cons"       , bu_compile_cons},
       {"declare"    , bu_compile_declare},
       {"defmacro"   , bu_compile_defmacro},
       {"defun"      , bu_compile_defun},
       {"eq"         , bu_compile_eq},
       {"errset"     , bu_compile_errset_or_catch},
       {"fixp"       , bu_compile_fixp},
       {"floatp"     , bu_compile_floatp},
       {"foreach"    , bu_compile_foreach},
       {"go"         , bu_compile_go},
       {"hunkp"      , bu_compile_hunkp},
       {"length"     , bu_compile_length},
       {"listify"    , bu_compile_listify},
       {"listp"      , bu_compile_listp},
       {"not"        , bu_compile_not},
       {"null"       , bu_compile_null},
       {"numberp"    , bu_compile_numbp},
       {"numbp"      , bu_compile_numbp},
       {"or"         , bu_compile_or},
       {"prog"       , bu_compile_prog},
       {"quote"      , bu_compile_quote},
       {"repeat"     , bu_compile_repeat},
       {"return"     , bu_compile_return},
       {"setq"       , bu_compile_setq},
       {"stringp"    , bu_compile_stringp},
       {"time-eval"  , bu_compile_time_eval},
       {"while"      , bu_compile_while},
       {"zerop"      , bu_compile_zerop}
};

/*
 | This function is just the standard binary search on the table above which
 | returns the address of the function associated with the given name.
 */
static fptr bu_lookup_compile_func(name)
       char *name;
{      register int mid, r;
       register int lo = 0, hi = (sizeof(bu_ftable)/sizeof(bu_ftable[0])) - 1;
       while(lo <= hi) {
           mid = (lo + hi)/2;
           r = strcmp(name, bu_ftable[mid].name);
           if (r == 0) return(bu_ftable[mid].func);
           if (r < 0) hi = mid-1; else lo = mid+1;
       }
       return(NULL);
}

/*
 | Top level LISP function (compile S-expr) will return a list with two elements
 | The first is a list of all the external references and their reference index
 | and the second is a list of all the instructions that make up the S-expression.
 | For example, if the following list is input:
 |
 |           (cond ((zerop x) '(a b c)) (t '(d e f)))
 |
 | Then this function will output the following S-expression. Note that the
 | First element is the literal reference set and the second element is the
 | list containing all the actual code in assembler style form.
 |
 |           ($$clisp$$
 |            (((d e f) . 2) ((a b c) . 1) (x . 0))
 |            ((PUSHLV 0)
 |             (ZEROP)
 |             (JNIL l2)
 |             (PUSHL 1)
 |             (JUMP l1)
 |         l2  (PUSHT)
 |             (JNIL l3)
 |             (PUSHL 2)
 |             (JUMP l1)
 |         l3
 |         l1
 |             (RETURN)))
 |
 | If passed the optional second argument then it must be an atom which when calls
 | to it are found are to be compiled as recursive calls. ie RCALL instructions.
 | This is done to make the resultant code faster. Normally compile operates on
 | a complete lambda expression which it got from a (getd 'func) call. It can then
 | be called (putd 'func (assemble (compile (getd 'func) 'func))) Which will replace
 | the existing lambda body of 'func with a compiled body which was compiled to
 | use RCALLs to the function 'func.
 */
struct conscell *bucompile(form)
       struct conscell *form;
{
       struct conscell *tmp, *tmp2, c1;
       struct conscell *all_hold = all;
       struct conscell *eml_hold = eml;
       struct conscell *errlist_hold = errlist;
       struct alphacell *rcall_atom_hold = rcall_atom;       /* stack all globals whose values must be restored recursively */
       int *return_stack_top_hold = return_stack_top;
       int return_stack_free_hold = return_stack_free;
       struct conscell *litset_hold = litset;
       int nextLabel_hold = nextLabel;
       int nextLit_hold = nextLit;
       int gotoScope_hold = gotoScope;
       int retstack[MAX_RETURN_STACK];                       /* stack of return labels */
       static int level = 0;                                 /* track recursive entry count */
       if (form == NULL) goto er;
       if (form->carp == NULL) goto er;
       if (form->carp->celltype == CLISPCELL)
           return(form->carp);                               /* already compiled & assembled ? */
       if ((form->carp->celltype == CONSCELL) &&
            form->carp->carp &&                              /* already compiled ? */
           (form->carp->carp->celltype == ALPHAATOM) &&
           (strcmp(ALPHA(form->carp->carp)->atom, "$$clisp$$") == 0))
            return(form->carp);
       return_stack_top = retstack;                          /* is local but referenced globally */
       return_stack_free = MAX_RETURN_STACK;                 /* init return stack space */
       push(eml); push(all); push(litset); xpush(eml_hold);  /* all = assembler, litset = literals ref'd */
       push(tmp); xpush(litset_hold); xpush(all_hold);
       push(tmp2); xpush(errlist_hold); push(errlist);
       level += 1;
       if (level == 1) {
           breaks_while_compiling = 0;                       /* no user SIGINT's occured yet */
           lierrh(cerror);                                   /* on any interpreter error call 'cerror' */
           push(typeset);                                    /* new NULL typeset only for TOP level, rest shared */
           bu_pretype();                                     /* declare predefined functions and their types */
       }
       if (form->cdrp != NULL) {
           rcall_atom = ALPHA(form->cdrp->carp);
           if ((rcall_atom == NULL) || (rcall_atom->celltype != ALPHAATOM)) goto er;
       } else
           rcall_atom = NULL;
       nextLabel = 1;                                        /* labels start at 0 */
       nextLit = 0;                                          /* literals start at 1, literal[0] reserved */
       gotoScope = 0;                                        /* current prog scope = level 0 */
       bu_compile_expr(form->carp);                          /* compile the expression accum in 'all' */
       EMIT1ATM("RETURN"); EMITEND();                        /* and append the 'return' expression */
       all = nreverse(all);                                  /* reverse instructions as they are built backwards */
       all = enlist(all);                                    /* listify it because we next return literal references */
       c1.celltype = CONSCELL;                               /* ask literal set to list itself */
       c1.cdrp = NULL;
       c1.carp = litset;
       tmp = busymtlist(&c1);
       tmp = enlist(tmp);                                    /* then make it the car of the returned */
       tmp->cdrp = all;                                      /* expression. */
       tmp2 = new(CONSCELL);
       tmp2->carp = nreverse(errlist);                       /* put them in order they were found (easier to read later) */
       tmp2->cdrp = tmp;
       tmp = new(CONSCELL);
       tmp->carp = LIST(CreateInternedAtom("$$clisp$$"));
       tmp->cdrp = tmp2;
       rcall_atom = rcall_atom_hold;                         /* unstack the old global values incase of recursive use */
       return_stack_top = return_stack_top_hold;
       return_stack_free = return_stack_free_hold;
       litset = litset_hold;
       nextLabel = nextLabel_hold;
       nextLit = nextLit_hold;
       gotoScope = gotoScope_hold;
       all = all_hold;
       eml = eml_hold;
       errlist = errlist_hold;
       level -= 1;                                           /* track recursive level */
       if (level == 0) {                                     /* if about to exit to top level compile */
           lierrh(NULL);                                     /* restore error handler */
           if (breaks_while_compiling > 0) {                 /* and if SIGINT's occurred throw the interrupt now */
               breaks_while_compiling = 0;
               gerror(INTERRUPT);
           }
           xpop(1);                                          /* if back to top level pop the typeset mark entry */
       }
       fret(tmp,10);
er:    ierror("compile");
}
