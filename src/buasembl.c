

/*
 | PC-LISP (C) 1990-1992 Peter J.Ashwood-Smith
 | ---------------------------------------------
 */
#include <stdio.h>
#include "lisp.h"

/*
 | Top level LISP function (assemble S-expr) expects an expression output from
 | (compile <expr>) which it will then traverse and convert to pure byte code.
 | It does this by making two passes over the list of assembly language statements.
 | The first pass binds each label to its offset from the beginning of the list
 | to do this we need to increment an instruction pointer by the size of each
 | instruction as we go. Then, when this symbol table has been built we can
 | allocate the buffer that will contain the machine code and make a second pass
 | to populate it with the actual op code and arguments for each instruction.
 | We use the bu_lookup_instruction function to get the opcode and number of args
 | for each instruction. The size of each instruction is then 1 + 2*nargs since
 | all instruction arguments are 16 bits literal or label references.
 |
 |           ($$clisp$$ nil
 |            (((d e f) . 3) ((a b c) . 2) (x . 1))
 |            ((PUSHLV 1)
 |             (ZEROP)
 |             (JNIL l2)
 |             (PUSHL 2)
 |             (JUMP l1)
 |         l2  (PUSHT)
 |             (JNIL l3)
 |             (PUSHL 3)
 |             (JUMP l1)
 |         l3
 |         l1
 |             (RETURN)))
 */

/*
 | The assembler uses symbol tables to track the location of labels so that jump
 | displacements can be computed.
 */
extern struct conscell *busymtcreate(), *busymtmember(), *busymtadd();
/*
 | The table is the instruction table it gives foreach instruction the number
 | of 16 bit arguments that it takes hence the size of any entry is 1+nargs*2
 | bytes. The opcode for the instruction is its position in the table + 1. This
 | is all the information required to assemble a list of instructions into an
 | array of byte coded instructions. The instrcount is used when debugging as a
 | way to determine which instructions are most frequently executed.
 */
static struct { char *name; int nargs, kind, instrcount; } bu_itable[] = {
              {"ARG"       ,   0  , OPK_STACK, 0        },
              {"ARG?"      ,   0  , OPK_STACK, 0        },
              {"ATOM"      ,   0  , OPK_STACK, 0        },
              {"CALL"      ,   2  , OPK_LITN1, 0        },
              {"CALLNF"    ,   2  , OPK_LITN1, 0        },
              {"CAR"       ,   0  , OPK_STACK, 0        },
              {"CDR"       ,   0  , OPK_STACK, 0        },
              {"CONS"      ,   0  , OPK_STACK, 0        },
              {"COPYFIX"   ,   0  , OPK_STACK, 0        },
              {"DDEC"      ,   0  , OPK_STACK, 0        },
              {"DEC"       ,   0  , OPK_STACK, 0        },
              {"DUP"       ,   0  , OPK_STACK, 0        },
              {"EQ"        ,   0  , OPK_STACK, 0        },
              {"FIXP"      ,   0  , OPK_STACK, 0        },
              {"FLOATP"    ,   0  , OPK_STACK, 0        },
              {"HUNKP"     ,   0  , OPK_STACK, 0        },
              {"INC"       ,   0  , OPK_STACK, 0        },
              {"JEQ"       ,   1  , OPK_BRANCH, 0       },
              {"JNIL"      ,   1  , OPK_BRANCH, 0       },
              {"JNNIL"     ,   1  , OPK_BRANCH, 0       },
              {"JUMP"      ,   1  , OPK_BRANCH, 0       },
              {"JZERO"     ,   1  , OPK_BRANCH, 0       },
              {"LENGTH"    ,   0  , OPK_STACK , 0       },
              {"LISTIFY"   ,   0  , OPK_STACK,  0       },
              {"LISTP"     ,   0  , OPK_STACK , 0       },
              {"NULL"      ,   0  , OPK_STACK , 0       },
              {"NUMBP"     ,   0  , OPK_STACK , 0       },
              {"POP"       ,   0  , OPK_STACK , 0       },
              {"PUSHL"     ,   1  , OPK_LIT1  , 0       },
              {"PUSHLV"    ,   1  , OPK_LIT1  , 0       },
              {"PUSHNIL"   ,   0  , OPK_STACK , 0       },
              {"PUSHT"     ,   0  , OPK_STACK , 0       },
              {"PUTCLISP"  ,   2  , OPK_LIT2  , 0       },
              {"RCALL"     ,   1  , OPK_N1    , 0       },
              {"RETURN"    ,   0  , OPK_STACK , 0       },
              {"SETQ"      ,   1  , OPK_LIT1  , 0       },
              {"SPOP"      ,   1  , OPK_LIT1  , 0       },
              {"SPUSHARGS" ,   1  , OPK_LIT1  , 0       },
              {"SPUSHLEX"  ,   1  , OPK_LIT1  , 0       },
              {"SPUSHNIL"  ,   1  , OPK_LIT1  , 0       },
              {"SPUSHWARG" ,   1  , OPK_LIT1  , 0       },
              {"STORR"     ,   1  , OPK_STACK , 0       },
              {"STRINGP"   ,   0  , OPK_STACK , 0       },
              {"ZEROP"     ,   0  , OPK_STACK , 0       },
              {"ZPOP"      ,   0  , OPK_STACK , 0       },
              {"ZPUSH"     ,   1  , OPK_LIT1  , 0       }};

