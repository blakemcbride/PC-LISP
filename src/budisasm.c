/* */

/*
 | PC-LISP (C) 1991-1992 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include "lisp.h"

/*
 | Top level LISP function (disassemble clisp [port]) expects an S-expression of type
 | compiled LISP 'clisp' which it will then disassemble. Basically it just dumps
 | the literal array and the code to the given output port or stdout if no port
 | is given. It will put out comments when an instruction is a branch indicating the
 | absolute address being jumped to and when an instruction references a literal
 | the literal will be dumped in the comment if there is room for it. Following the
 | code dump is a literal dump. Basically these two binary arrays are what make up
 | the binary coded data to the byte coded interpreter.
 |
 | +----------------------------------------------------------------------+
 | | ADDR   LABEL   INSTR       BYTE CODE          COMMENTS               |
 | +----------------------------------------------------------------------+
 | 0314528          PUSHLV      024 000000         ;x
 | 0314531          ZEROP       033
 | 0314532          JNIL        015 000006         ; ==> ADDR 314541
 | 0314535          PUSHL       023 000001         ;(a b c)
 | 0314538          JUMP        017 000010         ; ==> ADDR 314551
 | 0314541          PUSHT       026
 | 0314542          JNIL        015 000006         ; ==> ADDR 314551
 | 0314545          PUSHL       023 000002         ;(d e f)
 | 0314548          JUMP        017 000000         ; ==> ADDR 314551
 | 0314551          RETURN      027
 |
 | LITERALS REFERENCED
 | -------------------
 | [0000] = x
 | [0001] = (a b c)
 | [0002] = (d e f)
 */

/*
 | Disassemble a single instruction to the port fp. The instruction starts at 'ip' and may
 | reference literals from 'literal'. We dump the instruction on a line by itself with perhaps
 | a nice reference to the literal if it will fit. This is used by the budisassemble function
 | and also during run time trace debugging of the byte coded interpreter hence the reason for
 | which it is non static..
 */
char *bu_disassemble_1_inst(fp, ip, literal)
       FILE *fp; char *ip; struct conscell **literal;
{
       int kind, n, *cntr, lit, lit2, op = *ip; char *name;

       if (!bu_byop_lookup_instruction(op, &name, &n, &kind, &cntr)) {
           fprintf(fp, "%s  %-7.7s %-10.10s %03d\n", ip, " ", "***???***", op);
           return(ip + 1);
       }
       fprintf(fp, "%s  %-7.7s %-10.10s  %03d ", ip, " ", name, op);
       ip += 1;
       if (n > 0) {
           lit = XSHORT(ip);                                             /* they may be relative addresses or literal refs */
           fprintf(fp, "%06d ", lit);
           ip += 2;
       }
       if (n == 2) {
           lit2 = XSHORT(ip);                                            /* they may be relative addresses or literal refs */
           fprintf(fp, "%06d ", lit2);
           ip += 2;
       }
       switch(kind) {
              case OPK_BRANCH:                                           /* if we know it is a branch compute the target addr */
                   fprintf(fp,"        ; ADR => %s", ip + lit);          /* and print as a comment */
                   break;
              case OPK_LIT2:
                   if (flatsize(literal[lit2], 200) < 200) {
                       fprintf(fp," ; CLISP => ");                       /* if a literal ref that is fairly small then */
                       printlist(fp,literal[lit2],DELIM_ON,NULL,NULL);   /* also print it as a comment */
                   }   /* FALL IS THROUGH DELIBERATE HERE */
              case OPK_LIT1:
                   if (flatsize(literal[lit], 200) < 200) {
                       fprintf(fp,"        ; LIT  => ");                 /* if a literal ref that is fairly small then */
                       printlist(fp,literal[lit],DELIM_ON,NULL,NULL);    /* also print it as a comment */
                   }
                   break;
              case OPK_LITN1:
                   if (flatsize(literal[lit2], 200) < 200) {             /* LITN1 is the call format ie CALL <N> <LIT> */
                       fprintf(fp," ; CALL => ");                        /* if a literal ref that is fairly small then */
                       printlist(fp,literal[lit2],DELIM_ON,NULL,NULL);   /* also print it as a comment */
                   }
                   break;
       }
       fprintf(fp, "\n");
       return(ip);
}

