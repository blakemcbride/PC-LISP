
/*****************************************************************************
 **        PC-LISP (C) 1986 Peter Ashwood-Smith                             **
 **        MODULE INSTALL                                                   **
 ** ----------------------------------------------------------------------- **
 ** This module has two routines, InstallSpecialAtoms and InstallBuiltIn -  **
 ** Functions. They do what there names imply. The Specail Atoms are added  **
 ** to the initially empty oblist and certain prebindings (global) are made.**
 ** The InstallBuiltInFunctions routine will install each of the buXYZ func **
 ** addresses in the atom which is to dispatch it. We set the discipline of **
 ** the built in functions at this time.                                    **
 *****************************************************************************/
#include <stdio.h>
#include "lisp.h"

extern struct conscell   *buREPsopen(struct conscell *form);
extern struct conscell   *buabs(struct conscell *form);
extern struct conscell   *buacos(struct conscell *form);
extern struct conscell   *buadd1(struct conscell *form);
extern struct conscell   *bualphp(struct conscell *form);
extern struct conscell   *buand(struct conscell *form);
extern struct conscell   *buappend(struct conscell *form);
extern struct conscell   *buarg(struct conscell *form);
extern struct conscell   *buargq(struct conscell *form);
extern struct conscell   *buarray(struct conscell *form);
extern struct conscell   *buarraydims(struct conscell *form);
extern struct conscell   *buarrayp(struct conscell *form);
extern struct conscell   *buascii(struct conscell *form);
extern struct conscell   *buasin(struct conscell *form);
extern struct conscell   *buassoc(struct conscell *form);
extern struct conscell   *buatan(struct conscell *form);
extern struct conscell   *buatom(struct conscell *form);
extern struct conscell   *buattach(struct conscell *form);
extern struct conscell   *bubaktrace(struct conscell *form);
extern struct conscell   *buboole(struct conscell *form);
extern struct conscell   *buboundp(struct conscell *form);
extern struct conscell   *bubread(struct conscell *form);
extern struct conscell   *bubwrite(struct conscell *form);
extern struct conscell   *bucar(struct conscell *form);
extern struct conscell   *bucaseq(struct conscell *form);
extern struct conscell   *bucatch(struct conscell *form);
extern struct conscell   *bucdr(struct conscell *form);
extern struct conscell   *buchix(struct conscell *form);
extern struct conscell   *buclmemusage(struct conscell *form);
extern struct conscell   *bucmdlna(struct conscell *form);
extern struct conscell   *buconcat(struct conscell *form);
extern struct conscell   *bucond(struct conscell *form);

extern struct conscell   *bucons(struct conscell *form);
extern struct conscell   *bucopy(struct conscell *form);
extern struct conscell   *bucopysymbol(struct conscell *form);
extern struct conscell   *bucos(struct conscell *form);
extern struct conscell   *bucxr(struct conscell *form);
extern struct conscell   *budeclare(struct conscell *form);
extern struct conscell   *budefine(struct conscell *form);
extern struct conscell   *budefun(struct conscell *form);
extern struct conscell   *budiff(struct conscell *form);
extern struct conscell   *budelete(struct conscell *form);
extern struct conscell   *budelq(struct conscell *form);
extern struct conscell   *budivide(struct conscell *form);
extern struct conscell   *budrain(struct conscell *form);
extern struct conscell   *budsubst(struct conscell *form);
extern struct conscell   *buerr(struct conscell *form);
extern struct conscell   *buerrset(struct conscell *form);
extern struct conscell   *bueq(struct conscell *form);
extern struct conscell   *buequal(struct conscell *form);
extern struct conscell   *buequals(struct conscell *form);
extern struct conscell   *buevenp(struct conscell *form);
extern struct conscell   *buexec(struct conscell *form);
extern struct conscell   *buexit(struct conscell *form);
extern struct conscell   *buexp(struct conscell *form);
extern struct conscell   *buexpandmemory(struct conscell *form);
extern struct conscell   *buexplode(struct conscell *form);
extern struct conscell   *buexpt(struct conscell *form);
extern struct conscell   *bufact(struct conscell *form);
extern struct conscell   *bufclose(struct conscell *form);
extern struct conscell   *bufilepos(struct conscell *form);
extern struct conscell   *bufilestat(struct conscell *form);

extern struct conscell   *bufillarray(struct conscell *form);
extern struct conscell   *bufix(struct conscell *form);
extern struct conscell   *bufixp(struct conscell *form);
extern struct conscell   *buflatc(struct conscell *form);
extern struct conscell   *buflatten(struct conscell *form);

extern struct conscell   *buflatsize(struct conscell *form);
extern struct conscell   *bufloat(struct conscell *form);
extern struct conscell   *bufloatp(struct conscell *form);
extern struct conscell   *bufopen(struct conscell *form);
extern struct conscell   *buforeach(struct conscell *form);
extern struct conscell   *bufprintf(struct conscell *form);
extern struct conscell   *bufscanf(struct conscell *form);
extern struct conscell   *bufuncall(struct conscell *form);
extern struct conscell   *bugc(struct conscell *form);
extern struct conscell   *bugensym(struct conscell *form);
extern struct conscell   *buget(struct conscell *form);
extern struct conscell   *bugetd(struct conscell *form);
extern struct conscell   *bugetdata(struct conscell *form);
extern struct conscell   *bugetenv(struct conscell *form);
extern struct conscell   *bugetlength(struct conscell *form);
extern struct conscell   *bugo(struct conscell *form);
extern struct conscell   *bugpname(struct conscell *form);
extern struct conscell   *bugreap(struct conscell *form);
extern struct conscell   *bugthan(struct conscell *form);
extern struct conscell   *buhashstat(struct conscell *form);
extern struct conscell   *buhsize(struct conscell *form);

