/*  */

#define VERSION  "6.00"

/******************************************************************************
 **                PC-LISP V2.13 (C) 1986 Peter Ashwood-Smith.               **
 **==========================================================================**
 **    This file "lisp.h" defines all the constants and structures globally  **
 ** used by PC-LISP. It also contains a set of machine/compiler configurations*
 ** which will enable/disable certain code depending on the machine and the C**
 ** compiler being used. If your machine/compiler is not in this list then   **
 ** make any changes necessary to make PC-LISP compile but try to do so by   **
 ** adding a few new #define options to the configurations listed below. If  **
 ** you are compiling for MicroSoft C 3.0 under MSDOS then set MSC3_MSDOS to **
 ** 1. If you are compiling for SYSTEM V then set SYS5_UNIX to 1. If you are **
 ** compiling for BERKELEY UNIX then set BERKELEY_UNIX to 1. Etc. This list  **
 ** should grow as the program is ported from one system to another. Note    **
 ** ONLY 1 of these constants should be defined '1' otherwise you will have  **
 ** problems. When you are porting PC-LISP to a machine/compiler that is not **
 ** in this list, please create a new configuration for the machine/compiler **
 ** and add its name to the list below. If you need to add/remove code that  **
 ** is not handles by one of the configuration #defines, create a new one and**
 ** add it to all old configurations #define'd to 0 and #defined to 1 in the **
 ** new configuration. This will keep the conditional compilation consistant.**
 ******************************************************************************/

#define   MSC3_MSDOS              0             /* Microsoft C 3_0 and MSDOS */
#define   MSC4_MSDOS              0             /* Microsoft C 4_0 and MSDOS */
#define   MSC4_MSDOS_64KDATA      0             /* Microsoft C 4_0 and MSDOS */
#define   SYS5_UNIX               0             /* AT&T System 5, 5.2 etc... */
#define   IBM_RT_AIX              0             /* IBM RT or 6000 running AIX*/
#define   BERKELEY_UNIX           0             /* Berkeley Unix 4.X..       */

/*
 | Establish the correct machine configuration. For PC-LISP if we have
 | the RTPC flag when compiled then we will use the AIX defines, otherwise
 | assume Berkeley. The IBM RS6000 also uses the AIX configuration.
 */
#if RTPC
#   undef    IBM_RT_AIX
#   define   IBM_RT_AIX           1
#else
#   if RS6000
#      undef   IBM_RT_AIX
#      define  IBM_RT_AIX         1
#   else
#      undef    BERKELEY_UNIX
//#      define   BERKELEY_UNIX     1
#   endif
#endif

/* --------------- START OF MACHINE/COMPILER CONFIGURATIONS ------------------*/

/************************************************************************
 ** MicroSoft  C compiler version 3.0 under MS-DOS V2.10 or later.     **
 ** This is a large memory model configuration. We disable the mark    **
 ** stack checking because the C stack is checked for overflow and is  **
 ** guaranteed to overflow first. The SIGSEGV,SIGINT and SIGFPE do not **
 ** work sufficiently well for PC-LISP to make use of them. SIGFPE is  **
 ** not trapped nor is SIGSEGV. SIGINT is trapped via the extra.asm    **
 ** code. A small code model could be made by setting DATALIMITEDTO64K **
 ** to 1 however no stack checking routine is present in extra.asm for **
 ** this model. The small model is 30% faster but is too small for real**
 ** use. For this reason I have not developed a stack check for it.    **
 ** If you wish to compile with DATALIMITEDTO64K then comment out the  **
 ** __chkstk routine and reference to it in extra.asm or write one.    **
 ************************************************************************/
#if       MSC3_MSDOS                            /* IF YOU DON'T HAVE MSC4.0 */
#define   LONGMEMORY              1             /* ptrs are seg/offset ie 0L */
#define   DATALIMITEDTO64K        0             /* stack+data < 64K ? no */
#define   MARKCHECK               0             /* C stack MUST overflow first */
#define   SIGSEGVWORKS            0             /* no signal(SIGSEGV,func) */
#define   SIGINTWORKS             0             /* no signal(SIGINT,func)  */
#define   SIGFPEWORKS             0             /* no signal(SIGFPE,func)  */
#define   HASFCLOSEALL            1             /* has fcloseall() in stdio */
#define   HASMATHERRFUNCTION      1             /* do have matherr(x)  */
#define   WANTERRNOTESTING        1             /* check errno after calls */
#define   GRAPHICSAVAILABLE       1             /* graphics in extra.asm  */
#define   NEEDNLAFTERBREAKEXIT    0             /* EOF needs no NL on break */
#define   JMP_BUFISARRAY          1             /* typedef jmp_buf is array */
#define   DIRSEPSTRING           "\\"           /* directory separator string */
#define   DIRSEPCHAR             '\\'           /* directory separator char */
#define   MAXNEGINT             -32767          /* samllest 'int' value */
#define   MAXINT                 32767          /* largest 'int' value */
#define   MAXRANDVALUE           32767          /* largest rand() value */
#define   MAXLONG                2147483647L    /* largest 'long' value */
#define   MINLONG               -2147483647L    /* smallest 'long' value */
#define   RE_COMP                 0             /* RE_COMP vs REGCMP selection */
#define   HASTCP                  0             /* do we have TCP/IP sockets? */
#endif

/************************************************************************
 ** MicroSoft  C compiler version 4.0 under MS-DOS V2.10 or later.     **
 ** This is a large memory model configuration. We disable the mark    **
 ** stack checking because the C stack is checked for overflow and is  **
 ** guaranteed to overflow first. The SIGSEGV and SIGINT signals do not**
 ** work sufficiently well for PC-LISP to make use of them. SIGFPE is  **
 ** trapped, SIGSEGV is not.    SIGINT is trapped via the extra.asm    **
 ** code. A small code model could be made by setting DATALIMITEDTO64K **
 ** to 1 however no stack checking routine is present in extra.asm for **
 ** this model. The small model is 30% faster but is too small for real**
 ** use. For this reason I have not developed a stack check for it.    **
 ** If you wish to compile with DATALIMITEDTO64K then comment out the  **
 ** __chkstk routine and reference to it in extra.asm or write one.    **
 ************************************************************************/
