/* --  */

/*
 | PC-LISP (C) 1990-1992 Peter J.Ashwood-Smith
 | ---------------------------------------------
 | liceval.c : this file contains the function evalclisp which will cause a
 | compiled LISP object CLISPCELL to be executed. It is basically just a big
 | switch statement that fetches the next instruction, and dispatches one of
 | the 40 or so instructions depending on the op code. It is designed to run
 | effeciently and hence is a bit cryptic. For a complete description of each
 | instruction look in the bucompil.c file.
 */

#include <stdio.h>
#include "lisp.h"

static bcierror();


/*
 | The byte coded interpreters stack/mark stack, we put a guard word at the end
 | and if ever this value changes we know that the stack has been corrupted. We
 | do not do a stack top check every push/pop as this would be extremely costly
 | rather we abort the moment the gard is detected to have changed. We use '1'
 | as a value for the guard since this is not a legal pointer to a word and will
 | never naturally occur on the stack.
 */
static struct cl_stack_s {
              struct conscell *stack[MSSIZE];
              int guard;
            } clisp = { { NULL }, 1 };

static struct conscell **tos = &clisp.stack[-1];

/*
 | These functions permit the errset function to query/reset the BCI top of stack
 | value before/after an errset jump is taken.
 */
struct conscell **getclisptos()
{    return(tos);
}

void putclisptos(ntos)
     struct conscell **ntos;
{    tos = ntos;
}

/*
 | Mark the contents of the byte coded interpreters stack.
 */
void markclisp()
{    register struct conscell **s = &clisp.stack[0];
     if (clisp.guard != 1) bcierror("clisp stack overflow", NULL, 0);
     while (s <= tos)
        marklist(*s++);
}

/*
 | When the byte coded interpreter goes wrong it calls this function. This function
 | then prints an error message, dumps the stack and tries to figure out which
 | function went wrong.
 */
static bcierror(msg, code, recover)
     char *msg; char *code;  int recover;
{    struct conscell **s; char *nam, *liuclnam(); int i;
     printf("--- byte coded interpreter: error: %s ---\n", msg);
     nam = liuclnam(code);
     if (nam)
         printf("--- during execution of function '%s' ---\n", nam);
     else
         printf("--- during execution of a disembodied clisp ---\n");
     printf("--- byte coded interpreter top 20 stacked items were ---\n\n");
     i = 0;
     for(s = tos; s >= &clisp.stack[0]; s--) {
         printf("[] ");
         prettyprint(*s, 3, 3, stdout);
         printf("\n");
         if (i++ >= 20) break;
     }
     printf("\n");
     if (recover) ierror("byte coded interpreter");
     printf("--- unsafe to continue, forcing abort now ---\n");
     exit(-1);
}

/*
 | Similar to the above error function but for unbound atoms encountered during PUSHLV evaluations.
 */
static void cliuberror(t, code)
     struct conscell *t; char *code;
{
     char work[MAXATOMSIZE+32];
     sprintf(work," unbound atom '%s'", ALPHA(t)->atom);
     bcierror(work, code, 1);
}

/*
 |  The is to run the byte code machine using code 'code' referencing literals
 | 'literals' and arguments 'form'.
 */