extern struct conscell   *buhtolis(struct conscell *form);
extern struct conscell   *buhunk(struct conscell *form);
extern struct conscell   *buhunkp(struct conscell *form);
extern struct conscell   *buimplode(struct conscell *form);
extern struct conscell   *buintern(struct conscell *form);
extern struct conscell   *bulast(struct conscell *form);
extern struct conscell   *buldiff(struct conscell *form);
extern struct conscell   *bulength(struct conscell *form);
extern struct conscell   *bulessp(struct conscell *form);
extern struct conscell   *bulinenum(struct conscell *form);
extern struct conscell   *bulist(struct conscell *form);
extern struct conscell   *bulistarray(struct conscell *form);
extern struct conscell   *bulistify(struct conscell *form);
extern struct conscell   *bulistp(struct conscell *form);
extern struct conscell   *buload(struct conscell *form);
extern struct conscell   *bulog(struct conscell *form);
extern struct conscell   *bulog10(struct conscell *form);
extern struct conscell   *bulsh(struct conscell *form);
extern struct conscell   *bulthan(struct conscell *form);
extern struct conscell   *bumacroexpand(struct conscell *form);
extern struct conscell   *bumakhunk(struct conscell *form);
extern struct conscell   *bumaknam(struct conscell *form);
extern struct conscell   *bumakunb(struct conscell *form);
extern struct conscell   *bumap(struct conscell *form);
extern struct conscell   *bumapc(struct conscell *form);
extern struct conscell   *bumapcar(struct conscell *form);
extern struct conscell   *bumaplist(struct conscell *form);

extern struct conscell   *bumax(struct conscell *form);
extern struct conscell   *bumember(struct conscell *form);
extern struct conscell   *bumemq(struct conscell *form);
extern struct conscell   *bumemstat(struct conscell *form);
extern struct conscell   *bumemusage(struct conscell *form);
extern struct conscell   *bumin(struct conscell *form);
extern struct conscell   *buminus(struct conscell *form);
extern struct conscell   *buminusp(struct conscell *form);
extern struct conscell   *bumod(struct conscell *form);
extern struct conscell   *bunconc(struct conscell *form);
extern struct conscell   *bunexplode(struct conscell *form);
extern struct conscell   *bunot(struct conscell *form);
extern struct conscell   *bunth(struct conscell *form);
extern struct conscell   *bunthcdr(struct conscell *form);
extern struct conscell   *bunthchar(struct conscell *form);
extern struct conscell   *bunull(struct conscell *form);
extern struct conscell   *bunumbp(struct conscell *form);
extern struct conscell   *buoblist(struct conscell *form);
extern struct conscell   *buoddp(struct conscell *form);
extern struct conscell   *buoneminus(struct conscell *form);
extern struct conscell   *buoneplus(struct conscell *form);

extern struct conscell   *buor(struct conscell *form);
extern struct conscell   *bupairlis(struct conscell *form);
extern struct conscell   *buparsetq(struct conscell *form);
extern struct conscell   *bupatom(struct conscell *form);
extern struct conscell   *buphoptimize(struct conscell *form);
extern struct conscell   *buplist(struct conscell *form);
extern struct conscell   *buplus(struct conscell *form);
extern struct conscell   *buplusp(struct conscell *form);
extern struct conscell   *buportp(struct conscell *form);
extern struct conscell   *buppform(struct conscell *form);
extern struct conscell   *buprinc(struct conscell *form);
extern struct conscell   *buprint(struct conscell *form);
extern struct conscell   *buprintstack(struct conscell *form);
extern struct conscell   *buprintf(struct conscell *form);
extern struct conscell   *buprocess(struct conscell *form);
extern struct conscell   *buproduct(struct conscell *form);
extern struct conscell   *buprog(struct conscell *form);
extern struct conscell   *buput(struct conscell *form);
extern struct conscell   *buputd(struct conscell *form);
extern struct conscell   *buquote(struct conscell *form);
extern struct conscell   *buquotient(struct conscell *form);
extern struct conscell   *burandom(struct conscell *form);
extern struct conscell   *buread(struct conscell *form);
extern struct conscell   *bureadc(struct conscell *form);
extern struct conscell   *bureadli(struct conscell *form);
extern struct conscell   *bureadln(struct conscell *form);
extern struct conscell   *bureadst(struct conscell *form);
extern struct conscell   *buremob(struct conscell *form);
extern struct conscell   *buremq(struct conscell *form);
extern struct conscell   *buremove(struct conscell *form);
extern struct conscell   *buremprop(struct conscell *form);
extern struct conscell   *burepeat(struct conscell *form);
extern struct conscell   *buresetio(struct conscell *form);