#if       MSC4_MSDOS                            /* BEST MS-DOS MODEL !!! */
#define   LONGMEMORY              1             /* ptrs are seg/offset ie 0L */
#define   DATALIMITEDTO64K        0             /* stack+data < 64K ? no */
#define   MARKCHECK               0             /* C stack MUST overflow first */
#define   SIGSEGVWORKS            0             /* no signal(SIGSEGV,func) */
#define   SIGINTWORKS             0             /* no signal(SIGINT,func)  */
#define   SIGFPEWORKS             1             /* MSC4.0 signal(SIGFPE) ok */
#define   HASFCLOSEALL            1             /* has fcloseall() in stdio */
#define   HASMATHERRFUNCTION      1             /* do have matherr(x)  */
#define   WANTERRNOTESTING        1             /* check errno after calls */
#define   GRAPHICSAVAILABLE       1             /* graphics in extra.asm  */
#define   NEEDNLAFTERBREAKEXIT    0             /* EOF needs no NL on break */
#define   JMP_BUFISARRAY          1             /* typedef jmp_buf is array */
#define   DIRSEPSTRING           "\\"           /* directory separator string */
#define   DIRSEPCHAR             '\\'           /* directory separator char */
#define   MAXNEGINT             -32767          /* samllest 'int' value */
#define   MAXINT                 32767          /* largest 'int' value */
#define   MAXRANDVALUE           32767          /* largest rand() value */
#define   MAXLONG                2147483647L    /* biggest 'long' value */
#define   MINLONG               -2147483647L    /* smallest 'long' value */
#define   RE_COMP                 0             /* RE_COMP vs REGCMP selection */
#define   HASTCP                  0             /* do we have TCP/IP sockets? */
#endif

/************************************************************************
 ** Portable C compiler under AT&T SYSTEM V UNIX.S V2.10 or later. No  **
 ** memory models. The Mark stack is checked for overflow because we   **
 ** cannot predict the size of the C stack, it faults bigger. All the  **
 ** signals work properly and we have the matherr() function. We want  **
 ** errno checked after every eval() call. We do not have graphics.    **
 ** When EOF is typed we must echo a NL. The rest is straightforward.  **
 ************************************************************************/
#if       SYS5_UNIX                             /* AT&T Unix system V 2.0 + */
#define   LONGMEMORY              0
#define   DATALIMITEDTO64K        0             /* stack+data < 64K ? no */
#define   MARKCHECK               1
#define   SIGSEGVWORKS            1
#define   SIGINTWORKS             1
#define   SIGFPEWORKS             1
#define   HASFCLOSEALL            1             /* has fcloseall() in stdio */
#define   HASMATHERRFUNCTION      1
#define   WANTERRNOTESTING        1             /* check errno after calls */
#define   GRAPHICSAVAILABLE       0
#define   NEEDNLAFTERBREAKEXIT    1
#define   JMP_BUFISARRAY          1             /* typedef jmp_buf is array */
#define   DIRSEPSTRING           "/"
#define   DIRSEPCHAR             '/'
#define   MAXNEGINT             -2147483647L    /* samllest 'int' value */

#ifndef   MAXINT
#define   MAXINT                 2147483647L    /* largest 'int' value */
#endif

#define   MAXRANDVALUE           MAXINT         /* largest rand() value */

#ifndef   MAXLONG
#define   MAXLONG                2147483647L    /* biggest 'long' value */
#endif

#define   MINLONG               -2147483647L    /* smallest 'long' value */
#define   RE_COMP                 0             /* RE_COMP vs REGCMP selection */
#define   HASTCP                  1             /* do we have TCP/IP sockets? */
#endif


#ifdef    __linux__
#include <limits.h>
#define   LONGMEMORY              0
#define   DATALIMITEDTO64K        0             /* stack+data < 64K ? no */
#define   MARKCHECK               1
#define   SIGSEGVWORKS            1
#define   SIGINTWORKS             1
#define   SIGFPEWORKS             1
#define   HASFCLOSEALL            1             /* has fcloseall() in stdio */
#define   HASMATHERRFUNCTION      1
#define   WANTERRNOTESTING        1             /* check errno after calls */
#define   GRAPHICSAVAILABLE       0
#define   NEEDNLAFTERBREAKEXIT    1
#define   JMP_BUFISARRAY          1             /* typedef jmp_buf is array */
#define   DIRSEPSTRING           "/"
#define   DIRSEPCHAR             '/'
#define   MAXNEGINT             (long)INT_MIN    /* samllest 'int' value */

#ifndef   MAXINT
#define   MAXINT                 (long)INT_MAX    /* largest 'int' value */
#endif

#define   MAXRANDVALUE           MAXINT         /* largest rand() value */

#ifndef   MAXLONG
#define   MAXLONG                LONG_MAX    /* biggest 'long' value */
#endif

#define   MINLONG                LONG_MIN    /* smallest 'long' value */
#define   RE_COMP                 1             /* RE_COMP vs REGCMP selection */
#define   HASTCP                  0             /* do we have TCP/IP sockets? Although Linux does, the TCP code is outdated.  Disable for now. */
#endif


#ifdef    __APPLE__
#include <limits.h>
#define   LONGMEMORY              0
#define   DATALIMITEDTO64K        0             /* stack+data < 64K ? no */
#define   MARKCHECK               1
#define   SIGSEGVWORKS            1
#define   SIGINTWORKS             1
#define   SIGFPEWORKS             1
#define   HASFCLOSEALL            1             /* has fcloseall() in stdio */
#define   HASMATHERRFUNCTION      1
#define   WANTERRNOTESTING        1             /* check errno after calls */
#define   GRAPHICSAVAILABLE       0
#define   NEEDNLAFTERBREAKEXIT    1
#define   JMP_BUFISARRAY          1             /* typedef jmp_buf is array */
#define   DIRSEPSTRING           "/"
#define   DIRSEPCHAR             '/'
#define   MAXNEGINT             (long)INT_MIN    /* samllest 'int' value */

#ifndef   MAXINT
#define   MAXINT                 (long)INT_MAX    /* largest 'int' value */
#endif

#define   MAXRANDVALUE           MAXINT         /* largest rand() value */

#ifndef   MAXLONG
#define   MAXLONG                LONG_MAX    /* biggest 'long' value */
#endif

#define   MINLONG                LONG_MIN    /* smallest 'long' value */
#define   RE_COMP                 0             /* RE_COMP vs REGCMP selection */
#define   HASTCP                  0             /* do we have TCP/IP sockets? Although Linux does, the TCP code is outdated.  Disable for now. */
#endif


/************************************************************************
 ** University of California Berkeley UNIX. (All versions ?) No idea of**
 ** memory models. The Mark stack is checked for overflow because we   **
 ** cannot predict the size of the C stack, it faults bigger. All the  **
 ** signals work properly but we do not have a matherr(). Instead want **
 ** errno checked after every eval() call. We do not have graphics.    **
 ** When EOF is typed we must echo a NL. The rest is straightforward.  **
 ************************************************************************/
#if       BERKELEY_UNIX                         /* Univ of Cal. Berkeley Unix */
#define   LONGMEMORY              0
#define   DATALIMITEDTO64K        0             /* stack+data < 64K ? no */
#define   MARKCHECK               1
#define   SIGSEGVWORKS            1
#define   SIGINTWORKS             1
#define   SIGFPEWORKS             1
#define   HASFCLOSEALL            0             /* no fcloseall() fake it */
#define   HASMATHERRFUNCTION      0
#define   WANTERRNOTESTING        0
#define   GRAPHICSAVAILABLE       0
#define   NEEDNLAFTERBREAKEXIT    1
#define   JMP_BUFISARRAY          1             /* typedef jmp_buf is array */
#define   DIRSEPSTRING           "/"
#define   DIRSEPCHAR             '/'
#define   MAXNEGINT             -2147483647L    /* samllest 'int' value */