struct conscell *evalclisp( code, literals, form )
       char *code; struct conscell *form, **literals;
{
       register char *ip = code;
       register struct conscell **lits = literals;
       register struct conscell *t;
       register struct conscell **etos = tos;     /* record for return test */

      /*
       | If the guard value ever changes it is because the clisp.stack overflowed into it. If this ever
       | happens we must get out and abort execution.
       */
       if (clisp.guard != 1) bcierror("clisp stack overflow", code, 0);

 next:

#      if defined(TRACE_INSTR)
          printf("%07d [%05d] TOS ---> ", ip, tos - &clisp.stack[0]);
          if (tos >= &clisp.stack[0])
              prettyprint(*tos, 18, 18, stdout);
          else
              printf("-0-");
          printf("\n");
          bu_disassemble_1_inst(stdout, ip, lits);
#      endif

#      if defined(COUNT_INSTR)
       {  int dumi, *cntr; char *name;
          if (bu_byop_lookup_instruction(*ip, &name, &dumi, &dumi, &cnter))
              *cntr += 1;
       }
#      endif

      /*
       | This is the instruction fetch and lookup loop. It just switches to the appropriate OP code case
       | which is expected to advance the instruction pointer as necessary to the next instruction boundary.
       | When finished decoding and executing its instruction each case will jump back to the next: label
       | above to process the next instruction.
       */
       switch(*ip++) {

         /*
          | Instruction ARG: will use the top of stack to get an argument index which it then uses to get
          | the appropriate argument which is then pushed back on the stack.
          */
          case OP_ARG:        { register int n = FIX(*tos)->atom;
                                for(t = form; --n > 0; t = t->cdrp);
                                *tos = t->carp;
                              }
                              goto next;
         /*
          | Instruction ARGQ: is like ARG except that before the index is an expression to be used if the
          | corresponding argument does not exist.
          */
          case OP_ARGQ:       { register int n = FIX(*--tos)->atom;
                                for(t = form; t && --n > 0; t = t->cdrp);
                                *tos = t ? t->carp : *(tos+1);
                              }
                              goto next;

         /*
          | Instruction ATOM: will replace the top of stack with 't' if and only iff the top of stack
          | cell is not a list.
          */
          case OP_ATOM:       *tos = ((t = *tos) && ((t->celltype == CONSCELL) || (t->celltype == HUNKATOM))) ? NULL : LIST(thold) ;
                              goto next;

         /*
          | Instruction CALL <n> <lit>: will call the function <lit> with <n> arguments taken from the stack.
          | It pops the top <n> items from the stack and builds a list 't' which it then passes to apply with
          | the literal function. After the call it gathers back up the list of arguments and puts them on
          | its free list again.
          */
          case OP_CALL:       {  register int n = XSHORT(ip); ip += 2;
                                 t = *++tos = NULL;
                                 while(n--) {
                                     NEWCONS(t);
                                     t->cdrp = *tos--;
                                     t->carp = *tos;
                                     *tos = t;
                                 }
                              }
                              *tos = apply(lits[XSHORT(ip)], *tos);
                              ip += 2;
                              while(t) {
                                 t->carp = lifreecons;
                                 lifreecons = t;
                                 t = t->cdrp;
                                 lifreecons->cdrp = NULL;
                              }
                              goto next;

         /*
          | Instruction CALLNF <n> <lit>: identical to CALL but does not free its args. This is used to compile
          | a call to an nlambda.
          */
          case OP_CALLNF:     {  register int n = XSHORT(ip); ip += 2;
                                 *++tos = NULL;
                                 while(n--) {
                                     NEWCONS(t);
                                     t->cdrp = *tos--;
                                     t->carp = *tos;
                                     *tos = t;
                                 }
                              }
                              *tos = apply(lits[XSHORT(ip)], *tos);
                              ip += 2;
                              goto next;


         /*
          | Instruction RCALL <n> : will recursively call ourselves with <n> arguments taken from the stack.
          | It pops the top <n> items from the stack and builds a list 't' which it then uses to pass to
          | evalclisp as the new argument list. This removes the overhead of a call to apply which must then
          | dechypher the fact that we are comming right back here again.
          */
          case OP_RCALL:      {  register int n = XSHORT(ip); ip += 2;
                                 t = *++tos = NULL;
                                 while(n--) {
                                     NEWCONS(t);
                                     t->cdrp = *tos--;
                                     t->carp = *tos;
                                     *tos = t;
                                 }
                              }
                              *tos = evalclisp(code, literals, *tos);
                              while(t) {
                                 t->carp = lifreecons;
                                 lifreecons = t;
                                 t = t->cdrp;
                                 lifreecons->cdrp = NULL;
                              }
                              goto next;

         /*
          | Instruction CDR: will replace the top of stack with its cdr if non nil.
          */
          case OP_CDR:        if (t = *tos) (*tos) = t->cdrp;
                              goto next;

         /*
          | Instruction CDR: will replace the top of stack with its car if non nil.
          */
          case OP_CAR:        if (t = *tos) (*tos) = t->carp;
                              goto next;

         /*
          | Instruction CONS: will pop the top two elements from the stack and replace them with a cons cell
          | with the two poped elements as its cdr and car respectively. We access the freecons list directly
          | for speed and only call new if the list is empty. We do not have to set the type field because it
          | is set to CONSCELL by the free code.
          */
          case OP_CONS:       NEWCONS(t);
                              t->cdrp = *tos--; t->carp = *tos; *tos = t;
                              goto next;

         /*
          | Instruction COPYFIX: will replace the top of stack fixnum with a new copy of the fixnum.
          */
          case OP_COPYFIX:    NEWINTOP( FIX(*tos)->atom, t);
                              *tos = t;
                              goto next;

         /*
          | Instruction DDEC: will destructively decrement the value of the fixnum object on the top of stack.
          */
          case OP_DDEC:       FIX(*tos)->atom -= 1;
                              goto next;

         /*
          | Instruction DEC: will decrement the top fix on the stack and allocate new cell for the value.
          */
          case OP_DEC:        NEWINTOP( FIX(*tos)->atom - 1L, t);
                              *tos = t;
                              goto next;


         /*
          | Instruction DUP: will push the current top of stack on top of itself. Ie it duplicates the top of stack.
          */
          case OP_DUP:        *(tos+1) = *tos; tos++;
                              goto next;

         /*
          | Instruction EQ: will push t if and only if the top two stack elements (which it pops) are (eq).
          */
          case OP_EQ:         tos--; *tos = ((*tos == *(tos+1)) || eq(*tos, *(tos+1))) ? LIST(thold) : NULL;
                              goto next;

         /*
          | Instruction FIXP: will push t if and only if the top of stack (which it pops) is a fixnum cell.
          */
          case OP_FIXP:       *tos = (*tos != NULL) && ((*tos)->celltype == FIXATOM) ? LIST(thold) : NULL;
                              goto next;

         /*
          | Instruction FLOATP: will push t if and only if the top of stack (which it pops) is a flonum cell.
          */
          case OP_FLOATP:     *tos = (*tos != NULL) && ((*tos)->celltype == REALATOM) ? LIST(thold) : NULL;
                              goto next;

         /*
          | Instruction FLOATP: will push t if and only if the top of stack (which it pops) is a hunk cell.
          */
          case OP_HUNKP:      *tos = (*tos != NULL) && ((*tos)->celltype == HUNKATOM) ? LIST(thold) : NULL;
                              goto next;

         /*
          | Instruction INC: will increment the top fix on the stack and allocate new cell for the value.
          */
          case OP_INC:        NEWINTOP( FIX(*tos)->atom + 1L, t);
                              *tos = t;
                              goto next;

         /*
          | Instruction JEQ <displ>: will jump relative by <displ> if and only if the top two stack items are
          | eq. The stack is popped twice.
          */
          case OP_JEQ:        if ((*tos == *(tos-1)) || eq(*tos, *(tos-1))) ip += XSHORT(ip);
                              ip += 2;
                              tos -= 2;
                              goto next;

         /*
          | Instruction JNIL <displ>: will jump relative by <displ> if and only if the top of stack is NULL.
          | The item is popped from the stack.
          */
          case OP_JNIL:       if (!(*tos--)) ip += XSHORT(ip);
                              ip += 2;
                              goto next;

         /*
          | Instruction JNNIL <displ>: will jump relative by <displ> if and only if the top of stack is not NULL.
          | The item is popped from the stack.
          */
          case OP_JNNIL:      if ((*tos--)) ip += XSHORT(ip);
                              ip += 2;
                              goto next;

         /*
          | Instruction JUMP <displ>: will unconditionally jump relative by <displ>.
          */
          case OP_JUMP:       ip += XSHORT(ip) + 2;
                              goto next;

         /*
          | Instruction JZERO <displ>: will jump relative by <displ> if and only if the top of stack is fixnum 0.
          | The item is popped from the stack.
          */
          case OP_JZERO:      if (FIX(*tos)->atom == 0L) ip += XSHORT(ip);
                              ip += 2;
                              tos --;
                              goto next;

         /*
          | Instruction LENGTH: will push a fixnum whose value is the length of the list on top of stack.
          */
          case OP_LENGTH:     NEWINTOP( liulength(*tos), t);
                              *tos = t;
                              goto next;

         /*
          | Instruction LISTP: will replace the top of stack with 't' if and only iff the top of stack
          | cell is a cons cell.
          */
          case OP_LISTP:      *tos = (*tos == NULL) || ((*tos)->celltype == CONSCELL) ? LIST(thold) : NULL;
                              goto next;

         /*
          | Instruction LISITFY: will use the top of stack to get an argument index which it then uses as a
          | position from which to return a list of all arguments. Must copy these since they are free'd after
          | the call.
          */
          case OP_LISTIFY:    { register int n = FIX(*tos)->atom;
                                for(t = form; t && --n > 0; t = t->cdrp);
                                *tos = t ? topcopy(t) : NULL;
                              }
                              goto next;

         /*
          | Instruction NULL: will replace the top of stack with 't' if and only iff the top of stack
          | is NULL.
          */
          case OP_NULL:       *tos = (*tos == NULL) ? LIST(thold) : NULL;
                              goto next;

         /*
          | Instruction NUMBP: will replace the top of stack with 't' if and only iff the top of stack
          | is a flonum or fixnum cell.
          */
          case OP_NUMBP:      *tos = ((*tos != NULL) && ( ((*tos)->celltype == REALATOM) ||
                                                         ((*tos)->celltype == FIXATOM)
                                                        ) ) ? LIST(thold) : NULL;
                              goto next;


         /*
          | Instruction POP: will pop one item from the stack.
          */
          case OP_POP:        tos--;
                              goto next;

         /*
          | Instruction PUSHL <literal>: will push the literal <literal> on the stack.
          */
          case OP_PUSHL:      *++tos = lits[XSHORT(ip)];
                              ip += 2;
                              goto next;

         /*
          | Instruction PUSHLV <literal>: will push the value of the atom literal <literal> on the stack.
          | if no valstack, push the func if it is a user type.
          */
          case OP_PUSHLV:     t = ALPHA(lits[XSHORT(ip)])->valstack;
                              if (t) { *++tos = ALPHA(lits[XSHORT(ip)])->valstack->carp; ip += 2; goto next; }
                              t = lits[XSHORT(ip)];
                              if (!FN_ISUS(ALPHA(t)->fntype) && !FN_ISCLISP(ALPHA(t)->fntype)) cliuberror(t, code);
                              *++tos = LIST(ALPHA(t)->func);
                              ip += 2;
                              goto next;

         /*
          | Instruction PUSHLNIL: will push a NULL on the stack.
          */
          case OP_PUSHNIL:    *++tos = NULL;
                              goto next;

         /*
          | Instruction PUSHT: will push the t atom on the stack.
          */
          case OP_PUSHT:      *++tos = LIST(thold);
                              goto next;

         /*
          | Install a compiled function on an atom. The first short is the atom literal and the
          | second short is the clisp literal.
          */
          case OP_PUTCLISP:   t = lits[XSHORT(ip)];
                              ip += 2;
                              ALPHA(t)->func = ((struct conscell *(*)())( lits[XSHORT(ip)] ));
                              ALPHA(t)->fntype = FN_CLISP;
                              ip += 2;
                              goto next;

         /*
          | Instruction RETURN: will cause return from the current byte code machine back to pure interpreter.
          */
          case OP_RETURN:     goto ret;

         /*
          | Instruction SETQ <literal>: will assign the top of stack to the value of the atom <literal>
          */
          case OP_SETQ:       t = lits[XSHORT(ip)];
                              if (ALPHA(t)->valstack == NULL) {
                                  register struct conscell *v;
                                  NEWCONS(v);
                                  ALPHA(t)->botvaris = GLOBALVAR;
                                 (ALPHA(t)->valstack = v)->carp = *tos;
                              } else
                                  ALPHA(t)->valstack->carp = *tos;
                              ip += 2;
                              goto next;

         /*
          | Instruction SPOP <literal>: will pop a binding from the shallow stack of atom <literal> it also puts
          | the free cons cell back on the freecons list to minimize gabage collections.
          */
          case OP_SPOP:       {  register struct conscell **v = & ( ALPHA(lits[XSHORT(ip)])->valstack );
                                 t = *v;
                                 *v = t->cdrp;
                              }
                              t->cdrp = NULL;
                              t->carp = lifreecons;
                              lifreecons = t;
                              ip += 2;
                              goto next;

         /*
          | Instruction SPUSHNIL <literal>: will push a nil on the shallow stack of bindings for atom <literal>.
          */
          case OP_SPUSHNIL:   { register struct conscell **v =  & ( ALPHA(lits[XSHORT(ip)])->valstack );
                                NEWCONS(t);
                                t->cdrp = *v;
                                t->carp = NULL;
                                *v = t;
                              }
                              ip += 2;
                              goto next;

         /*
          | Instruction SPUSHLEX <literal>: will push the length of the arg list on the shallow stack of bindings
          | for atom <literal>. We use tos as a temporary holding spot for the length fixnum so that it does not
          | get GC'ed.
          */
          case OP_SPUSHLEX:   NEWINTOP(liulength(form), t);
                              *++tos = t;
                              { register struct conscell **v =  & ( ALPHA(lits[XSHORT(ip)])->valstack );
                                NEWCONS(t);
                                t->cdrp = *v;
                                t->carp = *tos--;
                                *v = t;
                              }
                              ip += 2;
                              goto next;

         /*
          | Instruction SPUSHWARG <literal>: will push a new arg on the shallow stack of bindings for atom <literal> and
          | then wind in the argument list by one.
          */
          case OP_SPUSHWARG:  { register struct conscell **v =  & ( ALPHA(lits[XSHORT(ip)])->valstack );
                                NEWCONS(t);
                                t->cdrp = *v;
                                t->carp = form->carp;
                                *v = t;
                              }
                              ip += 2;
                              form = form->cdrp;
                              goto next;

         /*
          | Instruction SPUSHARGS <literal>: will push a list of all args on the shallow stack of bindings for atom <literal>.
          | A copy is not required because nlambda calls DO NOT FREE THEIR ARGS AFTER THE CALL (ie they use CALLNF).
          */
          case OP_SPUSHARGS:  { register struct conscell **v =  & ( ALPHA(lits[XSHORT(ip)])->valstack );
                                NEWCONS(t);
                                t->cdrp = *v;
                                t->carp = form;
                                *v = t;
                              }
                              ip += 2;
                              goto next;

         /*
          | Instruction STORR <n>: will insert the top of stack into the stack <n> from the current top. This is normally
          | used to store below the top in a location reserved for a temporary calculation eg a return value etc. in
          | repeat loops.
          */
          case OP_STORR:      *(tos + XSHORT(ip)) = *tos;
                              tos--;
                              ip += 2;
                              goto next;

         /*
          | Instruction STRINGP: will replace the top of stack with 't' if and only iff the top of stack
          | is a string cell.
          */
          case OP_STRINGP:    *tos = (*tos != NULL) && ((*tos)->celltype == STRINGATOM) ? LIST(thold) : NULL;
                              goto next;

         /*
          | Instruction ZEROP: will replace the top of stack with 't' if and only iff the top of stack
          | is the fixnum 0 or the flonum 0.0.
          */
          case OP_ZEROP:      *tos = ( (*tos != NULL) &&
                                       ( (((*tos)->celltype == FIXATOM)  && (FIX(*tos)->atom == 0L))   ||
                                         (((*tos)->celltype == REALATOM) && (REAL(*tos)->atom == 0.0))
                                     ) ) ? LIST(thold) : NULL;
                              goto next;

         /*
          | Instruction ZPUSH <literal> will push the address of the address of this FIXFIX literal on
          | the eval stack so that throw/UnProg can reverse all the SPUSH** calls and put shallow stacks
          | back to proper levels.
          */
          case OP_ZPUSH:      expush(lits[XSHORT(ip)]);
                              ip += 2;
                              goto next;
         /*
          | Instruction ZPOP will just pop the eval mark stack once.
          */
          case OP_ZPOP:       expop(1);
                              goto next;

          default:            bcierror("invalid op code encountered", code, 1);
       }
 ret:  if (tos != etos+1) bcierror("stack top wrong on exit", code, 1);
       return(*tos--);
}