/*
 | This function returns the op code and number of arguments for a given
 | instruction. It does this with a simple binary search on the above table.
 */
int bu_lookup_instruction(name, opcode, nargs, kind)
       char *name; int *opcode, *nargs, *kind;
{      register int mid, r;
       register int lo = 0, hi = (sizeof(bu_itable)/sizeof(bu_itable[0])) - 1;
       while(lo <= hi) {
           mid = (lo + hi)/2;
           r = strcmp(name, bu_itable[mid].name);
           if (r == 0) {
              *opcode = mid+1;
              *nargs = bu_itable[mid].nargs;
              *kind = bu_itable[mid].kind;
              return(1);
           }
           if (r < 0) hi = mid-1; else lo = mid+1;
       }
       return(0);
}

/*
 | This function returns an instruction by op code and gives the number of args
 | name etc. It is used by the disassemble primitive to figure out what a given
 | op code's mnomonic is.
 */
int bu_byop_lookup_instruction(opcode, name, nargs, kind, count)
       char **name; int opcode, *nargs, *kind, **count;
{
       if ((opcode <= 0) || (opcode >= OP_MAX_OP)) return(0);
       opcode -= 1;
       *name  =  bu_itable[opcode].name;
       *nargs =  bu_itable[opcode].nargs;
       *kind  =  bu_itable[opcode].kind;
       *count = &bu_itable[opcode].instrcount;  /* this allows caller to set counter */
       return(1);
}

/*
 | The actual (assemble <clisp>) primtiive.
 */
struct conscell *buassemble(form)
       struct conscell *form;
{
       struct conscell *label, *pair, *labset;
       struct conscell *literals, *instructions, *inst, *ip, c1, c2, c3;
       int    nlit, i, op, n, kind; long dist, pos, ipo = 0L;
       struct clispcell *clisp;
       struct conscell **l;
       char   *code;

      /*
       | Initially extract the liberals and instructions lists. And create our set of
       | labels in prepation for pass 1 which builds a symbol table of label addresses
       | and computes the total size of the code. Also make sure the first element in
       | the passed argument is $$clisp$$.
       */
       if ((form == NULL)||(form->cdrp != NULL)) goto er1;
       if (!(form = form->carp)) goto er2;
       if (form->celltype == CLISPCELL) return(form);                 /* already assembled? */
       if (form->celltype != CONSCELL) goto er3;
       if (!form->carp || form->carp->celltype != ALPHAATOM) goto er3;
       if (strcmp(ALPHA(form->carp)->atom, "$$clisp$$") != 0) goto er4;
       if (!(form = form->cdrp)) goto er5;
       if (form->carp) goto er6;                                      /* cannot assemble if errors */
       if (!(form = form->cdrp)) goto er7;
       literals = form->carp;
       form = form->cdrp;
       if (!form) goto er8;
       instructions = form->carp;
       if (form->cdrp) goto er9;
       if (!instructions) goto ok;
       if (instructions->celltype != CONSCELL) goto er10;
       if (literals && (literals->celltype != CONSCELL)) goto er11;