#ifndef   MAXINT
#define   MAXINT                 2147483647L    /* largest 'int' value */
#endif

#define   MAXRANDVALUE           MAXINT         /* largest rand() value */

#ifndef   MAXLONG
#define   MAXLONG                2147483647L    /* biggest 'long' value */
#endif

#define   MINLONG               -2147483647L    /* smallest 'long' value */
#define   RE_COMP                 1             /* RE_COMP vs REGCMP selection */
#define   HASTCP                  1             /* do we have TCP/IP sockets? */
#endif

/************************************************************************
 * IBM's PC-RT under AIX it has many of the properties of a BERKELEY    *
 * system except the rand() function returns in the range 0..2^15 - 1   *
 ************************************************************************/
#if       IBM_RT_AIX                            /* Univ of Cal. Berkeley Unix */
#define   LONGMEMORY              0
#define   DATALIMITEDTO64K        0             /* stack+data < 64K ? no */
#define   MARKCHECK               1
#define   SIGSEGVWORKS            1
#define   SIGINTWORKS             1
#define   SIGFPEWORKS             1
#define   HASFCLOSEALL            0             /* no fcloseall() fake it */
#define   HASMATHERRFUNCTION      0
#define   WANTERRNOTESTING        0
#define   GRAPHICSAVAILABLE       0
#define   NEEDNLAFTERBREAKEXIT    1
#define   JMP_BUFISARRAY          1             /* typedef jmp_buf is array */
#define   DIRSEPSTRING           "/"
#define   DIRSEPCHAR             '/'
#define   MAXNEGINT             -2147483647L    /* samllest 'int' value */

#ifndef   MAXINT
#define   MAXINT                 2147483647L    /* largest 'int' value */
#endif

#define   MAXRANDVALUE           32767          /* largest rand() value */

#ifndef   MAXLONG
#define   MAXLONG                2147483647L    /* biggest 'long' value */
#endif

#define   MINLONG               -2147483647L    /* smallest 'long' value */
#define   RE_COMP                 0             /* RE_COMP vs REGCMP selection */
#define   HASTCP                  1             /* do we have TCP/IP sockets? */
#endif

/* -------------------- END OF MACHINE CONFIGURATIONS ------------------------*/

#include  <string.h>
#include  <stdlib.h>
#include  <errno.h>
#include  <setjmp.h>                              /* longjmp is used to escape*/

#if       LONGMEMORY                              /* if long memory model we  */
#undef    NULL                                    /* must make NULL 0L not 0  */
#define   NULL          0L                        /* because of big pointers. */
#endif
						  /* if not lattice C Unix?*/
#ifndef   max                                     /* UNIX stdio.h does not */
#define   max(a,b)      (((a)>(b))?(a):(b))       /* always define max and */
#define   min(a,b)      (((a)<(b))?(a):(b))       /* min. */
#endif

void stkovfl();

/*
 | These are the mark stack and memory manager block sizes. We must be
 | careful here to make sure that we are not using memory poorly because
 | many malloc/callocs use a power of 2 system where the request is
 | rounded up to the next power of two. This is true of the APOLLO so
 | the value 17400 is arrived at experimentally so as not to create
 | excessive waste. The calloc's are checked to see how far appart the
 | blocks are and if they are very far apart a warning message is
 | printed asking the developer to tune these values and avoid any
 | significant wasted space.
 */
#if       DATALIMITEDTO64K
#define   MSSIZE        1024                      /* mark stack is about 4K */
#define   BLOCKSIZE     8192                      /* half size config blocks */
#else
#define   MSSIZE        16384                     /* mark stack is about 64K */
#define   BLOCKSIZE     17400                     /* Large contig blocks  */
#endif

#define   MAXCBLOCKS    1800                      /* max blocks of cells */
#define   MAXABLOCKS    400                       /* max blocks of alphas */
#define   MAXHBLOCKS    400                       /* max blocks of heaps */
#define   HSSIZE        20                        /* showstack max size */
#define   ALPHATABSIZE  1499                      /* prime hash table size */
#define   MAXATOMSIZE   1024                      /* can be up to max cell size of mman */
                                                  /* but 1024 is more than enough */

/*
 * NOTE - there are two stacks used by PC-LISP. The normal C stack, and the
 * mark stack which is split into two sub stacks the eval and other stacks.
 * The mark stacks have a total of MSSIZE pointers allocated to them and grow
 * toward each other. There are 4 possible configurations for stack overflow
 * trapping in PC-LISP, none, mark stack overflow, C-stack overflow and both
 * mark and C stack overflow. The safest is to check both the C and the mark
 * stacks for overflow although this costs about 2% on the run time and is not
 * necessary if the smaller of the two stacks (C and Mark) can be guaranteed
 * to overflow before the other. If this is the case then you only need to
 * check the top of this one stack. For a system with 4byte pointers, and a C
 * stack of 50K the mark stack will never push more than 2000 pointers. So,
 * the ratio C over mark stack usage is about 50/8. Or about 6 times as much
 * C stack as mark stack is needed. You can compute this exact ratio by
 * defining DEBUG and MARKCHECK as 1, recompiling and running some evals that
 * will overflow the stack. When the overflow is trapped either by the C stack
 * overflow or the mark stack overflow a count of mark stack useage (in ptrs)
 * is dumped. If the C stack overflowed, then you can drop the mark stack size
 * MSSIZE until it is a bit bigger than the number dumped then set MARKCHECK to
 * 0 and be fairly sure it will never overflow. (You should give a 10 or 20%)
 * margin for error.
 *
 */                                               /**/
						  /* ENV VARS AND DEFAULTS */
#define   LIBENVVAR    "APPL_PATH"                /* load library paths*/
#define   AUTOSTART    "pclisprc.l"               /* auto load file name */
#define   BLOCKDEFAULT  70                        /* default maxblocks */
#define   ALPHDEFAULT   1                         /* default alpha blocks */
#define   HEAPDEFAULT   1                         /* default heap blocks */
						  /**/
#define   PRETTYWIDTH   75                        /* default pp-form width */
#define   MAXMETANEST   16                        /* max [] nesting allowed */

/***************************************************************************
 ** garbage collection stack push and pop routines. The stack 'mystack'   **
 ** contains a pointer to every local variable and parameter. It is much  **
 ** like the stack used by C. Infact the pointers will point into the C   **
 ** stack. When we enter a procedure in which we may cause the garbage    **
 ** collector to gather data, ie a new is invoked. We will xpush all of   **
 ** the arguements, and push all of the local variables. Then when we do  **
 ** a return we use fret or xret with the correct number of items that we **
 ** pushed at the entry to the procedure. Note that xret is provided for  **
 ** returns that do some evaluation in the returned argument,in which case**
 ** we wait until after the evaluation and then pop the mark stack. I have**
 ** added epush,efret,exret which are used only by eval(). They push and  **
 ** pop from at second stack that grows downward in the mystack array.    **
 ** The overall stack space usage will be the same but it is now very easy**
 ** to perform the showstack function. We simply copy the top of the eval **
 ** 'epushed' items. Before I did this I had to sift through the mystack  **
 ** looking for lists that looked like they were pushed by eval(). This   **
 ** did not work properly. The separate stack idea should work flawlessly.**
 ** Stack overflow testing is enabled only if MARKCHECK is defined 1. For **
 ** A 64K system stack and a 32K mark stack the C-stack will overflow     **
 ** before the mark stack and the markstack overflow can be removed to    **
 ** speed up the code a little. For unix systems though the MARKCHECK is  **
 ** turned on because of the unpredicatable size of the stack. Sorry about**
 ** the mess.                                                             **
 ***************************************************************************/
 extern   int mytop,emytop;
 extern   struct conscell ***mystack;

