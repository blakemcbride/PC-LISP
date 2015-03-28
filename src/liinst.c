
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

extern struct conscell   *buREPsopen();
extern struct conscell   *buabs();
extern struct conscell   *buacos();
extern struct conscell   *buadd1();
extern struct conscell   *bualphp();
extern struct conscell   *buand();
extern struct conscell   *buappend();
extern struct conscell   *buarg();
extern struct conscell   *buargq();
extern struct conscell   *buarray();
extern struct conscell   *buarraydims();
extern struct conscell   *buarrayp();
extern struct conscell   *buascii();
extern struct conscell   *buasin();
extern struct conscell   *buassoc();
extern struct conscell   *buatan();
extern struct conscell   *buatom();
extern struct conscell   *buattach();
extern struct conscell   *bubaktrace();
extern struct conscell   *buboole();
extern struct conscell   *buboundp();
extern struct conscell   *bubread();
extern struct conscell   *bubwrite();
extern struct conscell   *bucar();
extern struct conscell   *bucaseq();
extern struct conscell   *bucatch();
extern struct conscell   *bucdr();
extern struct conscell   *buchix();
extern struct conscell   *buclmemusage();
extern struct conscell   *bucmdlna();
extern struct conscell   *buconcat();
extern struct conscell   *bucond();

extern struct conscell   *bucons();
extern struct conscell   *bucopy();
extern struct conscell   *bucopysymbol();
extern struct conscell   *bucos();
extern struct conscell   *bucxr();
extern struct conscell   *budeclare();
extern struct conscell   *budefine();
extern struct conscell   *budefun();
extern struct conscell   *budiff();
extern struct conscell   *budelete();
extern struct conscell   *budelq();
extern struct conscell   *budivide();
extern struct conscell   *budrain();
extern struct conscell   *budsubst();
extern struct conscell   *buerr();
extern struct conscell   *buerrset();
extern struct conscell   *bueq();
extern struct conscell   *buequal();
extern struct conscell   *buequals();
extern struct conscell   *buevenp();
extern struct conscell   *buexec();
extern struct conscell   *buexit();
extern struct conscell   *buexp();
extern struct conscell   *buexpandmemory();
extern struct conscell   *buexplode();
extern struct conscell   *buexpt();
extern struct conscell   *bufact();
extern struct conscell   *bufclose();
extern struct conscell   *bufilepos();
extern struct conscell   *bufilestat();

extern struct conscell   *bufillarray();
extern struct conscell   *bufix();
extern struct conscell   *bufixp();
extern struct conscell   *buflatc();
extern struct conscell   *buflatten();

extern struct conscell   *buflatsize();
extern struct conscell   *bufloat();
extern struct conscell   *bufloatp();
extern struct conscell   *bufopen();
extern struct conscell   *buforeach();
extern struct conscell   *bufprintf();
extern struct conscell   *bufscanf();
extern struct conscell   *bufuncall();
extern struct conscell   *bugc();
extern struct conscell   *bugensym();
extern struct conscell   *buget();
extern struct conscell   *bugetd();
extern struct conscell   *bugetdata();
extern struct conscell   *bugetenv();
extern struct conscell   *bugetlength();
extern struct conscell   *bugo();
extern struct conscell   *bugpname();
extern struct conscell   *bugreap();
extern struct conscell   *bugthan();
extern struct conscell   *buhashstat();
extern struct conscell   *buhsize();

extern struct conscell   *buhtolis();
extern struct conscell   *buhunk();
extern struct conscell   *buhunkp();
extern struct conscell   *buimplode();
extern struct conscell   *buintern();
extern struct conscell   *bulast();
extern struct conscell   *buldiff();
extern struct conscell   *bulength();
extern struct conscell   *bulessp();
extern struct conscell   *bulinenum();
extern struct conscell   *bulist();
extern struct conscell   *bulistarray();
extern struct conscell   *bulistify();
extern struct conscell   *bulistp();
extern struct conscell   *buload();
extern struct conscell   *bulog();
extern struct conscell   *bulog10();
extern struct conscell   *bulsh();
extern struct conscell   *bulthan();
extern struct conscell   *bumacroexpand();
extern struct conscell   *bumakhunk();
extern struct conscell   *bumaknam();
extern struct conscell   *bumakunb();
extern struct conscell   *bumap();
extern struct conscell   *bumapc();
extern struct conscell   *bumapcar();
extern struct conscell   *bumaplist();

extern struct conscell   *bumax();
extern struct conscell   *bumember();
extern struct conscell   *bumemq();
extern struct conscell   *bumemstat();
extern struct conscell   *bumemusage();
extern struct conscell   *bumin();
extern struct conscell   *buminus();
extern struct conscell   *buminusp();
extern struct conscell   *bumod();
extern struct conscell   *bunconc();
extern struct conscell   *bunexplode();
extern struct conscell   *bunot();
extern struct conscell   *bunth();
extern struct conscell   *bunthcdr();
extern struct conscell   *bunthchar();
extern struct conscell   *bunull();
extern struct conscell   *bunumbp();
extern struct conscell   *buoblist();
extern struct conscell   *buoddp();
extern struct conscell   *buoneminus();
extern struct conscell   *buoneplus();