extern struct conscell   *bureturn(struct conscell *form);
extern struct conscell   *bureverse(struct conscell *form);
extern struct conscell   *bunreverse(struct conscell *form);
extern struct conscell   *burplaca(struct conscell *form);
extern struct conscell   *burplacd(struct conscell *form);
extern struct conscell   *burplacx(struct conscell *form);
extern struct conscell   *buscanf(struct conscell *form);
extern struct conscell   *buselect(struct conscell *form);
extern struct conscell   *buset(struct conscell *form);
extern struct conscell   *busetarg(struct conscell *form);
extern struct conscell   *busetplist(struct conscell *form);
extern struct conscell   *busetq(struct conscell *form);

extern struct conscell   *busetcreate(struct conscell *form);
extern struct conscell   *busetlist(struct conscell *form);
extern struct conscell   *busetand(struct conscell *form);
extern struct conscell   *busetor(struct conscell *form);
extern struct conscell   *busetdiff(struct conscell *form);
extern struct conscell   *busetmember(struct conscell *form);
extern struct conscell   *busymtcreate(struct conscell *form);
extern struct conscell   *busymtlist(struct conscell *form);
extern struct conscell   *busymtmember(struct conscell *form);
extern struct conscell   *busymtadd(struct conscell *form);
extern struct conscell   *busymtremove(struct conscell *form);
extern struct conscell   *busymtsize(struct conscell *form);
extern struct conscell   *busetsyntax(struct conscell *form);
extern struct conscell   *bushowstack(struct conscell *form);
extern struct conscell   *busin(struct conscell *form);
extern struct conscell   *busizeof(struct conscell *form);
extern struct conscell   *busleep(struct conscell *form);
extern struct conscell   *busort(struct conscell *form);
extern struct conscell   *busortcar(struct conscell *form);
extern struct conscell   *busopen(struct conscell *form);
extern struct conscell   *busprintf(struct conscell *form);
extern struct conscell   *busqrt(struct conscell *form);
extern struct conscell   *bustringp(struct conscell *form);
extern struct conscell   *bustrcomp(struct conscell *form);
extern struct conscell   *bustrlen(struct conscell *form);
extern struct conscell   *bustrpad(struct conscell *form);
extern struct conscell   *bustrtrim(struct conscell *form);
extern struct conscell   *bustrsetpat(struct conscell *form);
extern struct conscell   *bustrfndpat(struct conscell *form);
extern struct conscell   *busubst(struct conscell *form);
extern struct conscell   *busstatus(struct conscell *form);
extern struct conscell   *busub1(struct conscell *form);
extern struct conscell   *busubstring(struct conscell *form);
extern struct conscell   *busum(struct conscell *form);
extern struct conscell   *busystime(struct conscell *form);

extern struct conscell   *busysunlink(struct conscell *form);
extern struct conscell   *buthrow(struct conscell *form);
extern struct conscell   *butimes(struct conscell *form);
extern struct conscell   *butimestring(struct conscell *form);
extern struct conscell   *butildeexpand(struct conscell *form);
extern struct conscell   *butoupper(struct conscell *form);
extern struct conscell   *butolower(struct conscell *form);
extern struct conscell   *butrace(struct conscell *form);
extern struct conscell   *butruename(struct conscell *form);
extern struct conscell   *butype(struct conscell *form);
extern struct conscell   *buuapply(struct conscell *form);
extern struct conscell   *buuconcat(struct conscell *form);
extern struct conscell   *buueval(struct conscell *form);
extern struct conscell   *buuntrace(struct conscell *form);
extern struct conscell   *buwhile(struct conscell *form);
extern struct conscell   *buzapline(struct conscell *form);
extern struct conscell   *buzerop(struct conscell *form);

extern struct conscell   *bucompile(struct conscell *form);
extern struct conscell   *buassemble(struct conscell *form);
extern struct conscell   *budisassemble(struct conscell *form);
extern struct conscell   *butimeev(struct conscell *form);

#if    GRAPHICSAVAILABLE                        /* MSDOS bios functions */
extern struct conscell  *buscrmde(struct conscell *form);
extern struct conscell  *buscrscp(struct conscell *form);
extern struct conscell  *buscrspt(struct conscell *form);
extern struct conscell  *buscrsct(struct conscell *form);
extern struct conscell  *buscrsap(struct conscell *form);
extern struct conscell  *buscrwdot(struct conscell *form);
extern struct conscell  *buscrline(struct conscell *form);
#endif