#if       MARKCHECK
#         define   OVFT           if (mytop > emytop) stkovfl(0);
#else
#         define   OVFT
#endif

/***************************************************************************
 ** Various pushes onto the mark stack. push(x) is to push a local variable*
 ** onto the mark stack. We must set it to NULL because we do not want the**
 ** marking routine to follow an unitialized pointer. xpush(x) is to push **
 ** a parameter onto the mark stack, or to push a local variable that we  **
 ** are going to do an immediate assignment to. This avoids the NULL assi-**
 ** gnement that would otherwise by done by push and is a little faster.  **
 ** xpop(n) will pop n items from the mark stack. expush(x) will push an  **
 ** eval() parameter onto the eval stack (this is at the opposite end of  **
 ** the mystack space so it grows downward.                               **
 ***************************************************************************/
#define   push(x)           OVFT mystack[mytop++]=(struct conscell **)&x;x=NULL
#define   xpush(x)          OVFT mystack[mytop++]=(struct conscell **)&x
#define   MarkStackTop()    ((mytop > 0) ? *mystack[mytop-1] : NULL)
#define   ClearMarkStacks() { mytop = 0; emytop = MSSIZE-1; }
#define   EmptyMarkStacks() ((mytop == 0)&&(emytop == MSSIZE-1))
#define   xpop(nn)          mytop -= nn;
#define   expush(x)         OVFT mystack[emytop--]=(struct conscell **)&x
#define   expop(nn)         emytop += nn;

/***************************************************************************
 ** various pop/return combinations. fret - will do a fast return. It is  **
 ** used when the VARIABLE mr is to be returned and mn items are to be    **
 ** popped from the mark stack. xret - is used when the returned object is**
 ** a function invokation. It just assigns the result to a temporary and  **
 ** then pops the mark stack by mn objects and returns the temporary. The **
 ** efret and exret are just the same as fret and xret but they operate on**
 ** the downward growing eval stack. They are used exclusively by eval(). **
 ***************************************************************************/
#define   fret(mr,mn)  { mytop -= mn; \
			 return(mr);  \
		       }
#define   xret(mr,mn)  { register struct conscell *rrr=(struct conscell *)mr; \
			 mytop-=mn;   \
			 return(rrr); \
		       }
#define   efret(mr,mn) { emytop += mn; \
			 return(mr);   \
		       }
#define   exret(mr,mn) { register struct conscell *rrr=(struct conscell *)mr; \
			 emytop+=mn;   \
			 return(rrr);  \
		       }

/***************************************************************************
 ** Signal handler for break. Since MS-DOS is non reentrant this mechanism**
 ** allows periodic testing of call and reentrency is a problem with many**
 ** MSDOS systems the break handler operates by periodic testing of the   **
 ** variable bkhitcount which is normally zero but is incremented by one  **
 ** if ever a SIGINT occurs. The TEST_BREAK() macro is put in a few loops **
 ** in the code and will cause a jump to brkhit() which prints messages   **
 ** handles the logging of stack trace info and then longjmps out of the  **
 ** error. The TEST_BREAK is placed in the scan input loop the write out  **
 ** put loop, and in the eval() function which is the core of all activity**
 ***************************************************************************/
extern    int bkhitcount;
extern    int brkhit();
#define   TEST_BREAK()  if (bkhitcount) brkhit()
#define   BREAK_RESET() bkhitcount = 0

/***************************************************************************
 ** Next are the structures used by lisp. They are the cons cell alpha    **
 ** cell and real cell. They all contain a celltype and markbit field.    **
 ** The celltype contains a 0 1 or 2 depending on the type of the cell.   **
 ** The markbit is 1 when touched by the marking algorithm and 0 when it  **
 ** is clear. Note that the cons real and file are really a sort of union **
 ** because space will be allocated for the biggest of the 3 . We depend  **
 ** on the fact that the three structures will all be aligned in the same **
 ** way so that pointers to any three can be used interchangably. This is **
 ** not very good coding practice but it seems to work on most machines.  **
 ** If it does not on your machine you will have to pad the structures    **
 ** until they align in the same way.   A few notes about the alpha cell  **
 ** may elucidate things. The alpha cells are hashed into an array. If a  **
 ** collision occurs it is added to the tail of the list for that hash    **
 ** index. They are chained on the 'link' field. The fntype field will be **
 ** non NULL if the alpha atom is a built in (written in C) one. The value**
 ** will point to the function to be called when the atom is encountered. **
 ** The 'proplist' field is a pointer to a list of property that is       **
 ** owned by the atom. Each element in the property list is a dotted pair **
 ** if indicator atom and property list. A user defined function is just  **
 ** a lambda expression pointer placed in the 'func' field instead of the **
 ** C entry point. This diference is reflected in the 'fntype' bit field. **
 ***************************************************************************/

/***
 ***  Bit field settings and their meanings. They correspond to similar
 ***  fields in all cell types. Note especially the function type bits
 ***  which are FNT_xxx. They control the eval and apply.
 ***
 ****/
						/* allcells->celltype bits */
						/* cells s.t eval(c) != c  */
#define         ALPHAATOM               0       /* celltype for alpha cell */
#define         CONSCELL                1       /* celltype for conscell   */

						/* allcells->celltype bits */
						/* cells s.t. eval(c)==c   */
#define         REALATOM                2       /* a flonum cell type      */
#define         FILECELL                3       /* open file pointer FILE* */
#define         FIXATOM                 4       /* 32 bit integer cell     */
#define         STRINGATOM              5       /* "...." type of atom.    */
#define         HUNKATOM                6       /* array of cell pointers. */
#define         ARRAYATOM               7       /* large array of cells    */
#define         CLISPCELL               8       /* compiled LISP reference */
#define         FIXFIXATOM              9       /* cell of two fixnums     */

						/* alphacell->valstack bit */
#define         GLOBALVAR               1       /* valuestack bottom type  */
#define         LOCALVAR                0
						/* allcells->markbit values*/
#define         CLEAR                   0       /* markbit clear to gather */
#define         SET                     1       /* markbit do not gather   */

						/* alphacell->permbit values*/
#define         NOT_PERM                0       /* ok to collect if unref'ed*/
#define         PERM                    1       /* never collect this atom */

						/* alpahcell->interned bit */
#define         NOT_INTERNED            0       /* atom may not be unique. */
#define         INTERNED                1       /* atom unique by print name*/