extern struct conscell   *buor();
extern struct conscell   *bupairlis();
extern struct conscell   *buparsetq();
extern struct conscell   *bupatom();
extern struct conscell   *buphoptimize();
extern struct conscell   *buplist();
extern struct conscell   *buplus();
extern struct conscell   *buplusp();
extern struct conscell   *buportp();
extern struct conscell   *buppform();
extern struct conscell   *buprinc();
extern struct conscell   *buprint();
extern struct conscell   *buprintstack();
extern struct conscell   *buprintf();
extern struct conscell   *buprocess();
extern struct conscell   *buproduct();
extern struct conscell   *buprog();
extern struct conscell   *buput();
extern struct conscell   *buputd();
extern struct conscell   *buquote();
extern struct conscell   *buquotient();
extern struct conscell   *burandom();
extern struct conscell   *buread();
extern struct conscell   *bureadc();
extern struct conscell   *bureadli();
extern struct conscell   *bureadln();
extern struct conscell   *bureadst();
extern struct conscell   *buremob();
extern struct conscell   *buremq();
extern struct conscell   *buremove();
extern struct conscell   *buremprop();
extern struct conscell   *burepeat();
extern struct conscell   *buresetio();

extern struct conscell   *bureturn();
extern struct conscell   *bureverse();
extern struct conscell   *bunreverse();
extern struct conscell   *burplaca();
extern struct conscell   *burplacd();
extern struct conscell   *burplacx();
extern struct conscell   *buscanf();
extern struct conscell   *buselect();
extern struct conscell   *buset();
extern struct conscell   *busetarg();
extern struct conscell   *busetplist();
extern struct conscell   *busetq();

extern struct conscell   *busetcreate();
extern struct conscell   *busetlist();
extern struct conscell   *busetand();
extern struct conscell   *busetor();
extern struct conscell   *busetdiff();
extern struct conscell   *busetmember();
extern struct conscell   *busymtcreate();
extern struct conscell   *busymtlist();
extern struct conscell   *busymtmember();
extern struct conscell   *busymtadd();
extern struct conscell   *busymtremove();
extern struct conscell   *busymtsize();
extern struct conscell   *busetsyntax();
extern struct conscell   *bushowstack();
extern struct conscell   *busin();
extern struct conscell   *busizeof();
extern struct conscell   *busleep();
extern struct conscell   *busort();
extern struct conscell   *busortcar();
extern struct conscell   *busopen();
extern struct conscell   *busprintf();
extern struct conscell   *busqrt();
extern struct conscell   *bustringp();
extern struct conscell   *bustrcomp();
extern struct conscell   *bustrlen();
extern struct conscell   *bustrpad();
extern struct conscell   *bustrtrim();
extern struct conscell   *bustrsetpat();
extern struct conscell   *bustrfndpat();
extern struct conscell   *busubst();
extern struct conscell   *busstatus();
extern struct conscell   *busub1();
extern struct conscell   *busubstring();
extern struct conscell   *busum();
extern struct conscell   *busystime();

extern struct conscell   *busysunlink();
extern struct conscell   *buthrow();
extern struct conscell   *butimes();
extern struct conscell   *butimestring();
extern struct conscell   *butildeexpand();
extern struct conscell   *butoupper();
extern struct conscell   *butolower();
extern struct conscell   *butrace();
extern struct conscell   *butruename();
extern struct conscell   *butype();
extern struct conscell   *buuapply();
extern struct conscell   *buuconcat();
extern struct conscell   *buueval();
extern struct conscell   *buuntrace();
extern struct conscell   *buwhile();
extern struct conscell   *buzapline();
extern struct conscell   *buzerop();

extern struct conscell   *bucompile();
extern struct conscell   *buassemble();
extern struct conscell   *budisassemble();
extern struct conscell   *butimeev();

#if    GRAPHICSAVAILABLE                        /* MSDOS bios functions */
extern struct conscell  *buscrmde();
extern struct conscell  *buscrscp();
extern struct conscell  *buscrspt();
extern struct conscell  *buscrsct();
extern struct conscell  *buscrsap();
extern struct conscell  *buscrwdot();
extern struct conscell  *buscrline();
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
       bindvar(thold,thold);                            /* eval(t) == t     */
       thold->botvaris = GLOBALVAR;                     /* GLOBALLY         */
       bindvar(nilhold,NULL);                           /* eval(nil) == ()  */
       nilhold->botvaris = GLOBALVAR;                   /* GLOBALLY         */
       temp = insertatom("$gccount$",PERM);             /* set up system vars */
       temp->botvaris = GLOBALVAR;                      /* and bind to defaults */
       bindvar(temp,newintop(0L));                      /* all bindings are global */
       temp = insertatom("$ldprint",PERM);
       temp->botvaris = GLOBALVAR;                      /* shallow binding */
       bindvar(temp,thold);
       temp = insertatom("$gcprint",PERM);
       temp->botvaris = GLOBALVAR;
       bindvar(temp,NULL);
       temp = insertatom("poport",PERM);
       bindvar(temp,poporthold = PORT(MakePort(stdout,insertatom("stdout",PERM))));
       temp->botvaris = GLOBALVAR;
       temp = insertatom("piport",PERM);
       bindvar(temp,piporthold = PORT(MakePort(stdin,insertatom("stdin",PERM))));
       temp->botvaris = GLOBALVAR;
       temp = insertatom("errport",PERM);
       bindvar(temp,MakePort(stderr,insertatom("stderr",PERM)));
       temp->botvaris = GLOBALVAR;
       macroporthold = insertatom("|$[macroreadport]$|",PERM);
       temp = insertatom("displace-macros",PERM);
       bindvar(temp,NULL);
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