/****************************************************************************
 ** The interpreter needs certain 'built' in atoms to be predefined prior  **
 ** to the running of the eval function. Pointers to some of these atoms   **
 ** are held to make testing faster. For example to check if an atom is    **
 ** 't', 'lambda', 'quote' are frequent operations and by holding the ptr  **
 ** we save much loopup time in the core of the eval function() this is    **
 ** a major speed up factor.                                               **
 ****************************************************************************/
 struct alphacell *auxhold;                 /* "&aux" atom */
 struct alphacell *autoloadhold;            /* "autoload" atom */
 struct alphacell *blexprhold;              /* his binding is lexpr arg list */
 struct alphacell *catchstkhold;            /* stack of catch environments */
 struct alphacell *errstkhold;              /* stack of errset environments */
 struct alphacell *gohold;                  /* "go" atom */
 struct alphacell *labelhold;               /* "label" atom */
 struct alphacell *lambdahold;              /* "lambda" atom */
 struct alphacell *lexprhold;               /* "lexpr" atom */
 struct alphacell *macrohold;               /* "macro" atom */
 struct alphacell *macroporthold;           /* his binding is macro read port */
 struct alphacell *nilhold;                 /* dummy not used at moment */
 struct alphacell *nlambdahold;             /* "nlambda" atom */
 struct alphacell *optionalhold;            /* "&optional" atom */
 struct filecell  *piporthold;              /* "piport" atom (stdin) */
 struct filecell  *poporthold;              /* "poport" atom (stdout) */
 struct alphacell *quotehold;               /* "quote" atom */
 struct alphacell *resthold;                /* "&rest" atom */
 struct alphacell *returnhold;              /* "return" atom */
 struct alphacell *thold;                   /* "t" atom */

/*****************************************************************************
 ** Install Special Atoms -  will set all of the XYZhold global atom holding**
 ** pointers. For example thold points to the atom with printname "t" etc.  **
 ** in addition atoms which must be prebound are done here. We prebind the  **
 ** "$gcprint" etc atoms to their default values. All of these atoms are    **
 ** given the attribute 'PERM' meaning that they will not dissappear in GC  **
 ** if they are not referenced. This is different from the NOT_PERM attrib- **
 ** ute that most installed atoms are given by the read function. We also   **
 ** bind the piport, poport, and errport to their correct stdin,stdout and  **
 ** stderr values.                                                          **
 *****************************************************************************/
void InstallSpecialAtoms()
{      struct alphacell *temp;
       thold        = insertatom("t",PERM);             /* holds are in PERM */
       nilhold      = insertatom("nil",PERM);           /* space so not GC'D */
       lambdahold   = insertatom("lambda",PERM);        /* want these holdvar*/
       nlambdahold  = insertatom("nlambda",PERM);       /* s to be always ok */
       macrohold    = insertatom("macro",PERM);
       quotehold    = insertatom("quote",PERM);
       labelhold    = insertatom("label",PERM);
       resthold     = insertatom("&rest",PERM);
       auxhold      = insertatom("&aux",PERM);
       optionalhold = insertatom("&optional",PERM);
       gohold       = insertatom("$[|go|]$",PERM);
       returnhold   = insertatom("$[|return|]$",PERM);
       catchstkhold = insertatom("$[|catchstack|]$",PERM);
       errstkhold   = insertatom("$[|errsetstack|]$",PERM);
       autoloadhold = insertatom("autoload",PERM);
       bindvar(LIST(thold),LIST(thold));                            /* eval(t) == t     */
       thold->botvaris = GLOBALVAR;                     /* GLOBALLY         */
       bindvar(LIST(nilhold),NULL);                           /* eval(nil) == ()  */
       nilhold->botvaris = GLOBALVAR;                   /* GLOBALLY         */
       temp = insertatom("$gccount$",PERM);             /* set up system vars */
       temp->botvaris = GLOBALVAR;                      /* and bind to defaults */
       bindvar(LIST(temp),newintop(0L));                      /* all bindings are global */
       temp = insertatom("$ldprint",PERM);
       temp->botvaris = GLOBALVAR;                      /* shallow binding */
       bindvar(LIST(temp),LIST(thold));
       temp = insertatom("$gcprint",PERM);
       temp->botvaris = GLOBALVAR;
       bindvar(LIST(temp),NULL);
       temp = insertatom("poport",PERM);
       bindvar(LIST(temp),LIST(poporthold = PORT(MakePort(stdout,insertatom("stdout",PERM)))));
       temp->botvaris = GLOBALVAR;
       temp = insertatom("piport",PERM);
       bindvar(LIST(temp),LIST(piporthold = PORT(MakePort(stdin,insertatom("stdin",PERM)))));
       temp->botvaris = GLOBALVAR;
       temp = insertatom("errport",PERM);
       bindvar(LIST(temp),MakePort(stderr,insertatom("stderr",PERM)));
       temp->botvaris = GLOBALVAR;
       macroporthold = insertatom("|$[macroreadport]$|",PERM);
       temp = insertatom("displace-macros",PERM);
       bindvar(LIST(temp),NULL);
       temp->botvaris = GLOBALVAR;
       lexprhold  = insertatom("lexpr",PERM);
       blexprhold = insertatom("|$[lexprargs]$|",PERM);
}

/*****************************************************************************
 ** Install Built In Functions - will call funcinstall to put the machine   **
 ** address of each of the built in functions in its atom 'func' field. The **
 ** routine funcinstall is used to do this. The parameters to funcinstall   **
 ** are the functions discipline, BUEVAL is a normal nlambda expression, ie **
 ** the args are evaluated before the call and as many args as you want can **
 ** be provided. The second group BUNEVAL is an fexpr. It can take as many  **
 ** args as you want but they are not evaluated before the call. The next   **
 ** parameter to funcinstall is the address of the function. Then comes the **
 ** printname of the atom which will hold this pointer, followed by NULL.   **
 ** This parameter (NULL) is used if you have an atom pointer to the atom   **
 ** where the functions is to be installed. This speeds up the installation **
 ** of user defined functions because we know the atom before the call.     **
 ** Naturally the graphics oriented routines are only installed if we have  **
 ** a graphics capability.                                                  **
 ** ----------------------------------------------------------------------- **
 ** NOTE:  Current Built In Function Count = 171  (170 + cadadar class)     **
 *****************************************************************************/