#define         TRACE_ON                1       /* trace this functions calls*/
#define         TRACE_OFF               0       /* no trace of calls&exits  */
						/* useful groupings to save */
						/* typing and be consistent */

#define         FN_NONE                 0       /* no function for this atom*/
#define         FN_BUPRED               1       /* predicate, no args, t | nil returned */
#define         FN_USEXPR               2       /* user lexpr,(n)lambda */
#define         FN_USMACRO              3       /* user macro */
#define         FN_CLISP                4       /* function is clispcell */
#define         FN_BUEVAL               5       /* built in eval args */
#define         FN_BUNEVAL              6       /* built in no eval args */
#define         FN_ISBU(n)             (n >= FN_BUEVAL)
#define         FN_ISUS(n)            ((n == FN_USEXPR)||(n == FN_USMACRO))
#define         FN_ISPRED(n)           (n == FN_BUPRED)
#define         FN_ISCLISP(n)          (n == FN_CLISP)

/************************************************************************
 ** The conscell: The basic list element linking cell. The car pointer **
 ** points to the element and the cdr pointer points to the rest of the**
 ** elements in the list. celltype=CONSCELL.  The travbit is used when **
 ** doing the garbage collection marking phase. It is used to indicate **
 ** when when a link is inverted. The filecell,hunkcell,stringcell,    **
 ** realcell, fixcell have dummy travbits in them. This is because all **
 ** of these cells share the same space and size and may be converted  **
 ** from one to another. We require that this bit stay in sync as do   **
 ** the markbit and celltype bits.                                     **
 ************************************************************************/
struct  conscell                                /* lisp cons cell */
{               unsigned celltype  : 4;         /* celltype = CONSCELL */
		unsigned markbit   : 1;         /* CLEAR or SET  */
		unsigned travbit   : 1;         /* link invert traversal bit*/
                unsigned linenum   : 23;        /* line number from file this list came from */
		struct   conscell  * carp;      /* lisp CAR pointer */
		struct   conscell  * cdrp;      /* lisp CDR pointer */
};

/************************************************************************
 ** The alphacell: The type "symbol". Holds pointers to the value of   **
 ** the symbol as a stack of bindings. The stack is implemented as a   **
 ** list of conscells whose car is the value of this atom at this level**
 ** binding. A pointer is kept to any property lists. A pointer is kept**
 ** to a built in function which is dispatched when eval decides that  **
 ** this atom represents a function. This function pointer may double  **
 ** as a user defined function pointer. The cast is a bit dubious on   **
 ** some machines but on most orthogonal architectures it seems ok.    **
 ** Next is a pointer to the string which is the print name of the atom**
 ** this storage is in heap space. A number of flag bits are also used **
 ** by various routines in the evaluator. The permbit is used by the   **
 ** Garbage collector to decide if the atom is reclaimable or not. Ie  **
 ** If the atom is pure space or not. Built in functions and atoms used**
 ** by the system all have this bit SET so that the atom is not gather-**
 ** ed. Next the fntype bits indicate the kind of function these bits  **
 ** are described by the FNT_ bits above. They allow eval to know if   **
 ** the func field is a built in ASM function, or a lisp expression,   **
 ** and how to evaluate the parameters in this case. Macros are treated**
 ** specially and have their own bit. Next the tracebit is turned on & **
 ** off by the trace and untrace functions and cause apply to print deb**
 ** ugging information when a user function is entered&exited. Next the**
 ** iscadar bit is a '1' if the atom field contains a string which is  **
 ** of the form 'c{a|d}+r'. This bit is set by the insertatom function **
 ** and saves us a little time in eval() evaluating (caaadadddar xxx)  **
 ** expressions. Last the botvaris flag is used to indicate what type  **
 ** of variable the bottom variable on the valstack is. It may be one  **
 ** of GLOBALVAR or LOVALVAR and is needed to control the unbinding    **
 ** of all atoms performed when the break level is left. If the bottom **
 ** var is global we obviously do not want to pop it.                  **
 ************************************************************************/
struct  alphacell                               /* lisp alpha cell */
{               unsigned celltype  : 4;         /* celltype = ALPHAATOM */
		unsigned markbit   : 1;         /* CLEAR  or SET */
		unsigned permbit   : 1;         /* PERM = permanent atom no gc */
		unsigned interned  : 1;         /* INTERNED = atom on oblist */
		unsigned fntype    : 4;         /* function type bits see FN_*/
		unsigned tracebit  : 1;         /* tracing function YES or NO*/
		unsigned iscadar   : 1;         /* one of c{a|d}*r functions?*/
		unsigned botvaris  : 1;         /* var type of bot of valstack*/
		struct   conscell  * valstack;  /* scope stack for this var */
		struct   conscell  * proplist;  /* propert list pointer */
		struct   conscell *(*func)();   /* C function or Lambda expr */
		char     *atom;                 /* print name string       */
};
	
/************************************************************************
 ** The realcell: The type 'flonum' holds a floating point number. Has **
 ** the usual celltype field which will be REALCELL for a valid cell of**
 ** this kind, and a markbit for use by garbage collector(like all the **
 ** other lisp cell types).                                            **
 ************************************************************************/
struct  realcell                                /* lisp real cell */
{               unsigned celltype  : 4;         /* celltype = REALCELL */
		unsigned markbit   : 1;         /* markbit = SET or CLEAR */
		unsigned travbit   : 1;         /* Dummy invert traversal bit*/
		double    atom;                  /* value of real number cell */
};
	
/************************************************************************
 ** The fixcell: The type 'fixnum'  holds a 32 bit integer number. Has **
 ** the usual celltype field which will be FIXCELL  for a valid cell of**
 ** this kind, and a markbit for use by garbage collector (like all the**
 ** other lisp cell types).                                            **
 ************************************************************************/
struct  fixcell                                 /* lisp 32bit integer cell */
{               unsigned celltype  : 4;         /* celltype = REALCELL */
		unsigned markbit   : 1;         /* markbit = SET or CLEAR */
		unsigned travbit   : 1;         /* Dummy invert traversal bit*/
		long int atom;                  /* value of the fixnum */
};

/************************************************************************
 ** The fixfixcell: The type 'fixfixnum' holds a 2 32 bit integers.Has **
 ** the usual celltype field which will be FIXFIXCELL for a valid cell **
 ** this kind, and a markbit for use by garbage collector (like all the**
 ** other lisp cell types). This celltype is used by the compiler to   **
 ** generate literals that effeciently describe offset and literal# in **
 ** a compiled clisp object.                                           **
 ************************************************************************/
struct  fixfixcell                              /* lisp 2 32bit integer cell */
{               unsigned celltype  : 4;         /* celltype = REALCELL */
		unsigned markbit   : 1;         /* markbit = SET or CLEAR */
		unsigned travbit   : 1;         /* Dummy invert traversal bit*/
		long int atom1;                 /* value of the fixnum1 */
                long int atom2;                 /* value of the fixnum2 */
};
	