/*
 | The actual (disassemble clisp [port]) primitive body.
 */
struct conscell *budisassemble(form)
       struct conscell *form;
{
       struct clispcell *clisp;
       struct conscell  **l;
       struct filecell  *port;
       FILE   *fp = stdout;
       char  *ip;
       int    n_lit, returninstr, n, kind;
       char   label[20];

      /*
       | Validate the input parameters expect a single clisp argument with an optional
       | port.
       */
       if (form == NULL) goto er;
       clisp = CLISP(form->carp);
       form = form->cdrp;
       if (!clisp || (clisp->celltype != CLISPCELL)) goto er;
       if (form) {
           port = PORT(form->carp);
           if (!port || (port->celltype != FILECELL) || form->cdrp) goto er;
           if (!(fp = port->atom)) goto er;
       }

      /*
       | Extract the number of literals in the array.
       */
       n_lit = (*((int *)clisp->literal - 1))/sizeof(struct conscell *);

      /*
       | Find out the opcode of the return instruction because it marks the end of the
       | a complete compiled code unit.
       */
       if (!bu_lookup_instruction("RETURN", &returninstr, &n, &kind)) goto er;

      /*
       | Prepopulate the label buffer so that as we generate the code we can dump a label if the given addr
       | is the target of a jump instruction.
       */
       label[0] = '\0';

      /*
       | Dump a nice header for the instruction dump.
       */
       fprintf(fp, "+--------------+\n");
       fprintf(fp, "| EVAL = %1d     |\n", clisp->eval);
       fprintf(fp, "+--------------+-------------------------------------------------------+\n");
       fprintf(fp, "| ADDR   LABEL   INSTR       BYTE CODE          COMMENTS               |\n");
       fprintf(fp, "+----------------------------------------------------------------------+\n");

      /*
       | Walk through the code byte by byte and dump the offset 'ip' the name of the instruction and
       | the code associated with it. Then dump each literal, once for each argument 'nargs'.
       */
       for(ip = clisp->code ; *ip && (*ip != returninstr) ; ) {
           TEST_BREAK();
           ip = bu_disassemble_1_inst(fp, ip, clisp->literal);
       }
       fprintf(fp, "%s  %-7.7s %-10.10s  %03d\n\n", ip, label, "RETURN", *ip);

      /*
       | Now dump the literal array associated with the above byte code.
       */
       fprintf(fp, "LITERALS REFERENCED\n");
       fprintf(fp, "-------------------\n");
       l = clisp->literal;
       for(n = 0; n < n_lit; l++, n++) {
           fprintf(fp,"[%04d] = ", n);
           prettyprint(*l, 10, 10, fp);
           if ((*l) && ((*l)->celltype == CLISPCELL))
                if (*l != LIST(clisp))
                    fprintf(fp, "   <--- recursive disassembly follows below (see LIT[%d])", n);
                else
                    fprintf(fp, "   <--- self");
           fprintf(fp,"\n");
       }

      /*
       | Now do recursive disassembly of any clispcells stored in the literals area of
       | the current clisp.
       */
       l = clisp->literal;
       fprintf(fp,"\n");
       for(n = 0; n < n_lit; l++, n++) {
           if ((*l) && ((*l)->celltype == CLISPCELL) && (*l != LIST(clisp))) {
               struct conscell c1, c2;
               c1.celltype = c2.celltype = CONSCELL;
               c1.cdrp = NULL;
               c1.carp = (*l);
               if (fp != stdout) {                /* add optional argument if not writing to stdout */
                   c2.carp = LIST(port);
                   c2.cdrp = NULL;
                   c1.cdrp = &c2;
               }
               fprintf(fp, "LIT[%d] = ", n);
               prettyprint(*l, 10, 10, fp);
               fprintf(fp,"\n");
               budisassemble(&c1);
               fprintf(fp,"\n");
           }
       }
       return(LIST(thold));
er:    ierror("disassemble");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