      /*
       | About to start real work so push these variables on mark stack so we don't loose them to
       | the garbage collector on route.
       */
       push(labset); push(clisp);

      /*
       | Make a first pass through the instruction list and create a symbol table that maps each
       | label to its offset in the code. In addition the size of the code is computed and ipo
       | (instruction pointer offset) when done will reflect the size in bytes of the code.
       */
       c1.celltype = c2.celltype = c3.celltype = CONSCELL;
       c1.carp = c1.cdrp = NULL;
       labset = busymtcreate(&c1);
       c1.carp = labset; c1.cdrp = &c2; c2.cdrp = &c3; c3.cdrp = NULL;
       for(ip = instructions; ip != NULL; ip = ip->cdrp) {
           TEST_BREAK();                                 /* if interrupted abort */
           if (!(label = ip->carp)) continue;            /* NIL's are like a NOOP no instr generated */
           if (label->celltype == ALPHAATOM) {
               c2.carp = label;
               c3.carp = newintop(ipo);
               xpush(c3.carp);                           /* protect the ipo from GC */
               if (busymtadd(&c1) == NULL) goto er13;
               xpop(1);
           } else {
               if (label->celltype != CONSCELL) goto er14;
               inst = label->carp;
               if (!inst || inst->celltype != ALPHAATOM) goto er15;
               if (!bu_lookup_instruction(ALPHA(inst)->atom, &op, &n, &kind)) goto er16;
               ipo += 1 + 2 * n;
           }
       }

      /*
       | Actually create the clispcell now with its literal array and its code array
       | each of the proper size. Must use calloc because we want them NULL populated.
       | Each array is prefixed with an <int> containing its byte size. This is used
       | by various routines that dump or read/write the clisp objects. Since literal[0]
       | is reserved we must add 1 to the length of the literals acutally passed to
       | us.
       */
       nlit = n = liulength(literals)+1;
       l = (struct conscell **)calloc(n+1, sizeof(struct conscell *));
       if (!l) goto er17;
       *((int *)l) = n * sizeof(struct conscell *);
       l = (struct conscell **)((char *)l + sizeof(int));
       code = (char *)calloc(ipo+1+sizeof(int), sizeof(char));
       if (!code) goto er18;
       *((int *)code) = ipo+1;
       code += sizeof(int);

      /*
       | Now allocate the actual clisp structure and point it to the NULL filled literal
       | and code arrays.
       */
       clisp = CLISP(new(CLISPCELL));
       clisp->literal = l;
       clisp->code = code;

      /*
       | Go through the literals list of dotted pairs and populate the literals array with
       | each item eg if ( (a b c) . 2 ) is an element of the literals list then we put the
       | list (a b c) as the second literal in the literals array. We make sure that the
       | literals really are a list of dotted pairs whose second element is a fixnum and
       | that no fixnum is duplicated. If a literal is a $$clisp$$ LIST then recursively
       | assemble this object into a clisp object.
       */
       while(literals) {
            TEST_BREAK();
            if (!(pair = literals->carp)) goto er19;
            if (!GetFix(pair->cdrp, &pos)) goto er20;
            if ((pos <= 0) || (pos >= n)) goto er21;    /* bounds check (0 is reserved by us!) */
            if (l[pos]) goto er22;                      /* duplicate check */
            l[pos] = pair->carp;
            literals = literals->cdrp;
            pair = pair->carp;
            if (pair && (pair->celltype == CONSCELL)) {
                pair = pair->carp;
                if (pair && (pair->celltype == ALPHAATOM)) {
                    if (strcmp(ALPHA(pair)->atom, "$$clisp$$") == 0) {
                        struct conscell c1;
                        c1.celltype = CONSCELL;
                        c1.cdrp = NULL;
                        c1.carp = l[pos];
                        l[pos] = buassemble(&c1);
                    }
                }
            }
       }