/************************************************************************
 ** The filecell: The type 'port' holds a pointer to a standard I/O    **
 ** FILE  type. The FILE  may be open closed or something in between.  **
 ** In addition there is a pointer to the atom whose print name is the **
 ** opened files 'truename'. This is used by function 'truename' to    **
 ** print the file associated with a port and is printed by 'printatom'**
 ** If socketopen created the port then issocket is set to TRUE. This  **
 ** is then used by the I/O functions to turn force a rewind on the    **
 ** socket when the I/O direction changes. We also have a state which  **
 ** is used to decide if we are reading or writing a socket. This is   **
 ** saved so that we can issue rewind calls on the FILE * when there   **
 ** is a state change. If we do not do this on a stream/socket then the**
 ** FILE * routines do not work properly. See the man on fdopen(3).    **
 ************************************************************************/
struct  filecell                                /* lisp open file pointer */
{               unsigned celltype  :  4;        /* celltype = FILECELL */
		unsigned markbit   :  1;        /* markbit = SET or CLEAR */
		unsigned travbit   :  1;        /* Dummy invert traversal bit*/
                unsigned issocket  :  1;        /* TRUE if socketopen created it */
                unsigned state     :  2;        /* 0=>unknown, 1=>reading, 2=>writing */
                unsigned linenum   : 23;        /* last line read from this port */
		FILE   *atom;                   /* file pointer field */
		struct alphacell *fname;        /* ptr to atom with file name*/
};

/************************************************************************
 ** The stringcell: The type 'string' holds the usual celltype and the **
 ** markbit fields and a pointer into heap space. The data in the heap **
 ** space is a standard C string which is the value of this cell. This **
 ** cell along with alphacells and hunkcells is kept in the hashtable  **
 ** by the main.c module so that when heap space is moved about we can **
 ** find all those who refer to it (there will only be one Referent)   **
 ** and adjust his/her 'atom' field appropriately. See FindReferent()  **
 ** and InformRelocating() in main.c and mman.c respectively.          **
 ************************************************************************/
struct  stringcell                              /* lisp string "...." cell */
{               unsigned celltype  : 4;         /* celltype = STRINGCELL */
		unsigned markbit   : 1;         /* markbit = SET or CLEAR */
		unsigned travbit   : 1;         /* Dummy invert traversal bit*/
		char    *atom;                  /* pointer to heap */
};

/************************************************************************
 ** The hunkcell: The type 'hunk' holds  the necessary information for **
 ** maintaining an array of up to 254/(sizeof ptr)   pointers. This is **
 ** printed as {a b c...d} where a b c ... d are the result of printing**
 ** the data in each cell pointed to in the array. The array that is   **
 ** stored in the heap cannot be directly addressed because the heap   **
 ** manager may move the block around in memory thus destroying any    **
 ** alignment. The only solution is to load each byte of a pointer one **
 ** after the other from this heap space. This is a little slower than **
 ** a direct load but the alternative is to maintain the array alignme-**
 ** nt from compaction to compaction which adds time to the garbage    **
 ** collection cycle. Since garbage collection is a big hole in the    **
 ** execution of a program I would rather slow down the access of the  **
 ** array than increase the cost of maintaining it. This cell has a    **
 ** size field which is the number of elements in the the hunk. Then   **
 ** the atom pointer points to the first byte of the first pointer in  **
 ** the hunk. These cells are kept in the atomtable by main.c so that  **
 ** heap compaction can proceed quickly. This is because for a given   **
 ** block of memory we need to figure out who owns it quickly.         **
 ************************************************************************/
struct  hunkcell                                /* lisp hunk { ....} cell */
{               unsigned celltype  : 4;         /* celltype = HUNKCELL */
		unsigned markbit   : 1;         /* markbit = SET or CLEAR */
		unsigned travbit   : 1;         /* Dummy invert traversal bit*/
                unsigned symtbeq   : 1;         /* use 'eq' or 'equal' for symtabs? */
		int      size;                  /* number of elements */
		char    *atom;                  /* pointer to heap */
};

/************************************************************************
 ** The arraycell: Type 'array' holds all info necessary for dealing   **
 ** with a large virtually contiguous bank of cell pointers. The array **
 ** contiguos storage is stored in 'base' and may be zero, one or more **
 ** levels of indirection from this base. Depending on the size of the **
 ** array. The 'info' list stores the size of the array followed by the**
 ** dimensions this is stored as a list so that  array subscript calc  **
 ** ulation is done by walking down the 'info' list and performing a   **
 ** simple Horners rule calculation using info dimensions as powers.   **
 ** Raw storage is managed as an ARRAYN-ary tree. See ArrayAllocTree in**
 ** bufunc2.c for its construction. See also next comment box.         **
 ************************************************************************/
struct  arraycell
{               unsigned celltype  : 4;
		unsigned markbit   : 1;
		unsigned travbit   : 1;      /* Dummy invert traversal bit*/
		struct conscell *info;       /* (size dim1 dim2 dim3...) */
		struct hunkcell *base;       /* raw array storage */
};

/************************************************************************
 ** The compiled lisp cell, represents byte coded data. The (assemble) **
 ** function will return one of these that can then be putd'd on an    **
 ** atom. The
 ************************************************************************/
struct  clispcell                            /* compiled LISP cell */
{               unsigned celltype  : 4;      /* celltype = STRINGCELL */
		unsigned markbit   : 1;      /* markbit = SET or CLEAR */
		unsigned travbit   : 1;      /* Dummy invert traversal bit*/
                unsigned pure      : 1;      /* TRUE if pure native machine code FALSE if byte coded */
                unsigned eval      : 1;      /* TRUE if interpreter is to eval args before call */
                struct conscell **literal;   /* ARRAY!!! literal[i] as referenced by code & NULL terminated for GC */
		char            *code;       /* pointer to pure malloc space containg byte or machine code */
};

/************************************************************************
 ** ARRAYN : defines the width of the tree nodes to be used to build   **
 ** tree whose leaves are the elements of an array. For best results   **
 ** this should be as large as possible ie the maximum hunk size. Since**
 ** the largest hunk has 2*maxat/(size char *) elements because a hunk **
 ** is maxatom bytes of pointers to cons cells, each of which stores 2 **
 ** elements of the hunk. This gives the optimum speed value for ARRAYN**
 ** It may be better to make it smaller under some circumstances to    **
 ** make most objects on heap near same size, this will make GC faster.**
 ** ARRAYM : defines the number of spaces in the node that are used for**
 ** quotient storage it MUST BE exactly one less than ARRAYN. A small  **
 ** value for ARRAYN will however make lookup and allocation slower.   **
 ************************************************************************/
#define         ARRAYN          ((2*MAXATOMSIZE/(sizeof (char *)))-1)
#define         ARRAYM          (ARRAYN-1)
							
/*********************************************************
 *** Type casts used after a celltype field is tested. ***
 *** To trick compiler into using correct structures.  ***
 *** Note the FUNCTION is NOT correct type cast but it ***
 *** is correct size (it is used for a parameter cast  ***
 *** when sizeof(char *) != sizeof(&f()) ie various    ***
 *** Models for Segmented machines and seems to work.  ***
 *********************************************************/