void InstallBuiltInFunctions()
{
       funcinstall(FN_BUEVAL,buREPsopen ,"REP-socketopen",NULL);
       funcinstall(FN_BUEVAL,buabs ,"abs",NULL);
       funcinstall(FN_BUEVAL,buacos,"acos",NULL);
       funcinstall(FN_BUEVAL,buadd1,"add1",NULL);
       funcinstall(FN_BUEVAL,bualphp,"alphalessp",NULL);
       funcinstall(FN_BUEVAL,buappend,"append",NULL);
       funcinstall(FN_BUEVAL,buarg,"arg",NULL);
       funcinstall(FN_BUEVAL,buargq,"arg?",NULL);
       funcinstall(FN_BUEVAL,buarraydims,"arraydims",NULL);
       funcinstall(FN_BUEVAL,buarrayp,"arrayp",NULL);
       funcinstall(FN_BUEVAL,buascii,"ascii",NULL);
       funcinstall(FN_BUEVAL,buasin,"asin",NULL);
       funcinstall(FN_BUEVAL,buassoc,"assoc",NULL);
       funcinstall(FN_BUEVAL,buatan,"atan",NULL);
       funcinstall(FN_BUEVAL,buatom,"atom",NULL);
       funcinstall(FN_BUEVAL,buattach,"attach",NULL);
       funcinstall(FN_BUEVAL,bubaktrace,"baktrace",NULL);
       funcinstall(FN_BUEVAL,bubread,"b-read",NULL);
       funcinstall(FN_BUEVAL,bubwrite,"b-write",NULL);
       funcinstall(FN_BUEVAL,buboole,"boole",NULL);
       funcinstall(FN_BUEVAL,buboundp,"boundp",NULL);
       funcinstall(FN_BUEVAL,bucar,"car",NULL);
       funcinstall(FN_BUEVAL,bucdr,"cdr",NULL);
       funcinstall(FN_BUEVAL,buchix,"character-index",NULL);
       funcinstall(FN_BUEVAL,buclmemusage,"clisp-memusage",NULL);
       funcinstall(FN_BUEVAL,bucmdlna,"command-line-args",NULL);
       funcinstall(FN_BUEVAL,buconcat,"concat",NULL);
       funcinstall(FN_BUEVAL,bucons,"cons",NULL);
       funcinstall(FN_BUEVAL,bucopy,"copy",NULL);
       funcinstall(FN_BUEVAL,bucopysymbol,"copysymbol",NULL);
       funcinstall(FN_BUEVAL,bucos ,"cos",NULL);

       funcinstall(FN_BUEVAL,bucxr,"cxr",NULL);
       funcinstall(FN_BUEVAL,budelq,"delq",NULL);
       funcinstall(FN_BUEVAL,budelete,"delete",NULL);
       funcinstall(FN_BUEVAL,budiff,"diff",NULL);
       funcinstall(FN_BUEVAL,budiff,"difference",NULL);
       funcinstall(FN_BUEVAL,budivide,"/",NULL);
       funcinstall(FN_BUEVAL,budrain,"drain",NULL);
       funcinstall(FN_BUEVAL,budsubst,"dsubst",NULL);
       funcinstall(FN_BUEVAL,buerr,"err",NULL);
       funcinstall(FN_BUEVAL,bueq,"eq",NULL);
       funcinstall(FN_BUEVAL,buequal,"equal",NULL);
       funcinstall(FN_BUEVAL,buequals,"=",NULL);
       funcinstall(FN_BUEVAL,buevenp,"evenp",NULL);
       funcinstall(FN_BUEVAL,buexit,"exit",NULL);
       funcinstall(FN_BUEVAL,buexp ,"exp",NULL);
       funcinstall(FN_BUEVAL,buexpandmemory,"expand-memory",NULL);
       funcinstall(FN_BUEVAL,buexplode,"explode",NULL);
       funcinstall(FN_BUEVAL,buexpt,"expt",NULL);
       funcinstall(FN_BUEVAL,bufact,"fact",NULL);
       funcinstall(FN_BUEVAL,bufclose,"close",NULL);
       funcinstall(FN_BUEVAL,bufilepos,"filepos",NULL);
       funcinstall(FN_BUEVAL,bufilestat,"filestat",NULL);
       funcinstall(FN_BUEVAL,bufillarray,"fillarray",NULL);
       funcinstall(FN_BUEVAL,bufix,"fix",NULL);
       funcinstall(FN_BUEVAL,bufixp,"fixp",NULL);
       funcinstall(FN_BUEVAL,buflatc,"flatc",NULL);
       funcinstall(FN_BUEVAL,buflatten,"flatten",NULL);
       funcinstall(FN_BUEVAL,buflatsize,"flatsize",NULL);
       funcinstall(FN_BUEVAL,bufloat,"float",NULL);
       funcinstall(FN_BUEVAL,bufloatp,"floatp",NULL);

       funcinstall(FN_BUEVAL,bufopen,"fileopen",NULL);
       funcinstall(FN_BUEVAL,bufprintf,"fprintf",NULL);
       funcinstall(FN_BUEVAL,bufscanf,"fscanf",NULL);
       funcinstall(FN_BUEVAL,bufuncall,"funcall",NULL);
       funcinstall(FN_BUEVAL,bugc,"gc",NULL);
       funcinstall(FN_BUEVAL,bugensym,"gensym",NULL);
       funcinstall(FN_BUEVAL,buget,"get",NULL);
       funcinstall(FN_BUEVAL,bugetd,"getd",NULL);
       funcinstall(FN_BUEVAL,bugetdata,"getdata",NULL);
       funcinstall(FN_BUEVAL,bugetenv,"getenv",NULL);
       funcinstall(FN_BUEVAL,bugetlength,"getlength",NULL);
       funcinstall(FN_BUEVAL,bugpname,"get_pname",NULL);
       funcinstall(FN_BUEVAL,bugreap,"greaterp",NULL);
       funcinstall(FN_BUEVAL,bugthan,">",NULL);
       funcinstall(FN_BUEVAL,buhashstat,"hashtabstat",NULL);
       funcinstall(FN_BUEVAL,buhsize,"hunksize",NULL);
       funcinstall(FN_BUEVAL,buhtolis,"hunk-to-list",NULL);
       funcinstall(FN_BUEVAL,buhunk,"hunk",NULL);
       funcinstall(FN_BUEVAL,buhunkp,"hunkp",NULL);
       funcinstall(FN_BUEVAL,buimplode,"implode",NULL);
       funcinstall(FN_BUEVAL,buintern,"intern",NULL);
       funcinstall(FN_BUEVAL,bulast,"last",NULL);
       funcinstall(FN_BUEVAL,buldiff,"ldiff",NULL);
       funcinstall(FN_BUEVAL,bulength,"length",NULL);
       funcinstall(FN_BUEVAL,bulessp,"lessp",NULL);
       funcinstall(FN_BUEVAL,bulinenum,"line-num",NULL);
       funcinstall(FN_BUEVAL,bulist,"list",NULL);
       funcinstall(FN_BUEVAL,bulistarray,"listarray",NULL);
       funcinstall(FN_BUEVAL,bulistify,"listify",NULL);
       funcinstall(FN_BUEVAL,bulistp,"listp",NULL);
       funcinstall(FN_BUEVAL,buload,"load",NULL);
       funcinstall(FN_BUEVAL,buload,"include",NULL);  /* include & load are same to interpreter */
       funcinstall(FN_BUEVAL,bulog ,"log",NULL);
       funcinstall(FN_BUEVAL,bulog10,"log10",NULL);
       funcinstall(FN_BUEVAL,bulsh,"lsh",NULL);
       funcinstall(FN_BUEVAL,bulthan,"<",NULL);
       funcinstall(FN_BUEVAL,bumacroexpand,"macroexpand",NULL);
       funcinstall(FN_BUEVAL,bumakhunk,"makhunk",NULL);
       funcinstall(FN_BUEVAL,bumaknam,"maknam",NULL);
       funcinstall(FN_BUEVAL,bumakunb,"makunbound",NULL);
       funcinstall(FN_BUEVAL,bumap,"map",NULL);
       funcinstall(FN_BUEVAL,bumapc,"mapc",NULL);
       funcinstall(FN_BUEVAL,bumapcar,"mapcar",NULL);
       funcinstall(FN_BUEVAL,bumaplist,"maplist",NULL);
       funcinstall(FN_BUEVAL,bumax,"max",NULL);
       funcinstall(FN_BUEVAL,bumember,"member",NULL);
       funcinstall(FN_BUEVAL,bumemq,"memq",NULL);
       funcinstall(FN_BUEVAL,bumemstat,"memstat",NULL);
       funcinstall(FN_BUEVAL,bumemusage,"memusage",NULL);
       funcinstall(FN_BUEVAL,bumin,"min",NULL);
       funcinstall(FN_BUEVAL,buminus,"-",NULL);
       funcinstall(FN_BUEVAL,buminusp,"minusp",NULL);
       funcinstall(FN_BUEVAL,bumod,"mod",NULL);

       funcinstall(FN_BUEVAL,bunconc,"nconc",NULL);
       funcinstall(FN_BUEVAL,bunexplode,"exploden",NULL);
       funcinstall(FN_BUEVAL,bunot,"not",NULL);
       funcinstall(FN_BUEVAL,bunth,"nth",NULL);
       funcinstall(FN_BUEVAL,bunthcdr,"nthcdr",NULL);
       funcinstall(FN_BUEVAL,bunthchar,"nthchar",NULL);
       funcinstall(FN_BUEVAL,bunull,"null",NULL);
       funcinstall(FN_BUEVAL,bunumbp,"numberp",NULL);
       funcinstall(FN_BUEVAL,bunumbp,"numbp",NULL);
       funcinstall(FN_BUEVAL,buoblist,"oblist",NULL);
       funcinstall(FN_BUEVAL,buoddp,"oddp",NULL);
       funcinstall(FN_BUEVAL,buoneminus,"1-",NULL);
       funcinstall(FN_BUEVAL,buoneplus,"1+",NULL);
       funcinstall(FN_BUEVAL,bupairlis,"pairlis",NULL);
       funcinstall(FN_BUEVAL,bupatom,"patom",NULL);
       funcinstall(FN_BUEVAL,buphoptimize,"ph-optimize",NULL);
       funcinstall(FN_BUEVAL,buplist,"plist",NULL);
       funcinstall(FN_BUEVAL,buplus,"+",NULL);
       funcinstall(FN_BUEVAL,buplusp,"plusp",NULL);
       funcinstall(FN_BUEVAL,buportp,"portp",NULL);
       funcinstall(FN_BUEVAL,buppform,"pp-form",NULL);
       funcinstall(FN_BUEVAL,buprinc,"princ",NULL);
       funcinstall(FN_BUEVAL,buprint,"print",NULL);
       funcinstall(FN_BUEVAL,buprintstack,"printstack",NULL);
       funcinstall(FN_BUEVAL,buprintf,"printf",NULL);
       funcinstall(FN_BUEVAL,buprocess,"*process",NULL);
       funcinstall(FN_BUEVAL,buproduct,"product",NULL);

       funcinstall(FN_BUEVAL,buproduct,"times",NULL);
       funcinstall(FN_BUEVAL,buput,"putprop",NULL);
       funcinstall(FN_BUEVAL,buputd,"putd",NULL);
       funcinstall(FN_BUEVAL,buquotient,"quotient",NULL);
       funcinstall(FN_BUEVAL,burandom,"random",NULL);
       funcinstall(FN_BUEVAL,buread,"read",NULL);
       funcinstall(FN_BUEVAL,bureadc,"readc",NULL);
       funcinstall(FN_BUEVAL,bureadli,"readlist",NULL);
       funcinstall(FN_BUEVAL,bureadln,"readln",NULL);
       funcinstall(FN_BUEVAL,bureadst,"readstr",NULL);
       funcinstall(FN_BUEVAL,buremob,"remob",NULL);
       funcinstall(FN_BUEVAL,buremove,"remove",NULL);
       funcinstall(FN_BUEVAL,buremq,"remq",NULL);
       funcinstall(FN_BUEVAL,buremprop,"remprop",NULL);
       funcinstall(FN_BUEVAL,buresetio,"resetio",NULL);
       funcinstall(FN_BUEVAL,bureturn,"return",NULL);
       funcinstall(FN_BUEVAL,bureverse,"reverse",NULL);
       funcinstall(FN_BUEVAL,bunreverse,"nreverse",NULL);
       funcinstall(FN_BUEVAL,burplaca,"rplaca",NULL);
       funcinstall(FN_BUEVAL,burplacd,"rplacd",NULL);
       funcinstall(FN_BUEVAL,burplacx,"rplacx",NULL);
       funcinstall(FN_BUEVAL,buscanf,"scanf",NULL);
       funcinstall(FN_BUEVAL,buselect,"*select",NULL);
       funcinstall(FN_BUEVAL,buset,"set",NULL);
       funcinstall(FN_BUEVAL,busetcreate,"set-create",NULL);
       funcinstall(FN_BUEVAL,busetlist,"set-list",NULL);
       funcinstall(FN_BUEVAL,busetand,"set-and",NULL);
       funcinstall(FN_BUEVAL,busetor,"set-or",NULL);
       funcinstall(FN_BUEVAL,busetdiff,"set-diff",NULL);
       funcinstall(FN_BUEVAL,busetmember,"set-member",NULL);
       funcinstall(FN_BUEVAL,busopen,"socketopen",NULL);
       funcinstall(FN_BUEVAL,busymtcreate,"symtab-create",NULL);
       funcinstall(FN_BUEVAL,busymtlist,"symtab-list",NULL);
       funcinstall(FN_BUEVAL,busymtadd,"symtab-add",NULL);
       funcinstall(FN_BUEVAL,busymtremove,"symtab-remove",NULL);
       funcinstall(FN_BUEVAL,busymtsize,"symtab-size",NULL);
       funcinstall(FN_BUEVAL,busymtmember,"symtab-member",NULL);

       funcinstall(FN_BUEVAL,busetarg,"setarg",NULL);
       funcinstall(FN_BUEVAL,busetplist,"setplist",NULL);
       funcinstall(FN_BUEVAL,busetsyntax,"setsyntax",NULL);
       funcinstall(FN_BUEVAL,bushowstack,"showstack",NULL);
       funcinstall(FN_BUEVAL,busin ,"sin",NULL);
       funcinstall(FN_BUEVAL,busizeof,"sizeof",NULL);
       funcinstall(FN_BUEVAL,busleep,"sleep",NULL);
       funcinstall(FN_BUEVAL,busort,"sort",NULL);
       funcinstall(FN_BUEVAL,busortcar,"sortcar",NULL);
       funcinstall(FN_BUEVAL,busprintf,"sprintf",NULL);
       funcinstall(FN_BUEVAL,busqrt,"sqrt",NULL);
       funcinstall(FN_BUEVAL,bustringp,"stringp",NULL);
       funcinstall(FN_BUEVAL,bustrcomp,"strcomp",NULL);
       funcinstall(FN_BUEVAL,bustrlen,"strlen",NULL);
       funcinstall(FN_BUEVAL,bustrpad,"strpad",NULL);
       funcinstall(FN_BUEVAL,bustrtrim,"strtrim",NULL);
       funcinstall(FN_BUEVAL,bustrsetpat,"strsetpat", NULL);
       funcinstall(FN_BUEVAL,bustrfndpat,"strfndpat", NULL);
       funcinstall(FN_BUEVAL,busubst,"subst",NULL);
       funcinstall(FN_BUEVAL,busub1,"sub1",NULL);
       funcinstall(FN_BUEVAL,busubstring,"substring",NULL);
       funcinstall(FN_BUEVAL,busum,"add",NULL);
       funcinstall(FN_BUEVAL,busum,"plus",NULL);
       funcinstall(FN_BUEVAL,busum,"sum",NULL);
       funcinstall(FN_BUEVAL,busystime,"sys:time",NULL);
       funcinstall(FN_BUEVAL,busysunlink,"sys:unlink",NULL);
       funcinstall(FN_BUEVAL,buthrow,"throw",NULL);
       funcinstall(FN_BUEVAL,butimes,"*",NULL);
       funcinstall(FN_BUEVAL,butimestring,"time-string",NULL);
       funcinstall(FN_BUEVAL,butildeexpand,"tilde-expand",NULL);
       funcinstall(FN_BUEVAL,butoupper,"toupper",NULL);
       funcinstall(FN_BUEVAL,butolower,"tolower",NULL);
       funcinstall(FN_BUEVAL,butruename,"truename",NULL);
       funcinstall(FN_BUEVAL,butype,"type",NULL);
       funcinstall(FN_BUEVAL,buuapply,"apply",NULL);
       funcinstall(FN_BUEVAL,buuconcat,"uconcat",NULL);
       funcinstall(FN_BUEVAL,buueval,"eval",NULL);
       funcinstall(FN_BUEVAL,buzapline,"zapline",NULL);
       funcinstall(FN_BUEVAL,buzerop,"zerop",NULL);

       funcinstall(FN_BUEVAL,bucompile, "compile", NULL);
       funcinstall(FN_BUEVAL,buassemble,"assemble",NULL);
       funcinstall(FN_BUEVAL,budisassemble,"disassemble",NULL);
       funcinstall(FN_BUNEVAL,butimeev, "time-eval", NULL);

                                                       /* non eval args func */
       funcinstall(FN_BUNEVAL,buand,"and",NULL);
       funcinstall(FN_BUNEVAL,buarray,"array",NULL);
       funcinstall(FN_BUNEVAL,bucaseq,"caseq",NULL);
       funcinstall(FN_BUNEVAL,bucatch,"catch",NULL);
       funcinstall(FN_BUNEVAL,bucond,"cond",NULL);
       funcinstall(FN_BUNEVAL,budeclare,"declare",NULL);
       funcinstall(FN_BUNEVAL,budefine,"def",NULL);
       funcinstall(FN_BUNEVAL,budefun,"defun",NULL);
       funcinstall(FN_BUNEVAL,buerrset,"errset",NULL);
       funcinstall(FN_BUNEVAL,buexec,"exec",NULL);
       funcinstall(FN_BUNEVAL,buforeach,"foreach",NULL);
       funcinstall(FN_BUNEVAL,bugo,"go",NULL);

       funcinstall(FN_BUNEVAL,buor,"or",NULL);
       funcinstall(FN_BUNEVAL,buparsetq,"PAR-setq",NULL);
       funcinstall(FN_BUNEVAL,buprog,"prog",NULL);
       funcinstall(FN_BUNEVAL,buquote,"quote",NULL);
       funcinstall(FN_BUNEVAL,burepeat,"repeat",NULL);
       funcinstall(FN_BUNEVAL,busetq,"setq",NULL);
       funcinstall(FN_BUNEVAL,busstatus,"sstatus",NULL);
       funcinstall(FN_BUNEVAL,butrace,"trace",NULL);
       funcinstall(FN_BUNEVAL,buuntrace,"untrace",NULL);
       funcinstall(FN_BUNEVAL,buwhile,"while",NULL);

#if    GRAPHICSAVAILABLE
       funcinstall(FN_BUEVAL,buscrmde,"#scrmde#",NULL);
       funcinstall(FN_BUEVAL,buscrscp,"#scrscp#",NULL);
       funcinstall(FN_BUEVAL,buscrspt,"#scrspt#",NULL);
       funcinstall(FN_BUEVAL,buscrsct,"#scrsct#",NULL);
       funcinstall(FN_BUEVAL,buscrsap,"#scrsap#",NULL);
       funcinstall(FN_BUEVAL,buscrwdot,"#scrwdot#",NULL);
       funcinstall(FN_BUEVAL,buscrline,"#scrline#",NULL);
#endif
}