      /*
       | Literals 1...N populated now populate literal[0] with a pointer to the clisp cell being
       | created. This is so that give the address of a literal containing its offset we can find
       | our way back to the actual clisp object. This is used by errset/catch functions when
       | unwinding the SPUSH instructions of compiled code.
       */
       l[0] = LIST(clisp);

      /*
       | Make a second pass through the instruction list but this time populate the code buffer
       | with the actual byte coded instructions. When we encounter (JUMP label) type of instr
       | we lookup the target address in the symbol table and then compute the displacment to
       | this address and store it in the code buffer as arguments for the given instruction.
       | When we encounter a ZPUSH instruction its literal must be set to FIXFIX(lit#, offset)
       | the LIT# is already set but the offset must be set to the actual code offset of the
       | instruction following the ZPUSH.
       */
       ipo = 0;
       c2.cdrp = NULL;
       for(ip = instructions; ip != NULL; ip = ip->cdrp) {
           TEST_BREAK();
           if (!(label = ip->carp)) continue;                /* skip NULL instruction, just a NOOP */
           if (label->celltype == CONSCELL) {
               if (!(inst = label->carp) || (inst->celltype != ALPHAATOM)) goto er24;
               if (!bu_lookup_instruction(ALPHA(inst)->atom, &op, &n, &kind)) goto er25;
               code[ipo++] = op;
               for(i = 0; i < n; i++) {
                   register struct conscell *temp;
                   if (!(label = label->cdrp)) goto er26;
                   if (!(temp = label->carp)) goto er27;
                   if (temp->celltype == ALPHAATOM) {
                       c2.carp = temp;
                       if (!(pair = busymtmember(&c1))) goto er28;
                       if (!GetFix(pair->cdrp, &dist)) goto er29;
                       dist -= ipo + 2*n;
                   } else {
                       if (!GetFix(temp, &dist)) goto er30;
                   }
                   code[ipo++] = (dist >> 8) & 0xff;
                   code[ipo++] = (dist & 0xff);
               }
               if (strcmp(ALPHA(inst)->atom, "ZPUSH") == 0) {
                   if ((dist <= 0) || (dist >= nlit)) goto er31;
                   if ((l[dist] == NULL) || (l[dist]->celltype != FIXFIXATOM)) goto er32;
                   if (FIXFIX(l[dist])->atom1 != dist) goto er33;
                   FIXFIX(l[dist])->atom2 = ipo;
               }
           }
       }
       code[ipo] = 0;        /* place a bad instruction to end of code for disassemble stop */

      /*
       | nlambda expressions begin with ZPUSH/SPUSHARGS instruction so we set the eval bit
       | to '1' only if the code is not assigning the entire argument list to a local.
       */
       if ((*code == OP_ZPUSH) && (*(code+3) == OP_SPUSHARGS))
           clisp->eval = 0;
       else
           clisp->eval = 1;

      /*
       | Return the compiled LISP expression.
       */
ok:    fret(LIST(clisp),2);

      /*
       | Throw error assembling instructions.
       */
 er1:  ierror("assemble:1");
 er2:  ierror("assemble:2");
 er3:  ierror("assemble:3");
 er4:  ierror("assemble:4");
 er5:  ierror("assemble:5");
 er6:  ierror("assemble:6");
 er7:  ierror("assemble:7");
 er8:  ierror("assemble:8");
 er9:  ierror("assemble:9");
er10:  ierror("assemble:10");
er11:  ierror("assemble:11");
er13:  ierror("assemble:13");
er14:  ierror("assemble:14");
er15:  ierror("assemble:15");
er16:  ierror("assemble:16");
er17:  ierror("assemble:17");
er18:  ierror("assemble:18");
er19:  ierror("assemble:19");
er20:  ierror("assemble:20");
er21:  ierror("assemble:21");
er22:  ierror("assemble:22");
er24:  ierror("assemble:24");
er25:  ierror("assemble:25");
er26:  ierror("assemble:26");
er27:  ierror("assemble:27");
er28:  ierror("assemble:28");
er29:  ierror("assemble:29");
er30:  ierror("assemble:30");
er31:  ierror("assemble:31");
er32:  ierror("assemble:32");
er33:  ierror("assemble:33");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