#define         REAL(x)         ((struct realcell   *)(x))
#define         LIST(x)         ((struct conscell   *)(x))
#define         PORT(x)         ((struct filecell   *)(x))
#define         FIX(x)          ((struct fixcell    *)(x))
#define         FIXFIX(x)       ((struct fixfixcell *)(x))
#define         ALPHA(x)        ((struct alphacell  *)(x))
#define         STRING(x)       ((struct stringcell *)(x))
#define         HUNK(x)         ((struct hunkcell   *)(x))
#define         ARRAY(x)        ((struct arraycell  *)(x))
#define         FUNCTION(x)     ((int (*)())(x))
#define         CLISP(x)        ((struct clispcell  *)(x))

/*** compute roof(n/2) used for hunk size computations ***/
#define         CEILING_DIV_2(n)     (((n)>>1) + ((n) % 2))

/*** special flags used/returned by utility routines ****/
						/* function printatom() */
#define         DELIM_ON        1               /* printatoms with | | */
#define         DELIM_OFF       0               /* or without.         */

						/* function HoldStackOperation */
#define         MARK_STACK      2               /* these are operation number */
#define         COPY_STACK      1
#define         DUMP_STACK      0

						/* MixedTypeCompare() result */
#define         MT_ERROR        0               /* bad types to compare */
#define         MT_GREATER      1               /* op1 > op2 */
#define         MT_LESS         2               /* op1 < op2 */
#define         MT_EQUAL        3               /* op1 == op2 */

#define         INTERRUPT       "Interrupted"   /* string printed on BREAK */

/************************************************************************
 ** These constants are passed to GetOption and SetOption which are    **
 ** control the way that PC-LISP behaves under certain circumstances.  **
 ** They correspond to the (sstatus) function call (which is how the   **
 ** user can set them) Note the default values for each.               **
 ************************************************************************/
#define         SMARTSLASH           0          /* Get/Set Options flags */
#define         AUTORESET            1
#define         CHAINATOM            2
#define         IGNOREEOF            3
						/* DEFAULT SETTINGS */
#define         SMARTSLASHDEFAULT    1          /* do \n interpretation by default */
#define         AUTORESETDEFAULT     0          /* do break level on ERR */
#define         CHAINATOMDEFAULT     0          /* ERR when (ca|dr atom) */
#define         IGNOREEOFDEFAULT     0          /* exit on EOF at top level */

/************************************************************************
 ** Functions and variables that are used globally by PC-LISP. We put  **
 ** extern definitions here to make sure that no pointer problems occur**
 ** on machines with unusual pointer representations ie non int ptrs.  **
 ************************************************************************/
extern struct conscell   * apply();                 /* joint two lists       */
extern struct conscell   * arrayaccess();           /* array get/put routine */
extern struct conscell   * bucadar();               /* the c{a|d}+r function.*/
extern struct conscell   * copy();                  /* complete copy of parm.*/
extern struct alphacell  * CreateInternedAtom();
extern struct alphacell  * CreateUninternedAtom();
extern struct conscell   * CopyOblist();            /* returns (oblist)      */
extern struct conscell   * eval();                  /* main evaluator.       */
extern struct conscell   * evlis();                 /* main evalutor of list */
extern struct conscell   * evalclisp();             /* run byte coded eval   */
extern struct conscell  ** GetHunkIndex();          /* returns H[i] ptr ptr  */
extern struct conscell  ** GetArrayIndex();         /* returns A[i] ptr ptr  */
extern struct conscell   * GetTraced();             /* all (trace'ed) funcs  */
extern struct conscell   * getprop();               /* get property of atom  */
extern struct conscell   * HashStatus();            /* the collision counts  */
extern struct conscell   * HunkToList();            /* explode Hunk to list  */
extern struct stringcell * insertstring();          /* makes a new string    */
extern struct hunkcell   * inserthunk();            /* makes a new hunk      */
extern struct alphacell  * insertatom();            /* makes a new atom      */
extern struct alphacell  * lookupatom();            /* given name finds atom */
extern struct conscell   * lexprify();              /* process &optional's   */
extern struct conscell   * enlist();                /* (cons l nil)          */
extern struct conscell   * MakePort();              /* returns an opened port*/
extern struct conscell   * macroexpand();           /* run macro on code     */
#define new(x) newcons(x)                           /* rename new it clashes */
extern struct conscell   * newcons();               /* cell allocator fn.    */
extern struct alphacell  * newalpha();              /* alpha cell allocator  */
extern struct conscell   * newintop();              /* calls new(FIXATOM).   */
extern struct conscell   * newfixfixop();           /* calls new(FIXFIXATOM).*/
extern struct conscell   * newrealop();             /* calls new(REALATOM).  */
extern struct conscell   * nreverse();              /* destructive 'reverse' */
extern struct conscell   * reverse();               /* top level reverse     */
extern struct conscell   * ReadExpression();        /* read S expr from port */
extern struct conscell   * takenewlist();           /* used by 'input'       */
extern struct conscell   * topcopy();               /* make top level copy   */
extern struct conscell   * putprop();
extern struct conscell   * pairlis();
extern struct hunkcell   * ListToHunk();
extern struct conscell   * assoc();
extern struct conscell   * buREPsopen();
extern struct conscell   * budefun();
extern struct conscell   * buppform();

  /*** VOID OR SIMPLE FUNCTIONS ***/

extern void    funcinstall();
extern char ** FindReferent();            /* need for relocation */
extern int     gather();
extern void    prettyprint();
extern void    pushvariables();
extern char *  getenv();
extern int     GetOption();
extern char *  heapget();
extern void    ierror();
extern void    initmem();
extern void    mark();
extern void *  malloc();
extern long int     memorystatus();
extern void    printlist();
extern void    ResetTrace();
extern void    SetOption();
extern int     scan();
extern void    serror();
extern int     strcmp();
extern void    printatom();
extern void    marklist();
extern void    ioerror();
extern int     GetFloat();
extern int     GetString();
extern int     GetFix();
extern int     ExtractArray();
extern void    HoldStackOperation();
extern void    gerror();
extern void    bindvar();
extern int     liulength();
extern void    lierrh();
extern void    unbindvar();
extern int     iscadar();
extern void    buresetlog();
extern int     equal();
extern int     GetNumberOrString();
extern void    removehunk();
extern void    fatalerror();
extern void    removeatom();
extern void    putclisptos();
extern void    UpError();
extern void    catcherror();
extern int     TestForNonNil();
extern int     ScanReset();
extern void    printstats();
extern void    InitMarkStack();
extern void    markstack();
extern void    ExpandEscapesInto();
extern void    removestring();
extern void    markclisp();
extern void    SetLongVar();
extern void    initerrors();
extern void    InstallSpecialAtoms();
extern void    popvariables();
extern void    lifreelist();
extern int     ScanSetLineNum();
extern int     hash();
extern int     CopyCellIfPossible();
extern int     syserror();
extern int     isalphatoken();
extern void    ScanSetSynClassMacro();
extern int     liargc;
extern char  **liargv;
extern int     lillev;                    /* lexical level for (go..) validation */

  /*** GLOBAL VARIABLES ***/

extern jmp_buf  env;                      /* break or error return point */
extern int      errno;                    /* normal libc global error number */
extern int      marking;                  /* '1' means GC is marking 0 not */
extern FILE  *  zapee;                    /* last input port for (zapline) */
extern struct conscell *lifreecons;       /* list of all free cons cells */
extern int      liScanLineNum;            /* line number stored by scanner */

/****************************************************************************
 ** The interpreter needs certain 'built' in atoms to be predefined prior  **
 ** to the running of the eval function. Pointers to some of these atoms   **
 ** are held to make testing faster. For example to check if an atom is    **
 ** 't', 'lambda', 'quote' are frequent operations and by holding the ptr  **
 ** we save much loopup time in the core of the eval function() this is    **
 ** a major speed up factor.                                               **
 ****************************************************************************/
 extern struct alphacell *auxhold;                 /* "&aux" atom */
 extern struct alphacell *blexprhold;              /* his binding is lexpr arg list */
 extern struct alphacell *catchstkhold;            /* stack of catch environments */
 extern struct alphacell *errstkhold;              /* stack of errset environments */
 extern struct alphacell *gohold;                  /* "go" atom */
 extern struct alphacell *labelhold;               /* "label" atom */
 extern struct alphacell *lambdahold;              /* "lambda" atom */
 extern struct alphacell *lexprhold;               /* "lexpr" atom */
 extern struct alphacell *macrohold;               /* "macro" atom */
 extern struct alphacell *macroporthold;           /* his binding is macro read port */
 extern struct alphacell *nilhold;                 /* dummy not used at moment */
 extern struct alphacell *nlambdahold;             /* "nlambda" atom */
 extern struct alphacell *optionalhold;            /* "&optional" atom */
 extern struct filecell  *piporthold;              /* "piport" atom (stdin) */
 extern struct filecell  *poporthold;              /* "poport" atom (stdout) */
 extern struct alphacell *quotehold;               /* "quote" atom */
 extern struct alphacell *resthold;                /* "&rest" atom */
 extern struct alphacell *returnhold;              /* "return" atom */
 extern struct alphacell *thold;                   /* "t" atom */

/***************************************************************************************
 ** Allocate a new cell type effeciently. We access the freecons list directly and if **
 ** no cells are available THEN we call new(type) knowing that it will GC. This saves **
 ** a function call per cell allocation. This is mostly used by the liceval.c         **
 ***************************************************************************************/

#define NEW(type, var)                       \
        { if (var = lifreecons) {            \
              lifreecons = lifreecons->carp; \
              var->celltype = type;          \
          } else                             \
              var = new(type);               \
        }

/***************************************************************************************
 ** Allocate a new CONS cell effeciently. We access the freecons list directly and if **
 ** no cells are available THEN we call new(type) knowing that it will GC. This saves **
 ** a function call per cell allocation. This is mostly used by the liceval.c. Since  **
 ** we know the cell is a CONS cell and we know that the GC routines reset the type to**
 ** CONSCELL we do not have to make the var->celltype = type assignment and squeeze a **
 ** small improvment out of the above macro.                                          **
 ***************************************************************************************/

#define NEWCONS(var)                         \
        { if (var = lifreecons)              \
              lifreecons = lifreecons->carp; \
          else                               \
              var = new(CONSCELL);           \
        }

/***************************************************************************************
 ** Allocate a new fixcell  effeciently. We access the freecons list directly and if  **
 ** no cells are available THEN we call new(type) knowing that it will GC. This saves **
 ** a function call per cell allocation. This is mostly used by the liceval.c         **
 ***************************************************************************************/

#define NEWINTOP(value, var)                 \
        { if (var = lifreecons) {            \
              lifreecons = lifreecons->carp; \
              var->celltype = FIXATOM;       \
          } else                             \
              var = new(FIXATOM);            \
          FIX(var)->atom = value;            \
        }

/******************************************************************************************
 ** Given a pointer to a short consisting of two bytes return an integer that is         **
 ** signed as per the stored short. This is used to extract the <lit> and <displacement> **
 ** parts of instructions when running.                                                  **
 ******************************************************************************************/

#if !defined(SUN)
#    define XSHORT(ip) (((*((signed char *)ip)) << 8) | (*((ip)+1) & 0xff))
#else
#    define XSHORT(ip) (((*(ip)) << 8) | (*((ip)+1) & 0xff))
#endif

/******************************************************************************************
 ** Byte coded interpreter instructions and their op codes. If you change this ALL code  **
 ** LISP code must be recompiled since instructions will be garbage.                     **
 ******************************************************************************************/

#define OP_ARG               1
#define OP_ARGQ              2
#define OP_ATOM              3
#define OP_CALL              4
#define OP_CALLNF            5
#define OP_CAR               6
#define OP_CDR               7
#define OP_CONS              8
#define OP_COPYFIX           9
#define OP_DDEC              10
#define OP_DEC               11
#define OP_DUP               12
#define OP_EQ                13
#define OP_FIXP              14
#define OP_FLOATP            15
#define OP_HUNKP             16
#define OP_INC               17
#define OP_JEQ               18
#define OP_JNIL              19
#define OP_JNNIL             20
#define OP_JUMP              21
#define OP_JZERO             22
#define OP_LENGTH            23
#define OP_LISTIFY           24
#define OP_LISTP             25
#define OP_NULL              26
#define OP_NUMBP             27
#define OP_POP               28
#define OP_PUSHL             29
#define OP_PUSHLV            30
#define OP_PUSHNIL           31
#define OP_PUSHT             32
#define OP_PUTCLISP          33
#define OP_RCALL             34
#define OP_RETURN            35
#define OP_SETQ              36
#define OP_SPOP              37
#define OP_SPUSHARGS         38
#define OP_SPUSHLEX          39
#define OP_SPUSHNIL          40
#define OP_SPUSHWARG         41
#define OP_STORR             42
#define OP_STRINGP           43
#define OP_ZEROP             44
#define OP_ZPOP              45
#define OP_ZPUSH             46
#define OP_MAX_OP            47   /* no op code may be greater than or equal to this */

/******************************************************************************************
 ** Byte coded interpreter instruction kinds.  These define how many extra bytes are used**
 ** by the op code and what they are. The assembler uses this to create the pure byte    **
 ** codes.                                                                               **
 ******************************************************************************************/

#define OPK_STACK     0           /* this kind of instruction is a pure stack operation */
#define OPK_BRANCH    1           /* this is a branch instruction kind */
#define OPK_LIT1      2           /* this is a stack operation that uses a single literal arg */
#define OPK_LITN1     3           /* has a <n> and <lit> arg */
#define OPK_LIT2      4           /* has two <lit> args */
#define OPK_INTERNAL  5           /* state change operation, arguments are internal to byte machine */
#define OPK_N1        6           /* operation uses a single <n> argument */





