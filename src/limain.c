/* EDITION AD09, APFUN PAS.821 (92/04/23 15:06:44) -- CLOSED */                 
/****************************************************************************
 **             PC-LISP (C) 1986 Peter Ashwood-Smith                       **
 **             MODULE: MAIN                                               **
 **------------------------------------------------------------------------**
 **    The lisp parser module contains the routines takelist and printlist.**
 ** These are list I/O functions. This module also controls how atoms are  **
 ** hashed and provides some access routines to the "oblist" or hash table.**
 ** Atoms are all unique objects if they have a different print name. They **
 ** are hashed into the hash table and collisions are chained off the atom **
 ** struct field 'link'. This means that if we run (eq 'a 'a) we will get  **
 ** true but (eq 2.2 2.2) will return 'nil because real numbers are not put**
 ** into this hash structure. Each number, flonum or fixnum is a unique    **
 ** object. Here is an example of simple list structure. Where [*|*] is a  **
 ** cons cell and everything else is a literal alpha atom.                 **
 **                                                                        **
 **      (a b c)               [*|*]--->[*|*]---->[*|/]                    **
 **                             |        |         |                       **
 **                            |a|      |b|       |c|                      **
 **                                                                        **
 **                                                                        **
 **      (now (is . the))     [*|*]------->[*|X]                           **
 **                             |            |                             **
 **                           |now|        [*|*]                           **
 **                                        /   \                           **
 **                                     |is|   |the|                       **
 **                                                                        **
 ** Note that we use [*|*] to depict a cons cell. Note that a dottet pair  **
 ** has two atoms assiciated with its two pointers car and cdr. Note that  **
 ** we also provide the ' facility for the shortcut of quoting atoms and   **
 ** lists. In particular we parse 'a as (quote a) and '(a b c)    as       **
 ** (quote (a b c)). This code also has the main() function. Which controls**
 ** which file is being read from, and where errors are to be vectored. We **
 ** use the Unix standard library functions setjmp and longjmp to handle   **
 ** jumping from a deep recursive error back to the command line loop. The **
 ** target of this jump is always the if (setjmp()) statement in the main()**
 ** function.                                                              **
 ****************************************************************************/

#include        <stdio.h>
#include        <signal.h>
#include        <math.h>
#include        "lisp.h"
#define         ISATOM()    ((curtok == 6)||(curtok == 7)||(curtok == 10))
#define         ISALPHA()   (curtok == 6)
#define         ISSTRING()  (curtok == 10)
#define         ISREAL()    (curtok == 7)
#define         ISOPEN()    (curtok == 1)
#define         ISCLOSE()   (curtok == 2)
#define         ISDOT()     (curtok == 5)
#define         NOTEOTOK()  (curtok != 0)
#define         ISEOTOK()   (curtok == 0)
#define         ISSRMACRO() (curtok == 11)
#define         ISRMACRO()  (curtok == 8)       /* the quote for the moment */
#define         ISUNKNOWN() (curtok == 12)

#define         ECHO         1                  /* processfile() flags */
#define         NOECHO       0                  /* parameter */

static  int curtok;                       /* we make call curtok=scan(buff)  */
static  char buff[MAXATOMSIZE+4];         /* this is where tok string goes   */
static  struct conscell *inlist;          /* list currently being parsed.    */
static  int PrettyWidth = PRETTYWIDTH;    /* pretty printing current width   */
int     lillev = 0;                       /* lexical level is currently 0    */
jmp_buf env;                              /* setjump environment area.       */

/*****************************************************************************
 ** Following is the code for the storage of the atoms. Atoms are of two    **
 ** types. ALPHA and REAL atoms. Alpha atoms are just strings of characters **
 ** whereas REAL atoms are binary floating point numbers. A real number only**
 ** needs a fixed storage so is placed in a realcell which is of fixed size **
 ** The alpha cell even though it appears to be of fixed size is actually of**
 ** variable size. We store the string by 'tacking' it onto the cell. This  **
 ** is done by allocating as much space as is needed for the cell and its   **
 ** text string. These two cells are differentiated by their celltype field.**
 ** Atoms are stored such that if two alpha atoms have the same strings they**
 ** do not occupy different storage locations. That is they are EQ. They are**
 ** at the same memory location. Real atoms are never EQ, ie they occupy a  **
 ** different cell regardless of their values. The ALPHA and STRING atoms   **
 ** are stored in a hash table called atomtable its elements are pointers   **
 ** to lists of cons cells (buckets) and we store in the CAR of these the   **
 ** acutual object, the cdr points to the next object in the bucket. This   **
 ** allows several different objects to be placed in the same table with    **
 ** different hashing functions for each object. But space is used well.    **
 *****************************************************************************/

struct conscell *atomtable[ALPHATABSIZE];    /* table of buckets  */

/**********************************************************
 ** function initatoms(): reset the atoms lists to null  **
 **********************************************************/
initatoms()
{   int i;
    extern struct conscell *atomtable[];
    for(i=0; i < ALPHATABSIZE; atomtable[i++]=LIST(NULL));
}

/***************************************************************************
 ** function FindReferent(s) Find the atom,string or hunk hoding  a ptr to**
 ** heap @ 's' and return a pointer to this pointer. This is used by the  **
 ** heap   compactor to relocate this block  in mman.c by InformRelocating**
 ** We just hash into the table and walk down the bucket list looking for **
 ** a string, atom or hunk atom that owns this pointer 's'.               **
 ***************************************************************************/
 char **FindReferent(s,n)
 char *s; int n;
 {    struct conscell *l,*o;
      l = atomtable[hash(s,n)];
      while(l != NULL)
      {    if (l->carp!=NULL)
           {   o = l->carp;
               switch(o->celltype)
               {  case ALPHAATOM : if (ALPHA(o)->atom == s)
                                       return(&(ALPHA(o)->atom));
                                   break;
                  case STRINGATOM: if (STRING(o)->atom == s)
                                       return(&(STRING(o)->atom));
                                   break;
                  case HUNKATOM  : if (HUNK(o)->atom == s)
                                       return(&(HUNK(o)->atom));
                                   break;
                  default        : fatalerror("FindReferent");
               };
           };
           l = l->cdrp;
      };
      return(NULL);
 }

/***************************************************************************
 ** functions lookupatom() and insertatom(): This pair will search for and**
 ** insert an atom into the hash table. A pointer is returned to the found**
 ** or inserted element. When we search we hash to the correct location   **
 ** and follow the chain of cdrs  with this hash value. When found we end **
 ** and return a pointer to the atom. If nonexistent we return null. When **
 ** we insert, we place it at the head of the bucket list. Lookupatom will**
 ** look for atoms whose interned bit is set to 'kind' this makes it more **
 ** generally usefull throughout the rest of the program.                 **
 ***************************************************************************/
 struct alphacell *lookupatom(s,kind)
 char *s; int kind;
 {    register struct conscell *l, *t; register struct alphacell *at;
      if (strcmp(s,"nil") == 0) return(NULL);
      for(t = l = atomtable[hash(s,strlen(s)+1)]; l != NULL; l = l->cdrp) {
           if (((at = ALPHA(l->carp))!=NULL) && (at->celltype == ALPHAATOM)) {
              if (at->interned == kind)
                  if (strcmp(s,at->atom)==0) {
                      l->carp = t->carp;         /* splay to front of list */
                      t->carp = LIST(at);
                      return(at);
                  }
           }
      }
      return(NULL);
 }

/***************************************************************************
 ** Insertatom - lookup hash bucket for atom and put it into the table at **
 ** this place. Set all fields to default values. The permsetting is a bit**
 ** to indicate if the atom can be reclaimed. Special case is "nil". We   **
 ** always return NULL from lookupatom("nil") so that the routine will    **
 ** call insertatom("nil",NOT_PERM) when it scans "nil" or if "nil" ever  **
 ** gets constructed. We then intercept the insertion and return NULL. If **
 ** the call is insertatom("nil",PERM) we do not intercept it.  Note the  **
 ** heapget() call is made before the newalpha() call because if the heap **
 ** call were to follow and cause GC the alpha would have all its fields  **
 ** marked but they are uninitialized so this would crash the system. This**
 ** function will create a atom, either interned or uninterned but the    **
 ** caller must call lookupatom first before calling insertatom if an     **
 ** interned atom is too be created. See CreateInternedAtom....           **
 ***************************************************************************/
 struct alphacell *insertatom(s,permsetting)
 char *s; int permsetting;
 {    struct conscell *n; struct alphacell *k;
      int h,len; char *temp;
      if ((permsetting == NOT_PERM)&&(strcmp(s,"nil")==0))
           return(NULL);
      push(k);                          /* don't want k to go on new() call */
      h = hash(s,len = strlen(s)+1);
      temp = heapget(len);              /* CRITICAL order heapget&newalpha! */
      k = ALPHA(newalpha());
      k->atom = temp;
      strcpy(k->atom,s);
      k->celltype = ALPHAATOM;
      k->permbit = permsetting;
      k->interned = INTERNED;
      k->func = NULL;
      k->fntype = FN_NONE;
      k->tracebit = TRACE_OFF;
      k->valstack = k->proplist = NULL;
      k->iscadar = iscadar(s);          /* flag 'c{a|d}+r' atoms */
      k->botvaris = LOCALVAR;           /* it will be popped on error jump */
      n = new(CONSCELL);                /* make new con cell for bucket entry*/
      n->carp = LIST(k);                /* car points to new atom cell */
      n->cdrp = atomtable[h];           /* cdr is link in the overflow bucket */
      atomtable[h] = n;                 /* install into hash table, now we are*/
      xpop(1);                          /* ok k&n safely visible from table */
      return(k);
 }

/***************************************************************************
 ** CreateInternedAtom - intern the atom with string print name 's'. We   **
 ** look it up in the hash table. If it is there and the atom that is ther**
 ** is not interned we just return it. If however it is either not there  **
 ** or is there but is an uninterned atom, we must create a new one. Must **
 ** be careful of s = "nil" because insertatom will return NULL.          **
 ***************************************************************************/
struct alphacell *CreateInternedAtom(s)
char *s;
{    register struct alphacell *r;
     if ((r = lookupatom(s,INTERNED)) != NULL)
          return(r);
     if ((r = insertatom(s,NOT_PERM)) != NULL)
          r->interned = INTERNED;
     return(r);
}

/***************************************************************************
 ** CreateUninternedAtom - just insert a new NOT_PERM ie (gc'able) atom   **
 ** with perm bit set uninterned.  Watch out for s = "nil" get NULL back. **
 ***************************************************************************/
struct alphacell *CreateUninternedAtom(s)
char *s;
{     register struct alphacell *r;
      if ((r = insertatom(s,NOT_PERM)) != NULL)
           r->interned = NOT_INTERNED;
      return(r);
}

/***************************************************************************
 ** Insertstring - Create a new string cell. We just hash into the atom   **
 ** table and link in a new bucket to the string cell. Then set up the    **
 ** string cell fields so that it points into some heap space into which  **
 ** we copy the string 's' that is passed to us. Note that strings are    **
 ** unique objects, we always create a new table entry for one. Special   **
 ** note CRITICAL heapget&new order, think about GC between them in the   **
 ** opposite order! This is especially dangerous in insertatom where the  **
 ** atom fields have not been initialized yet and GC will follow these    **
 ** uninitialized fields.                                                 **
 ***************************************************************************/
 struct stringcell *insertstring(s)
 char *s;
 {    struct stringcell *k; struct conscell *n; int h,len;
      char *temp, swork[MAXATOMSIZE];
      push(k);
      h=hash(s,len=strlen(s)+1);
      if (len > MAXATOMSIZE)
          fatalerror("insertstring");         /* this should not be possible */
      memcpy(swork,s,len);                    /* must copy heapget may move it! */
      temp = heapget(len);
      k = STRING(new(STRINGATOM));            /* get a string atom cell */
      k->atom = temp;                         /* allocate space for string */
      memcpy(k->atom,swork,len);              /* copy it to heap space */
      n = new(CONSCELL);                      /* get a new bucket link cell */
      n->carp = LIST(k);                      /* link stringcell into bucket*/
      n->cdrp = atomtable[h];                 /* list at head of list for */
      atomtable[h] = n;                       /* this table entry */
      xpop(1);
      return(k);
 }

/***************************************************************************
 ** function removeatom(a) This function will look up at atom in the hash **
 ** table and remove it from its bucket position in the chain. root is a  **
 ** pointer to the location containing the address of the cell that we    **
 ** are removing. It is initially set to point to the array entry (head). **
 ** Note that this is called by the gather() function in mman.c to remove **
 ** an atom from the atom table after its print name heap space has been  **
 ** freed. The removestring function does exactly the same with a strcell **
 ***************************************************************************/
 removeatom(a)
 struct alphacell *a;
 {    struct conscell *l,**root;
      l = *(root = &atomtable[hash(a->atom,strlen(a->atom)+1)]);
      while((l != NULL)&&(l->carp != LIST(a)))
      {    root = &(l->cdrp);
           l = l->cdrp;
      };
      if (l != NULL) *root = l->cdrp;
 }

/***************************************************************************
 ** function removestring(a) -function will look up at string cell in the **
 ** hash table and remove it from its bucket position. Almost identical to**
 ** the removeatom function above but hashes using a different cell type. **
 ** Is called by gather() in mman.c when an unaccessable string is found, **
 ** The strings heap space will have been freed prior to this call by the **
 ** memory manager function gather().                                     **
 ***************************************************************************/
 removestring(a)
 struct stringcell *a;
 {    struct conscell *l,**root;
      l = *(root = &atomtable[hash(a->atom,strlen(a->atom)+1)]);
      while((l != NULL)&&(l->carp != LIST(a)))
      {    root = &(l->cdrp);
           l = l->cdrp;
      };
      if (l != NULL) *root = l->cdrp;
 }

/***************************************************************************
 ** Inserthunk(n) - Create a new hunk of 'n' elements. 'n' must be 1 to   **
 ** 126. First we allocate a hunk cell, then we allocate enough space for **
 ** 'n' pointers from the heap. This space is used to store 'raw' pointers**
 ** The pointers are unaligned, they must be loaded byte by byte and they **
 ** point to cons cells. Thus a full hunk looks like this:                **
 **                                                                       **
 **     [HUNKATOM|MARKBIT|SIZE|*]                                         **
 **                            |                                          **
 **                            V                                          **
 **                            [*|*|*|*|*|......*]                        **
 **                             |  .... |  .... |                         **
 **                             V       V       V                         **
 **                           [*|*]   [*|*]   [*|*]                       **
 **                           /  \    /  \     \  \                       **
 **                         #0   #1  #n  #n+1  .......                    **
 **                                                                       **
 **     In otherwords a hunkcell's size field tells elements are in the   **
 ** hunk. The heap space will be exactly roundup(n/2) * sizeof(char *) ie **
 ** one pointer for every pair of adjacent elements in the hunk. These    **
 ** pointers then point to cons cells, the car of which points to the ele-**
 ** ment with address n/2 and the cdr points to element with address n/2+1**
 ** Note that the pointers in the data space must point to the same cons  **
 ** cells, these are never EVER altered otherwise the hashing function    **
 ** will fail to work. These cons cells must stay with the hunk from its  **
 ** birth to it's death and never be changed. Their car and cdr pointers  **
 ** may of course be altered to change the elements of the hunk.          **
 ***************************************************************************/
 struct hunkcell *inserthunk(n)
 int n;
 {    struct hunkcell *h; char *heap,*p; int len,bytes,v,i;
      struct conscell *l;
      if ((n<=0)||(n>(2*(MAXATOMSIZE/sizeof(char *)))))
         return(NULL);
      push(h);                                /* marking point for G/C */
      len = CEILING_DIV_2(n);                 /* base size is roundup(n/2) */
      bytes = len * sizeof(char *);           /* byte size of base ptr space */
      heap = heapget(bytes);                  /* get storage base pointers */
      h = HUNK(new(HUNKATOM));                /* allocate a hunk cell */
      h->size = n;                            /* set hunk size in elements */
      h->atom = heap;                         /* set hunk's base ptr space */
      for(i=0;i<len;i++)                      /* BASE MUST BE NULL INCASE */
          WriteHunkBase(h,i,NULL);            /* OF GC IN FOLLOWING new() */
      for(i=0;i<len;i++)                      /* point bases to cons cells*/
          WriteHunkBase(h,i,new(CONSCELL));   /* point base[i] to new cons */
      v = hash(heap,bytes);                   /* get hash bucket address */
      l = new(CONSCELL);                      /* make a new bucket entry*/
      l->carp = LIST(h);                      /* and link hunk into bucket*/
      l->cdrp = atomtable[v];                 /* rest of bucket chain */
      atomtable[v] = l;                       /* and into table */
      xpop(1);                                /* get rid of G/C mark point */
      return(h);
 }

/***************************************************************************
 ** function removehunk(a) This function will look up the hunk in the hash**
 ** table and remove it from its bucket position in the chain. root is a  **
 ** pointer to the location containing the address of the cell that we    **
 ** are removing. It is initially set to point to the array entry (head). **
 ** Note that this is called by the gather() function in mman.c to remove **
 ** a hunk  from the atom table after its base ptr   heap space has been  **
 ** freed. This is eactly like removestring and removeatom.               **
 ***************************************************************************/
 removehunk(a)
 struct hunkcell *a;
 {    struct conscell *l,**root; register int n;
      n = CEILING_DIV_2(a->size) * sizeof(char *);
      l = *(root = &atomtable[hash(a->atom,n)]);
      while((l != NULL)&&(l->carp != LIST(a)))
      {    root = &(l->cdrp);
           l = l->cdrp;
      };
      if (l != NULL) *root = l->cdrp;
 }

/**************************************************************************
 ** WriteHunkBase(hunk,index,ptr) : Make hunk base pointer #index be set **
 ** to ptr. We just do hunk->atom[index] = ptr; but in an alignment indep**
 ** endent way. This is necessary because the base pointer array may not **
 ** be aligned correctly because heap space may move about in memory.    **
 ** We also use it to set the base to NULL pointers. Should be totally   **
 ** portable to any of the cyber/prime etc. Machines with strange ptrs.  **
 **************************************************************************/
 WriteHunkBase(hunk,index,ptr)
 struct hunkcell *hunk; int index; char *ptr;
 {      register char *s,*d;  register int n;
        n = sizeof(char *);
        d = hunk->atom + index*n;
        s = (char *) &ptr;
        while(n--) *d++ = *s++;
 }

/**************************************************************************
 ** ReadHunkBase(hunk,index) : Returns hunk->atom[index] base pointer but**
 ** does so in a machine/alignment independent way. Because Hunk Base ptr**
 ** array may be moved around in memory for garbage collection/compaction**
 **************************************************************************/
 struct conscell *ReadHunkBase(hunk,index)
 struct hunkcell *hunk; int index;
 {      register char *s,*d;  register int n;
        struct conscell *ret;
        n = sizeof(char *);
        s = hunk->atom + index*n;
        d = (char *) &ret;
        while(n--) *d++ = *s++;
        return(ret);
 }

/**************************************************************************
 ** GetHunkIndex(h,i) Returns a pointer to a pointer to the cell where   **
 ** hunk element 'i' is stored. We First compute the base pointer for the**
 ** index, then we use the remainder to decide if it is the car or cdr   **
 ** pointer of this base element. No legality checks are done!           **
 **************************************************************************/
struct conscell **GetHunkIndex(h,i)
struct hunkcell *h; int i;
{      register int index1,index2;
       register struct conscell *c;
       index1 = i>>1;                   /* index1 says which consc cell */
       index2 = i%2;                    /* index2 says car or cdr of cell */
       c = ReadHunkBase(h,index1);      /* c is cons cell for index1 */
       if (index2)                      /* if odd element at cdr ptr */
           return(&(c->cdrp));
       return(&(c->carp));              /* else even so elem at car ptr*/
}

/**************************************************************************
 ** markhunk(h) : Mark the hunk and each element in it. We first mark the**
 ** hunkcell itself, then mark each of the base pointer cons cells and   **
 ** they will mark whatever they point to. We compute the number of base **
 ** pointers in the hunk, then call ReadHunkBase to get the i'th pointer **
 ** Note that a call to markhunk(NULL) may occur because of the order of **
 ** construction of an array and a possible GC during the construction,  **
 ** we must therefore test this or we very occasionally get a crash.     **
 **************************************************************************/
markhunk(h)
struct hunkcell *h;
{      register int basesize,i;
       if (h == NULL) return;
       h->markbit = SET;
       basesize = CEILING_DIV_2(h->size);  /* basesize=roundup(h->size/2)*/
       for(i = 0; i < basesize; i++)
           marklist(ReadHunkBase(h,i));
}

/**************************************************************************
 ** markclispliterals(l): Will walk the literals array 'literal' under   **
 ** a compiled LISP cell 'clisp' and mark each of the cells in it. Note  **
 ** that the literals array is prefixed with an integer giving its length**
 ** in bytes so we look up 'n' and loop accordingly. We do not mark the  **
 ** first clisp literal because it points to the clisp from which we     **
 ** just came so this would be a waste of time.                          **
 **************************************************************************/
markclispliterals(l)
struct conscell **l;
{      register int n = (*((int *)l - 1))/sizeof(struct conscell *);
       --n; l++;
       while(--n >= 0)
           marklist(*l++);
}

/*****************************************************************************
 ** marklist(pres) : Will perform a postorder traversal of the list 'pres'  **
 ** issuing a switch for each major node kind it finds. The switch will then**
 ** cause a goto up if we have reached a leaf and must back up. Otherwise   **
 ** nothing indicating    we are on a cons cell and must continue to descend**
 ** and mark. This algorithm is a link inversion traversal taken from the   **
 ** Text: Data Structure Techniques, Thomas A. Standish. from page 76. Sorry**
 ** it is messy and unstructured but I could not figure out as efficient a  **
 ** structured solution as this tangled web. The algorithm proceeds like    **
 ** this. First we descend all the way down the 'left' ie the car chains    **
 ** inverting pointers as we descend leaving a trail so we can get back up. **
 ** We stop when we reach a cell which is already marked or when we reach   **
 ** a leaf. The leaf cases may recursively call for marking of lists. This  **
 ** handles cases such as symbol property, file truename etc. This recursion**
 ** is never going to go very deep. If we reach a leaf we jump to the code  **
 ** that climbs back up the tree using the inverted links. Once the left    **
 ** most leaf is handles we back up and go right once, then all the way left**
 ** again. Whenever we go right, we set the travbit indicating that the node**
 ** , left and right subtrees have been visited. The 'up' code follows the  **
 ** backward pointers as long as the travbit is set, it resets them on the  **
 ** way up. In doing this it 'pops' the stack of all visited nodes. When it **
 ** reaches a non set travbit it jumps to the 'right' code to handle the    **
 ** unvisited right subtree. etc. etc. etc.                                 **
 *****************************************************************************/
marklist(pres)
struct conscell *pres;
{      register struct conscell *prev,*next;
       if (pres == NULL) return;
       prev = NULL;
left:  if (pres->markbit == SET) goto up;
       pres->markbit = SET;
       switch(pres->celltype)
       {   case STRINGATOM: case REALATOM: case FIXFIXATOM: case FIXATOM:
                            goto up;
           case FILECELL  : marklist(PORT(pres)->fname);
                            goto up;
           case HUNKATOM  : markhunk(HUNK(pres));
                            goto up;
           case ARRAYATOM : marklist(ARRAY(pres)->info);
                            marklist(ARRAY(pres)->base);
                            goto up;
           case ALPHAATOM : marklist(ALPHA(pres)->valstack);
                            marklist(ALPHA(pres)->proplist);
                            if ( FN_ISUS(ALPHA(pres)->fntype) || FN_ISCLISP(ALPHA(pres)->fntype) )
                               marklist(ALPHA(pres)->func);
                            goto up;
           case CLISPCELL:  markclispliterals(CLISP(pres)->literal);
                            goto up;
       };
       if ((next = pres->carp) != NULL)
       {   pres->carp = prev;
           prev = pres;
           pres = next;
           goto left;
       };
right: if ((next = pres->cdrp) != NULL)
       {   pres->travbit = SET;
           pres->cdrp = prev;
           prev = pres;
           pres = next;
           goto left;
       };
up:    if (prev == NULL) return;               /* backup up the tree using */
       if (prev->travbit == CLEAR)             /* the inverted tag bits */
       {   next = prev->carp;
           prev->carp = pres;
           pres = prev;
           prev = next;
           goto right;
       };
       next = prev->cdrp;
       prev->travbit = CLEAR;
       prev->cdrp = pres;
       pres = prev;
       prev = next;
       goto up;
}

/**************************************************************************
 ** mark(): Mark all cells that are important to the operation of the    **
 ** interpreter at this moment in time. First we mark the original input **
 ** list that eval is working on. Next we mark all partially constructed **
 ** lists that were stored in the markstack by eval as it decended the   **
 ** list recursively constructing the correct eval'ed list. At this point**
 ** all reachable cells will have been marked. However there may be some **
 ** global variables built in functions or user defined functions that   **
 ** require marking. Atoms with property must also not be lost and the   **
 ** property and values of such atoms should not be lost either.         **
 **************************************************************************/
mark()
{      int i; struct conscell *l; struct alphacell *c;
       markstack();
       markclisp();
       HoldStackOperation(MARK_STACK);
       for (i=0 ; i < ALPHATABSIZE; ++i) {
           l = atomtable[i];
           while (l != NULL) {
              l->markbit = SET;
              c = ALPHA(l->carp);
              if ((c!=NULL)&&(c->celltype == ALPHAATOM)) {
                 if (c->markbit != SET) {
                    if ((c->proplist!=NULL)||(c->valstack!=NULL)||(c->func!=NULL)) {
                         c->markbit = SET;
                         marklist(c->proplist);
                         marklist(c->valstack);
                         if ( FN_ISUS(c->fntype) || FN_ISCLISP(c->fntype) )
                             marklist(LIST((c->func)));
                    }
                 }
              }
              l = l->cdrp;
           }
       }
}


/****************************************************************************
 ** pushvariables and popvariables - These will handle the scope up and    **
 ** scope down needed when we enter a user defined function. pushvariables **
 ** takes a list of local vars and a list of values. It then pushes each   **
 ** value onto the 'valuestack' of each atom in the vars list. popvaribles **
 ** does the opposite. It just unwinds each valstack by 1. This scheme is  **
 ** the best possible scoping technique. It is order(1) lookup as fast as  **
 ** any compiled language. Vars and vals should be null at same time when  **
 ** we finish otherwise we have a bad function call. Note that the vars    **
 ** list may contain the atoms &optional and &aux. If &optinal is found the**
 ** parameters following it may not be present. If they are not they are   **
 ** bound to nil. If &aux is found the following variables are not passed  **
 ** parameters but local variables which are to be bound to nil.           **
 ****************************************************************************/
 pushvariables(vars,vals)
 struct conscell *vars,*vals;
 {      struct alphacell *at; struct conscell *temp;
        while((vars != NULL)&&(vals != NULL))
        {     at = (struct alphacell *)vars->carp;
              if ((at == NULL)||(at->celltype != ALPHAATOM))
              {    gerror("a formal parameter is not type symbol");
              };
              temp = new(CONSCELL);
              temp->carp = vals->carp;
              temp->cdrp = at->valstack;
              at->valstack = temp;
              vals = vals->cdrp;
              vars = vars->cdrp;
        };
        if ((vars == NULL)&&(vals == NULL)) return;
        gerror("actual parameters don't agree with formals");
 }


/****************************************************************************
 ** funcinstall(type,body,name,atom). This is the access function which is **
 ** responsible for the definition of a new function. The function is of   **
 ** type 'type' according to the FNT bits in lisp.h. It's body is 'body'   **
 ** which may be either a lisp expression or a pointer to an assembly lang **
 ** uage routine. This depends on the bits set in 'type'. The name of the  **
 ** function is stored in 'name' and if we know the atom pointer it is     **
 ** passed in 'atom'. This saves us looking up something that we already   **
 ** have direct access to.                                                 **
 ****************************************************************************/
 funcinstall(type,body,name,at)
 int type; struct conscell *(*body)(); char *name; struct alphacell *at;
 {      if (at == NULL)
        {  at = CreateInternedAtom(name);
           at->permbit = PERM;
        };
        at->fntype = type;
        at->func = body;
 }

/****************************************************************************
 ** popvariables(vars) We will pop one value from the valuestack for each  **
 ** of the variables in the list of atoms 'vars'. This is the end scope op.**
 ** It also frees up the shallow stack cons cells.                         **
 ****************************************************************************/
 popvariables(vars)
 struct conscell *vars;
 {      struct alphacell *at;
        register struct conscell *f;
        while(vars != NULL)
        {     at = (struct alphacell *)vars->carp;
              f = at->valstack;
              at->valstack = f->cdrp;
              f->cdrp = NULL;
	      f->carp = lifreecons;
	      lifreecons = f;
              vars = vars->cdrp;
        };
 }

/****************************************************************************
 ** bindvar and unbindvar: These are like the pushvariables and pop vars   **
 ** above but the operate on only one variable.                            **
 ****************************************************************************/
 bindvar(var,val)
 struct alphacell *var;
 struct conscell *val;
 {      struct conscell *temp;
        temp = new(CONSCELL);
        temp->carp = val;                             /* push var onto valstack */
        temp->cdrp = var->valstack;
        var->valstack = temp;
 }

/****************************************************************************
 ** bindlabel, just like bindvar but puts the lexical level in the stack   **
 ** cell for checking by (go).                                             **
 ****************************************************************************/
 bindlabel(var,val)
 struct alphacell *var;
 struct conscell *val;
 {      struct conscell *temp;
        temp = new(CONSCELL);
        temp->linenum = lillev;
        temp->carp = val;                             /* push var onto valstack */
        temp->cdrp = var->valstack;
        var->valstack = temp;
 }

/****************************************************************************
 ** unbindvar(var): Pop one value from the value stack for atom 'var' and  **
 ** free up the shallow stack cons cell.                                   **
 ****************************************************************************/
 unbindvar(var)
 struct alphacell *var;
 {      register struct conscell *f = var->valstack;
	var->valstack = f->cdrp;                      /* pop the valstack once */
        f->cdrp = NULL;
        f->carp = lifreecons;
        lifreecons = f;
 }

/****************************************************************************
 ** bindtonil(vars) : Like pushvariables except that all values are NIL    **
 ** this is used when a prog is invoked establish locals all bound to NIL  **
 ****************************************************************************/
 bindtonil(vars)
 struct conscell *vars;
 {      struct alphacell *at; struct conscell *temp;
        while(vars != NULL)
        {     at = (struct alphacell *)vars->carp;
              if ((at == NULL)||(at->celltype != ALPHAATOM))
              {    gerror("a local was not of type symbol");
              };
              temp = new(CONSCELL);
              temp->cdrp = at->valstack;
              at->valstack = temp;
              vars = vars->cdrp;
        };
 }

/****************************************************************************
 ** unwindscope(): Restore scope of all variables to their global values.  **
 ** we traverse the ALPHAHASH table traversing each bucket chain reading   **
 ** one atom at a time. We wind each valstack in until either it is nul if **
 ** the variable is a LOCAL VARIABLE otherwise we unwind until there is    **
 ** one variable left on the stack, the GLOBAL value of this variable.     **
 ****************************************************************************/
 unwindscope()
 {      int i; struct alphacell *a; struct conscell *t,*l;
        for(i=0;i < ALPHATABSIZE;i++)
        {   l = atomtable[i];
            while (l != NULL)
            {   if ((l->carp != NULL)&&(l->carp->celltype == ALPHAATOM))
                {   a = ALPHA(l->carp);
                    t = a->valstack;
                    if (a->valstack != NULL)
                    {   if (a->botvaris == LOCALVAR)
                            a->valstack = NULL;
                        else
                        {  while(t->cdrp != NULL) t = t->cdrp;
                           a->valstack = t;
                        };
                    };
                };
                l = l->cdrp;
            };
        };
 }

/***************************************************************************
 ** SetLongVar(s,val) : Set system variable 's' to have long integer value**
 ** 'val'. This is used to set the $gccount$ variable at the moment and   **
 ** will later be used for other system variable settings. If we try to   **
 ** set the value of a variable that is not present it is an error, or if **
 ** that variable has no system binding that is also an error.            **
 ***************************************************************************/
 SetLongVar(s,val)
 char *s; long int val;
 {    struct alphacell *a; struct conscell *l;
      if ((a=lookupatom(s,INTERNED))==NULL) goto ERR;   /* atom present ? */
      if (a->valstack == NULL) goto ERR;                /* bound ? */
      l = a->valstack;
      while(l->cdrp != NULL) l = l->cdrp;               /* find global binding */
      l->carp = newintop(val);                          /* rebind to 'val' */
      return;
 ERR: fatalerror("SetLongVar");
      exit(0);
 }

/***************************************************************************
 ** TestForNonNil(s,def)     : System variable 's' is being tested for non**
 ** nilness. If s is non-nil return 1 else return 0. If the variable does **
 ** not exist the default value is returned. This allows variables to be  **
 ** assumed non-nil if they do not exist, or to be assumed nil if they do **
 ** exits.                                                                **
 ***************************************************************************/
 int TestForNonNil(s,def)
 char *s; int def;
 {    struct alphacell *a;
      if ((a=lookupatom(s,INTERNED))==NULL)
          return(def);
      if (a->valstack != NULL)
          if (a->valstack->carp == NULL)       /* bound value is nil */
              return(0);                       /* so test fails */
      return(1);
 }

/***************************************************************************
 ** HoldStackOperation(flag) : If flag = COPY we will copy the top of the **
 ** mark stack into a holding stack. If flag = DUMP we will print the held**
 ** stack from top to bottom.  When we built the holdstack we will only   **
 ** try to pick up apply() and eval() stack pushed items. eval pushes a   **
 ** form like (func args) and apply pushes two elements func and args. We **
 ** construct the (func args) form for any apply data we find in stack.   **
 ** Again we must be careful of garbage collection when we call the new   **
 ** function because we will be called recursively and asked to mark the  **
 ** holdstack. Hence holdtop must be correct when new is called.          **
 ***************************************************************************/
 HoldStackOperation(flag)
 int flag;
 {      struct conscell *temp, *h; register int ix;
        static struct conscell *hold[HSSIZE];
        static int holdtop = 0;
        switch(flag)
        {   case COPY_STACK:
                 ix = emytop+1;
                 holdtop = 0;
                 while((ix < MSSIZE) && (holdtop < HSSIZE))
                     hold[holdtop++] = *mystack[ix++];
                 break;
            case DUMP_STACK:
                 temp = NULL;
                 putchar('\n');
                 for(ix = 0; ix < holdtop; ix++) {
                     printf("[] ");
                     h = hold[ix];
                     if (h && (h->celltype == FIXFIXATOM)) {
                         char *liuffnam(), *s;
                         s = liuffnam(h);
                         printf("(%s -\?\?\?-)", s ? s : "?");
                     } else
                         printlist(stdout,h,DELIM_ON,temp,NULL);
                     temp = h;
                     putchar('\n');
                 }
                 putchar('\n');
                 break;
            case MARK_STACK:
                 for(ix=0; ix < holdtop;) marklist(hold[ix++]);
                 break;
            default:
                 ierror("Internal - HoldStackOperation");
        };
 }


/***************************************************************************
 ** function CopyOblist(): This function will make a list of all interned **
 ** atoms from the atom table. We just traverse the table and the buckets **
 ** and construct a list 'l'ast and return it.  Push(l) so that if we run **
 ** out of memory it will be marked. We do not copy the atoms *return*    **
 ** or *go*, where * is the junk added to avoid accidental entry. These   **
 ** atoms should not get into lists that the user creats otherwise the    **
 ** prog function can do strange things. Ie consider (setq l (cdr l))     **
 ** where l is part of the oblist! (Have a look at how prog operates.)    **
 ***************************************************************************/
 struct conscell *CopyOblist()
 {      struct conscell *l,*n,*w;
        struct alphacell *a; int i;
        push(l); push(w);
        for(i=0;i < ALPHATABSIZE;i++)
        {   w = atomtable[i];
            while(w != NULL)
            {     a = ALPHA(w->carp);
                  if ((a!=NULL)&&(a->celltype == ALPHAATOM))
                  {   if ((a != gohold)&&(a != returnhold))    /* skip go and return */
                      {   if (a->interned == INTERNED)
                          {   n = new(CONSCELL);
                              n->carp = LIST(a);
                              n->cdrp = l;
                              l = n;
                          };
                      };
                  };
                  w = w->cdrp;
            };
        };
        fret(l,2);
 }

/***************************************************************************
 ** function HashStatus():   This function will make a list of real values**
 ** each which represents the number of collisions that have occured for  **
 ** each entry in the hash table. Ie the number in each link chain.       **
 ***************************************************************************/
 struct conscell *HashStatus()
 {      struct conscell *l,*n;
        struct conscell *a; long c; int i;
        push(l); push(n);
        for(i=0;i < ALPHATABSIZE;i++)
        {   c = 0L;
            for(a = atomtable[i]; a != NULL; a = a->cdrp) c += 1L;
            n = new(CONSCELL);
            n->carp = newintop(c);
            n->cdrp = l;
            l = n;
        };
        fret(l,2);
 }

/****************************************************************************
 ** MixedTypeCompare(a,b) : 'a' and 'b' are cells of type -number- ie they **
 ** may be either fixnum or flonum. We will do the comparisson for the mix-**
 ** tures of numeric type. Note that these comparissons are pretty slow!   **
 ****************************************************************************/
int MixedTypeCompare(a,b)
struct fixcell *a,*b;
{   if ((a==NULL)||(b==NULL)) return(MT_ERROR);
    if ((a->celltype == FIXATOM)&&(b->celltype  == FIXATOM))
    {    if (a->atom < b->atom)  return(MT_LESS);
         if (a->atom > b->atom)  return(MT_GREATER);
         if (a->atom == b->atom) return(MT_EQUAL);
    };
    if ((a->celltype == REALATOM)&&(b->celltype == REALATOM))
    {    if (REAL(a)->atom < REAL(b)->atom)  return(MT_LESS);
         if (REAL(a)->atom > REAL(b)->atom)  return(MT_GREATER);
         if (REAL(a)->atom == REAL(b)->atom) return(MT_EQUAL);
    };
    if ((a->celltype == FIXATOM)&&(b->celltype == REALATOM))
    {    if (a->atom < REAL(b)->atom)  return(MT_LESS);
         if (a->atom > REAL(b)->atom)  return(MT_GREATER);
         if (a->atom == REAL(b)->atom) return(MT_EQUAL);
    };
    if ((a->celltype == REALATOM)&&(b->celltype == FIXATOM))
    {    if (REAL(a)->atom < b->atom)  return(MT_LESS);
         if (REAL(a)->atom > b->atom)  return(MT_GREATER);
         if (REAL(a)->atom == b->atom) return(MT_EQUAL);
    };
    return(MT_ERROR);
}

/****************************************************************************
 ** GetNumberOrString(l,where) - like GetString except that if l is a fix  **
 ** or flonum the ascii representation of the number is returned. We use a **
 ** static buffer of 256 bytes for this expansion. If we find a flonum we  **
 ** must expand it to ascii the same way that printatom does this means we **
 ** must check to see if it has a simple long representation or not. See   **
 ** the printatom code for more comments on what is going on here.         **
 ****************************************************************************/
int GetNumberOrString(l,where)
struct conscell *l; char **where;
{   static char numbuf[MAXATOMSIZE]; double v;
    if (l != NULL)
    {   switch(l->celltype)
        { case ALPHAATOM:
                    *where = ALPHA(l)->atom;
                    return(1);
          case STRINGATOM:
                    *where = STRING(l)->atom;
                    return(1);
          case FIXATOM:
                    sprintf(numbuf,"%ld",FIX(l)->atom);
                    *where = numbuf;
                    return(1);
          case REALATOM:                           /* mimic the patom code */
                    v = REAL(l)->atom;
                    if (CanStoreInLong(v))
                        sprintf(numbuf,"%.1lf",v);
                    else
                        sprintf(numbuf,"%lg",v);
                    *where = numbuf;
                    return(1);
          default:
                    return(0);
        };
    };
    *where = "nil";                        /* NULL is nil is "nil" */
    return(1);
}

/****************************************************************************
 ** RunReadMacro(port,c) Will run a read macro to port 'port' whose print  **
 ** name is in the string pointed to by 'c'. We handle the quote atom inter**
 ** nally because it is quicker. Otherwise if we are not dealing with the  **
 ** quote read macro so we get the atom whose current binding represents   **
 ** read macro body (not a lambda expression, we have stripped off this in **
 ** set syntax). Once we have the body that is to be run, we bind the port **
 ** so that the (read) and (readc) statements in the body will know which  **
 ** port to read from. We then eval this body, unbind the port and return. **
 ****************************************************************************/
struct conscell *RunReadMacro(port,c)
struct filecell *port; char *c;
{    struct conscell *n, *TakeSexpression();
     struct alphacell *at;
     if (*c == '\'')
     {   push(n);
         n = new(CONSCELL);
         n->carp = LIST(CreateInternedAtom("quote"));
         curtok = scan((zapee = port->atom),buff);      /* must log for (zapline) */
         n->cdrp = new(CONSCELL);
         n->cdrp->carp = TakeSexpression(port);
         fret(n,1);
     };
     if ((at = lookupatom(c,INTERNED)) != NULL)         /* get macro atom */
     {    if (at->valstack != NULL)                     /* its value is body */
          {   n = at->valstack->carp;                   /* n is macro to run */
              bindvar(macroporthold,port);              /* bind for read(c) */
              n = eval(n);                              /* run it */
              unbindvar(macroporthold);                 /* drop binding */
              return(n);
          };
     };
     gerror("read macro expression not found");
}

/****************************************************************************
 ** MakePort(fd,at) - Create a new open port cell for stream 'fd' with true**
 ** name whose string is the print name of atom 'at'. Just allocate it and **
 ** link in the fields.                                                    **
 ****************************************************************************/
struct conscell *MakePort(fd,at)
FILE *fd;
struct alphacell *at;
{      struct filecell *fcell;
       xpush(at);
       fcell = PORT(new(FILECELL));
       fcell->atom = fd;
       fcell->fname = at;
       fcell->issocket = 0;
       fcell->state = 0;         /* unknown read/write state yet */
       fcell->linenum = 1;
       xpop(1);
       return(LIST(fcell));
}

/****************************************************************************
 ** CanStoreInLong(d) Will return '1' if the double 'd' can be stored in a **
 ** long without loss of precision. If the double is greater than MAXLONG  **
 ** or less than MINLONG then it cannot be stored in a double. If it is in **
 ** this range then we check to see if it is whole or not. If non whole    **
 ** it cannot be stored in a long without loosing the fraction.            **
 ****************************************************************************/
int CanStoreInLong(d)
double d;
{   long l;
    if ((d > MAXLONG)||(d < MINLONG)) return(0);
    l = (long) d;
    return(((double) l) == d);
}

/****************************************************************************
 ** HasFloatPart(s) Will return 1 if the string s  represents a double that**
 ** cannot be represented as a fixnum. This is determinted by looking for  **
 ** any of the sci notation stuff e or E or the radix point '.'.           **
 ****************************************************************************/
int HasFloatPart(s)
char *s;
{   while(*s)
    {   if ((*s == '.')||(*s == 'e')||(*s == 'E')) return(1);
        s++;
    };
    return(0);
}

/****************************************************************************
 ** ConvertToBest(s,&flonum,&fixnum) : Convert the ascii string in s to the**
 ** best possible representation either a double, or a long int. We store   **
 ** the converted number in the location pointed to by the second or third **
 ** parameter as appropriate and return a value of REALATOM  or FIXATOM to **
 ** indicate which type was chosen.                                        **
 ****************************************************************************/
int   ConvertToBest(s,flonum,fixnum)
char  *s;double *flonum; long int *fixnum;
{     char junk;
      if (sscanf(s,"%lf%c",flonum,&junk)==1) {      /* extract a double number */
          if (HasFloatPart(s))                      /* if it has . e or E then*/
             return(REALATOM);                      /* user wants it to be real*/
          if (CanStoreInLong(*flonum)) {            /* can it be a long w/o loss */
              *fixnum = (long) *flonum;             /* of precision? If so do it */
              return(FIXATOM);                      /* and return fixatom else */
          }                                         /* precision was lost so */
          return(REALATOM);                         /* make it a float. */
      }
      serror(NULL, "invalid fixnum/flonum format", s, -1);
}

/****************************************************************************
 ** function takeatom(); This will just take the currently scanned atom and**
 ** place it into a cell. If the atom is an ALPHA atom we look it up in the**
 ** list of atoms. If it is there we return its address. If not we insert  **
 ** it and return the address of the new atom. If the atom is a REAL atom  **
 ** this means that it may be either a real or an integer type. The scanner**
 ** does not make any distinction because a very very large integer will   **
 ** be represented as a double. We use the routine ConvertToBest to decide  **
 ** which representation to use. It returns the LISP celltype that it has  **
 ** chosen and stores the representation in the flonum or fixnum location  **
 ** depending on which representation it has chosen. If the type string we **
 ** call newstring to allocate a cell and heap space for the string. All   **
 ** strings are considered unique objects unlike alpha atoms.              **
 ****************************************************************************/
struct conscell *takeatom()
{      if ISREAL()
       {  struct conscell *work; double flonum; long int fixnum;
          push(work);
          if (ConvertToBest(buff,&flonum,&fixnum) == REALATOM)
          {   work = LIST(new(REALATOM));
              REAL(work)->atom = flonum;
          }
          else
          {   work = LIST(new(FIXATOM));
              FIX(work)->atom = fixnum;
          };
          fret(LIST(work),1);
       };
       if ISALPHA()
          return(LIST(CreateInternedAtom(buff)));
       if ISSTRING()
          return(LIST(insertstring(buff)));
       return(NULL);
}

/*****************************************************************************
 ** function TakeSexpression(port) Will read the next S expression from the **
 ** port 'port'. And return it: The BNF for an S-expression is as follows:  **
 **                                                                         **
 **      <S-expression> ::= <atom> | '(' <list>                             **
 **              <list> ::= ')'                                             **
 **                       | <S-expression>+  ( '.' <S-expression> ) ')'     **
 **                                                                         **
 ** The function TakeSexpression will take the S-expression by looking at   **
 ** token (which must have been preprimed) and performing the first line in **
 ** the above BNF. If it finds a '(' it calls TakeList(port) to get the     **
 ** more complex list body and terminating ')'. When taking an S-expression **
 ** if the input is a read macro or spliced read macro we allow it to run   **
 ** and then return the result. If we get an error, we dump the top expr    **
 ** ession on the mark stack, this is usually from TakeList, the value 'r'. **
 *****************************************************************************/
struct conscell *TakeSexpression(port)
struct filecell *port;
{      extern struct conscell *TakeList();
       if ISATOM() return(takeatom());
       if ISOPEN() return(TakeList(port));
       if (ISRMACRO()||ISSRMACRO())
          return(RunReadMacro(port,buff));
       serror(NULL, "this token does not begin a valid S-expression", buff, -1);
}

/****************************************************************************
 ** fuction TakeList(port) Handles the second line of the BNF listed above.**
 ** As we are taking the <S-expression>+ sequence we built a list of cons  **
 ** cells. If we encounter a read macro we run it and add it to the list,  **
 ** if we encounter a splice read macro we run it and splice it into the   **
 ** list so far.                                                           **
 ****************************************************************************/
struct conscell *TakeList(port)
struct filecell *port;
{      struct conscell *l,*r,*n; int lineNum = liScanLineNum;
       push(l); push(n); push(r);               /* r ON TOP OF STACK */
       curtok = scan((zapee = port->atom),buff);/* log for zapline */
       do {   if ISCLOSE() fret(r,3);
              if ISRMACRO() {
                 n = new(CONSCELL);
                 n->linenum = liScanLineNum;
                 n->carp = RunReadMacro(port,buff);
              } else {
                 if ISSRMACRO()
                    n = RunReadMacro(port,buff);
                 else {
                    if ISEOTOK() serror(NULL, "expecting ')' found EOF", NULL, lineNum);
                    n = new(CONSCELL);
                    n->linenum = liScanLineNum;
                    n->carp = TakeSexpression(port);
                 }
              }
              if (r == NULL) r = n;
              if (l == NULL) l = n; else l->cdrp = n;
              while(l->cdrp != NULL) l = l->cdrp;
              curtok = scan(port->atom,buff);
          }
       while(!ISDOT());                         /* take '.' <S-exp> ')' */
       curtok = scan(port->atom,buff);
       l->cdrp = (ISRMACRO()||ISSRMACRO()) ?
                   RunReadMacro(port,buff) : TakeSexpression(port);
       curtok = scan(port->atom,buff);
       if ISCLOSE() fret(r,3);
       serror(NULL, "expecting ')' after dotted pair, found", buff, -1);
}

/************************************************************************
 ** Outside world's access to Read an Expression. Return eofval if the **
 ** end of file is detected. Read is from filecell 'port'. We prime the**
 ** curtok by scanning from the file then calling TakeSexpression. The **
 ** lnflag if TRUE causes ReadExpression to restore the line number in **
 ** the scanner. This is necessary when reading from a file which is   **
 ** currently being read. If lnflag is false the line numbers are not  **
 ** saved/restored, this is the case when readmacros are being run.    **
 ************************************************************************/
struct conscell *ReadExpression(port,eofval,lnflag)
struct filecell *port;
struct conscell *eofval;
int    lnflag;
{      int oldNum;
       if (port->atom == NULL) ioerror(NULL);          /* NULL if port closed */
       if (lnflag)
           oldNum = ScanSetLineNum(port->linenum);     /* remember current scan line num and set this ports one */
       curtok = scan((zapee = port->atom),buff);       /* log it for (zapline) */
       if (ferror(port->atom)) {
           if (port->issocket) return(eofval);         /* return EOF on I/O error on socket */
           if (lnflag)
               port->linenum = ScanSetLineNum(oldNum); /* restore old scan line number and remember this ports current one */
           ioerror(port->atom);                        /* if read error occurs */
       }
       if (NOTEOTOK())
           eofval = TakeSexpression(port);
       if (lnflag)
           port->linenum = ScanSetLineNum(oldNum);     /* restore old scan line number and remember this ports current one */
       return(eofval);
}

/************************************************************************
 ** printhunk(p,l,how,counter). Print the hunk 'l' on stream 'p' pass  **
 ** delim on/off down to the printlist used to print every element of  **
 ** the hunk. If counter is set we do not print the { }'s but just add **
 ** two to the counter. See printatom/printlist please. Note that we   **
 ** must be careful not to print the last base cdr pointer cell if the **
 ** hunk is not a power of two in size, that is why 'j' counts elements**
 ** so that I can stop printing the last element if necessary.  Note if**
 ** we are computing the counter length of the hunk we exit if this    **
 ** count ever goes positive.                                          **
 ************************************************************************/
printhunk(p,h,how,counter)
FILE *p;
struct hunkcell *h;
int  how; int *counter;
{    int basesize,i,j;
     struct conscell *c;
     if (counter != NULL) (*counter)++; else fprintf(p,"{");
     basesize = CEILING_DIV_2(h->size);
     for(i=0,j=1;i<basesize;i++,j++)
     {   if ((counter != NULL)&&(*counter >= 0)) return;
         c = ReadHunkBase(h,i);
         if (i>0)
         {  if (counter != NULL) (*counter)++; else fprintf(p," ");
         };
         printlist(p,c->carp,how,NULL,counter);
         j ++;
         if (j <= h->size)
         {  if (counter != NULL) (*counter)++; else fprintf(p," ");
            printlist(p,c->cdrp,how,NULL,counter);
         };
     };
     if (counter != NULL) (*counter)++; else fprintf(p,"}");
}

/************************************************************************
 ** printatom(l) : This will print the atom pointed to by 'l'. If the  **
 ** celltype is an ALPHA atom we print a string , if it is an open file**
 ** pointer we will print out it's value in hex. 'how' will be set to  **
 ** be either DELIM_ON or DELIM_OFF depending on who calls us. If the  **
 ** atom is alpha and it is a scannable token (isalphatoken) we will   **
 ** print it as is, or if the DELIM_OFF flag is set we will print it   **
 ** as is anyway. If DELIM_ON and not alphatoken we print with | |'s   **
 ** If counter is a pointer to an integer (non null) then we do not    **
 ** actually print anything, we just increment the counter by the numb-**
 ** of characters that would have been output. If the counter ever goes**
 ** positive we stop because it has gone past limit set by user call.  **
 ** Note that tbuff must be 2*256 to allow for doubling due to \ insert**
 ** ion by the ExpandEscapes function.                                 **
 ************************************************************************/
printatom(p,l,how,counter)
FILE *p;
struct alphacell *l;
int  how; int *counter;
{    char tbuf[MAXATOMSIZE * 2];
     if ((counter != NULL)&&(*counter >= 0))  return;
     if (l != NULL)
     {   switch(l->celltype)
         {  case ALPHAATOM :
                 if ((how == DELIM_OFF)||(isalphatoken(l->atom)))
                 {  if (counter == NULL)
                        fprintf(p,"%s",l->atom);
                    else
                        *counter += strlen(l->atom);
                 }
                 else
                 {  ExpandEscapesInto(tbuf,l->atom);
                    if (counter == NULL)
                        fprintf(p,"|%s|",tbuf);
                    else
                        *counter += (strlen(tbuf) + 2);
                 };
                 break;
            case FILECELL :
                 if (PORT(l)->fname != NULL)
                 {   int num = (PORT(l)->atom == NULL) ? -1 : fileno(PORT(l)->atom);
                     if (counter == NULL)
                         fprintf(p,"%%%s@%02d%%",PORT(l)->fname->atom,num);
                     else
                         *counter += (strlen(PORT(l)->fname->atom) + 5);
                 };
                 break;
            case CLISPCELL:
                 if (counter == NULL)
			 fprintf(p,"%%L(%lx),C(%lx)%%",(long)CLISP(l)->literal,(long)CLISP(l)->code);
                 else
                     *counter += 20;
                 break;
            case REALATOM :
                 {   double v = REAL(l)->atom;
                     if (CanStoreInLong(v))           /* cleaner output */
                     {   sprintf(tbuf,"%.1lf",v);
                         if (counter == NULL)
                             fprintf(p,"%s",tbuf);
                         else
                             *counter += strlen(tbuf);
                     }
                     else
                     {   sprintf(tbuf,"%lg",v);
                         if (counter == NULL)
                             fprintf(p,"%s",tbuf);
                         else
                             *counter += strlen(tbuf);
                     };
                 };
                 break;
            case FIXATOM :
                 {   long int v = FIX(l)->atom;
                     sprintf(tbuf,"%ld",v);
                     if (counter == NULL)
                         fprintf(p,"%s",tbuf);
                     else
                         *counter += strlen(tbuf);
                 };
                 break;
            case FIXFIXATOM :
                 sprintf(tbuf,"%%1(%ld)2(%ld)%%",FIXFIX(l)->atom1, FIXFIX(l)->atom2);
                 if (counter == NULL)
                     fprintf(p,"%s",tbuf);
                 else
                     *counter += strlen(tbuf);
                 break;
            case STRINGATOM :
                 if (how == DELIM_OFF)
                 {  if (counter == NULL)
                        fprintf(p,"%s",STRING(l)->atom);
                    else
                        *counter += strlen(STRING(l)->atom);
                 }
                 else
                 {  ExpandEscapesInto(tbuf,STRING(l)->atom);
                    if (counter == NULL)
                        fprintf(p,"\"%s\"",tbuf);
                    else
                        *counter += (strlen(tbuf) + 2);
                 };
                 break;
            case HUNKATOM:
                 printhunk(p,l,how,counter);
                 break;
            case ARRAYATOM:
                 l = ALPHA(ARRAY(l)->info->carp);    /* size is info->carp */
                 if (counter == NULL)
                 {   fprintf(p,"array[");
                     printatom(p,l,how,counter);
                     fprintf(p,"]");
                 }
                 else
                 {   printatom(p,l,how,counter);
                     *counter += 7;                  /* length of "array[]" */
                 };
                 break;
            default :
                 fatalerror("PrintAtom");
                 exit(0);
         };
     };
}


/*****************************************************************************
 ** procedure printlist(): This procedure will recursively print a list. We **
 ** print a list by printing all of its elements surrounded by ( ). If an   **
 ** element in the list is a list we call ourselves recursively. If the elem**
 ** ent is the second half of a dotted pair we print the atom preceeded by  **
 ** a dot and return because we are finished the list. 'how' will indicate  **
 ** if individual atoms are to be surrounded by delimiters if not scannable.**
 ** Squash is a list which we want to print as <**> for stack dumping etc.  **
 ** Compress (quote XXX) into 'XXX only if DELIM_ON, else raw print list.   **
 ** If counter is a non null pointer to an integer, we will increment the   **
 ** count once for every character rather than print each character. This   **
 ** is used by the functions (flatc) and (flatsize). Note that if the count **
 ** ever goes positive we stop because the callers limit has been reached.  **
 *****************************************************************************/
printlist(p,l,how,squash,counter)
FILE  *p;
struct conscell *l,*squash;
int    how; int *counter;
{      int n = 0; struct alphacell *at;
       TEST_BREAK();
       if ((counter != NULL)&&(*counter >= 0))  return;
       if ((squash != NULL) && (l == squash))             /* squshed print? */
       {   fprintf(p," <**>");
           return;
       };
       if (l==NULL)
       {   if (counter == NULL)
               fprintf(p,"nil");                    /* handle null list */
           else
               *counter += 3;
           return;
       };
       if (l->celltype != CONSCELL)
       {   printatom(p,(struct alphacell *) l,how,counter);
           return;
       };
       if (l->carp != NULL)                               /* handle (quote X)*/
       {   at = (struct alphacell *)l->carp;
           if (at->celltype == ALPHAATOM)
           {   if (strcmp(at->atom,"quote") == 0)
               {   if ((l->cdrp != NULL)&&(l->cdrp->celltype == CONSCELL))
                   {   if (counter == NULL)
                           fprintf(p,"'");
                       else
                           *counter += 1;
                       printlist(p,l->cdrp->carp,how,squash,counter);
                       return;
                   };
               };
           };
       };
       if (counter == NULL) fprintf(p,"("); else *counter += 1;
       while( l != NULL )
       {      TEST_BREAK();                                      /* user hit break key ? */
              if ((counter != NULL)&&(*counter >= 0)) return;    /* quit? */
              if (l->carp != NULL)
              {   if (l->carp->celltype == CONSCELL)
                  {   if (n>0)
                      {  if (counter == NULL)
                            fprintf(p," ");
                         else
                            *counter += 1;
                      };
                      printlist(p,l->carp,how,squash,counter);
                  }
                  else
                  {   if (n>0)
                      {  if (counter == NULL)
                             fprintf(p," ");
                         else
                             *counter += 1;
                      };
                      printatom(p,(struct alphacell *) l->carp,how,counter);
                  };
              } else
                  {   if (n>0)
                      {   if (counter == NULL)
                              fprintf(p," ");
                          else
                              *counter += 1;
                      };
                      if (counter == NULL) fprintf(p,"nil"); else *counter +=1;
                  };
              if (l->cdrp != NULL)
              {   if (l->cdrp->celltype != CONSCELL)
                  {   if (counter == NULL) fprintf(p," . "); else *counter +=3;
                      printatom(p,(struct alphacell *) l->cdrp,how,counter);
                      if (counter == NULL) fprintf(p,")"); else *counter +=1;
                      return;
                  };
              };
              l = l->cdrp;
              n++;
       };
       if (counter == NULL) fprintf(p,")"); else *counter += 1;
}

/***************************************************************************
 ** flatsize(l): Returns the number of bytes the list 'l' will occupy if  **
 ** printed using (print l). limit is an integer that is a point beyond   **
 ** which we are no longer interested in the result. Since printlist will **
 ** stop printing when the count goes positive we just start the print    **
 ** counter at -limit and wait for it to go positive or finish.           **
 ***************************************************************************/
int flatsize(l,limit)
struct conscell *l;
int limit;
{      int i;
       limit = i = -limit;
       printlist(NULL,l,DELIM_ON,NULL,&i);
       return(i - limit);
}

/***************************************************************************
 ** prettyprint: Print in a nice way the expression 'expr' to file 'fp'   **
 ** indent level and indent so far control position on output page. If we **
 ** are pp'ing a prog() we do not indent the atoms which are assumed to be**
 ** labels. This shows labels in a 'nice' way. I am sorry that this is    **
 ** so cryptic but it was translated to C from LISP in a direct manner and**
 ** this shows in the C style. Think in LISP and you should get the idea. **
 ** This pretty printer algorithm was taken of USENET  a while ago. It is **
 ** not perfect but it gets the job done.                                 **
 ***************************************************************************/
prettyprint(expression,indent_level,indent_so_far,sink)
struct conscell *expression;
int    indent_level;
int    indent_so_far;
FILE * sink;
{      struct conscell *current_expression;
       struct conscell *CARexpr,*CDRexpr,*CADRexpr,*CDDRexpr;
       int new_level,is_prog,at_size,i,sizeCARexpr;
       for(i= (indent_level - indent_so_far);i>0;i--) fprintf(sink," ");
       if ( ((expression == NULL) || (expression->celltype != CONSCELL)) ||
            ((PrettyWidth - indent_level) > flatsize(expression,PrettyWidth))) {
              printlist(sink,expression,DELIM_ON,NULL,NULL);
              return;
       }
       fprintf(sink,"(");
       CARexpr = (expression != NULL) ? expression->carp : NULL;
       CDRexpr = (expression != NULL) ? expression->cdrp: NULL;
       CADRexpr= ((CDRexpr   != NULL)&&(CDRexpr->celltype == CONSCELL)) ? CDRexpr->carp: NULL;
       CDDRexpr= ((CDRexpr   != NULL)&&(CDRexpr->celltype == CONSCELL)) ? CDRexpr->cdrp: NULL;
       prettyprint(CARexpr,indent_level+1,indent_level+1,sink);
       if (CDRexpr == NULL) goto END;
       if (CDRexpr->celltype != CONSCELL) {
           fprintf(sink," . ");
           prettyprint(CDRexpr,indent_level+4,indent_level+4,sink);
           goto END;
       }
       sizeCARexpr = flatsize(CARexpr, MAXATOMSIZE);            /* atom limit */
       if ((CARexpr!=NULL)&&(CARexpr->celltype == CONSCELL)) {
           fprintf(sink,"\n");
           prettyprint(CADRexpr,indent_level+1,0,sink);
       } else
           if ((sizeCARexpr/3) > PrettyWidth) {
               fprintf(sink,"\n");
               prettyprint(CADRexpr,indent_level+1,0,sink);
           } else
               prettyprint(CADRexpr,2 + indent_level + sizeCARexpr,
                                    1 + indent_level +  sizeCARexpr,sink);
       current_expression = CDDRexpr;
       if ((CARexpr==NULL)||(CARexpr->celltype!=CONSCELL)) {
           at_size = sizeCARexpr;
           new_level = 2 + indent_level; /*  + at_size; */
           is_prog = (lookupatom("prog",INTERNED) == ALPHA(CARexpr));
       } else {
           new_level = 1 + indent_level;
           is_prog = 0;
       }
       while(current_expression != NULL) {
           if (current_expression->celltype != CONSCELL) {
               fprintf(sink," . ");
               prettyprint(current_expression,i,0,sink);
               goto END;
           }
           fprintf(sink,"\n");
           if (is_prog && ((current_expression->carp == NULL) ||
              (current_expression->carp->celltype != CONSCELL)))
               i = new_level - at_size;
           else
               i = new_level;
           prettyprint(current_expression->carp,i,0,sink);
           current_expression = current_expression->cdrp;
       }
  END: fprintf(sink,")");
       return;
}

/***************************************************************************
 ** CopyNextPath(to,&from) : Where to is a work buffer, and from is a ptr **
 ** to a ptr to a PATH buffer. The PATH buffer is a set of colon separated**
 ** MS-DOS or UNIX paths. ie "\junk1\junk2:\junk3\junk4:\junk5:" We copy  **
 ** the first path from *from to to and stop when we hit the ':'. We then **
 ** adjust **from to point to the next path in the buffer and return a 1  **
 ** meaning that we successfully extracted a path. If we did not manage to**
 ** extract a path we return 0. When coping we skip blanks. Also if there **
 ** is a DIRSEPCHAR on the end of the extracted path, we strip it off.    **
 ** This is because the loadfile function will put in its own DIRSEPSTRING**
 ***************************************************************************/
int CopyNextPath(to,from)
char *to,**from;
{    register int n = 0;
     while((**from != '\0')&&(**from != ':'))    /* copy up to \0 or : */
     {  if (**from == ' ')                       /* blank ? so just    */
           (*from)++;                            /* skip & continue... */
        else                                     /* non blank so copy  */
        {  *to++ = *(*from)++;                   /* char & update *from*/
           n++;                                  /* inc copy counter.  */
        };
     };
     if (n > 0)                                  /* copied something?  */
     {   *to-- = '\0';                           /* yes so delimite it */
         if (**from == ':')                      /* stopped by a ':' ? */
            (*from)++;                           /* prepare next call. */
         if (*to == DIRSEPCHAR)                  /* strip of a sep ch  */
             *to = '\0';                         /* if there is one.   */
         return(n);                              /* say we got a 'path'*/
     };
     return(0);                                  /* say got no 'path'  */
}

/***************************************************************************
 ** loadfile(fname) : will try to load the file fname.l. It will assume   **
 ** that the file is a (dot L) or lisp file and will not append the .l    **
 ** first. If this file does not exist it will append a .l and try again  **
 ** if this does not work it will try the same thing from the directory   **
 ** given in the environment variable LIBENVVAR. Eventually when it finds **
 ** the file it will start to read and evaluate lists in the file. Then   **
 ** it returns 1 for success or 0 for failure. Note that the LIBENVVAR    **
 ** may specify a set of ':' separated directories to be searched. The    **
 ** function CopyNextPath(to,&from) will copy a path from from to to and  **
 ** set from to point to the end of the copied path. CopyNextPath returns **
 ** 1 when it extraces a path and 0 when none is found.                   **
 ***************************************************************************/
int  loadfile(fname)
char *fname;
{     FILE *fp, *fopen();
      char work[512], *path;
      struct conscell *input;
      struct filecell *port;
      int c, oldNum;
      if (*fname == '\0') return(0);
      strcpy(work,fname);
      if ((fp = fopen(work,"r")) != NULL) goto hit;
      strcpy(work+strlen(work),".l");
      if ((fp = fopen(work,"r")) != NULL) goto hit;
      if ((path = getenv(LIBENVVAR)) == NULL) {              /* file not found and no search path provided */
          if ((path = getenv("PCLISP_DIR")) == NULL) return(0); /* default to looking in pclispdir since */
          sprintf(work, "%s/pclispdir/%s", path, fname);   /* this is where most of the pure LISP files are */
          if ((fp = fopen(work,"r")) != NULL) goto hit;
          strcat(work,".l");
          if ((fp = fopen(work,"r")) != NULL) goto hit;
      }                                                      /* the pclisp script i.e. when #!/.../lisp.out is used */
      while(CopyNextPath(work,&path)) {                      /* there is a search path so iterate through it */
          strcat(work,DIRSEPSTRING);
          strcat(work,fname);
          if ((fp = fopen(work,"r")) != NULL) goto hit;
          strcat(work,".l");
          if ((fp = fopen(work,"r")) != NULL) goto hit;
      }
      errno = 0;                                             /* don't want apply to catch it */
      return(0);

     /*
      | We have a file opened 'fp' so start reading and evaluating the
      | S expressions from the file. Note that it is important that the
      | local 'input' be pushed LAST on the mark stack because the error
      | handling uses it as a 'close' S-expression when informing the user
      | of the syntax error. Note that we call buresetlog with the file
      | pointer and 1 after opening it so that (resetio) can close it if
      | anything goes wrong and we call buresetlog with fp and 0 after we
      | have closed the file so that (resetio) will no longer close this
      | file if called.
      */
hit:  buresetlog(fp, 2);
      push(port); push(input);

     /*
      | Create a port object from this fp and start reading & evaluating the
      | file
      */
      port = PORT(MakePort(fp,CreateInternedAtom(work)));

     /*
      | If first char in a file is '#' then ignore the entire line, this
      | allows LISP files to begin with '#!pclisp" and be run directly.
      */
      oldNum = 1;
      c = getc(fp);
      if (c == '#') {
          while( (c != EOF) && (c != '\n') ) c = getc(fp);
          oldNum = 2;
      } else
          if (c != EOF)
              ungetc(c, fp);
          else
              goto xit;

     /*
      | Reset scanned line number to 1(or2) and hang on to the old value.
      */
      oldNum = ScanSetLineNum(oldNum);
      port->linenum = oldNum;

     /*
      | For each expression in the file check to see if it is a binary expression or
      | ASCII by peeking ahead at the char, if binary b-read it and eval otherwise if
      | textual (read) it and eval it.
      */
      while((c = getc(fp)) != EOF) {
           ungetc(c, fp);
           errno = 0;                                      /* clear for new errors */
           if (c >= 128) {
               extern struct conscell *libread();
               input = libread(port, 0L);
               if (!input || (input->celltype != CONSCELL) || !input->carp) break;
               input = input->carp;
           } else {
               curtok = scan((zapee = port->atom), buff);  /* log for (zapline) */
               if (ISEOTOK()) break;                       /* if got end of file token we're done */
               if (ISUNKNOWN()) continue;                  /* got >= 128 char probably a CLISP */
               input = TakeSexpression(port);              /* otherwise (read) expression */
           }
           eval(input);
      }

 xit: buresetlog(fp, 0);                                   /* close the file */
      ScanSetLineNum(oldNum);                              /* restore scanner line number */
      fclose(fp);                                          /* ditto */
      port->atom = NULL;
      if (TestForNonNil("$ldprint",1))                     /* 1-> default is nil */
          printf("--- [%s] loaded ---\n",work);            /* normal exit procedure */
      xpop(2);                                             /* .... */
      return(1);
}

/***************************************************************************
 ** processfile(fp,etc..)   This procedure will take all of the lists in  **
 ** a file. When the end of file token is scanned it returnes. Processing **
 ** consists of reading and evaluating. If the ECHO flag is set the input **
 ** is comming from the standard input. In this case we echo prompts etc. **
 ** Otherwise no echo is ever performed. After every eval() the marktop   **
 ** should be 0 again. If it is not one of the built in functions has made**
 ** a mistake doing an xpush-xret pair. This is a common error that I make**
 ** when working on a complex built in function. When this error is hit I **
 ** reset the top of the mark stack to 0. And print an error. If we are   **
 ** reading from stdin we do not want to have eval take a break error exit**
 ** if the break was entered before execution began. So we reset the break**
 ** if we read the list from the standard input.                          **
 ***************************************************************************/
processfile(port,echoflag,prompt)
struct filecell *port;
int  echoflag;
char *prompt;
{   struct conscell *l; int oldNum;
    if (echoflag == ECHO) printf("%s",prompt);
    oldNum = ScanSetLineNum(port->linenum);          /* save old linenum and set current line number */
    curtok = scan((zapee = port->atom),buff);        /* log for (zapline) */
    errno = 0;                                       /* clear for new error */
    while NOTEOTOK() {
          inlist = TakeSexpression(port);
          if (port->atom == stdin) {
              BREAK_RESET();
#             if !defined(APOLLO)
                 printf("..."); fflush(stdout);      /* before eval and then erases it after */
                 printf("\b\b\b   \b\b\b");          /* before printing result of eval. */
#             endif
          }
          inlist = eval(inlist);
          if (echoflag == ECHO) {
              printlist(stdout, inlist, DELIM_ON, NULL, NULL);
              printf("\n");
          }
          if (echoflag == ECHO) printf("%s",prompt);
          curtok = scan(port->atom,buff);
    }
#   if NEEDNLAFTERBREAKEXIT                          /* UNIX does not need a NL   */
       if (echoflag == ECHO) printf("\n");           /* for EOF to work but MSDOS */
#   endif                                            /* does. Get bk>--> otherwise*/
    port->linenum = ScanSetLineNum(oldNum);          /* restore previous line number and remeber current port line num */
}

/***************************************************************************
 ** takelispexit() : Exit the lisp interpreter. We must reset any machine **
 ** states that are liable to have been altered. Note that for MSDOS we   **
 ** call the video reset function in bufunc.c to set the sane video modes.**
 ***************************************************************************/
takelispexit()
{
#      if GRAPHICSAVAILABLE
          resetvideo();
#      endif
       deiniterrors();                        /* restore exception handlers */
       exit(0);
}

/***************************************************************************
 ** liargs(argc, argv) is used to remember the argument list so that it   **
 ** can be processed from LISP using the (command-line-args) primitive.   **
 ***************************************************************************/
int liargc = 0; char **liargv = NULL;

int liargs(argc, argv)
    int argc; char **argv;
{
    liargc = argc;
    liargv = argv;
}

/***************************************************************************
 ** mainline code: first we print the header message. Next we look at the **
 ** environment variables and extract any memory configuration info. These**
 ** will most likely be the percentage of alpha and percentage of heap to **
 ** use. We first call InitMarkStack to allocate the MSSIZE array for gc  **
 ** marking. Next call initmem with these env settings or defaults if     **
 ** they are not present. Next we init all the atoms, and all built in    **
 ** functions by calling initeval(). Finally we process each file on the  **
 ** command line one by one, call loadfile for each. Finally when all the **
 ** files are processed we turn to stdin and process it. We also set up   **
 ** the signal traps for stack overflow and user break conditions. Also   **
 ** we set up the longjmp target as the top of the command line processing**
 ** loop. An error return here will cause us to process standard input    **
 ** with a break error prompt "er>" and with all local variables bound as **
 ** they were at the time of the break. When the user exits the break loop**
 ** we unbind all the locals and resume the normal standard input loop.   **
 ** Note we process a few (sstatus) options in the main loop. The ignore  **
 ** EOF option causes the EOF at the top level to be ignored. This is done**
 ** by looping until the status of this becomes not ignoreeof when the    **
 ** takelispexit() function is called (which never returns). The option   **
 ** automatic-reset causes the break level to be ignored, we just skip the**
 ** invokation of the break level processor.                              **
 ***************************************************************************/
#if !defined(MACRO)
    main(argc,argv)
    int argc; char *argv[];
    {
	printf("%s%s%s", "PC-LISP V", VERSION, " Copyright (C) Peter J.Ashwood-Smith, 1989-2015\n");
        zapee = stdin;
        liargs(argc, argv);
        InitMarkStack();
        initmem();
        initatoms();                        /* zero out atom hash table  */
        InstallSpecialAtoms();              /* put 't', 'lambda' etc. in oblist */
        InstallBuiltInFunctions();          /* putd 'eval','apply' */
        if (!setjmp(env)) {                 /* <-- IERROR TARGET */
           initerrors();                    /* init stack & break handlers */
           loadfile(AUTOSTART);             /* load auto start up file */
           if (argc > 1)
           {   while(--argc > 0)            /* load each file on command line */
               {   argv += 1;
                   if (!loadfile(*argv))
                        printf("--- can't find %s ---\n",*argv);
                   else
                        ScanReset();
               }
           }
        }                                                   /* BREAK HANDLER LOOP */
        else                                                /* error enter break */
        {   ScanReset();                                    /* reset [ ] handling */
            if (!GetOption(AUTORESET))                      /* enter bk level if  */
               processfile(piporthold,ECHO,"er>");          /* option & do til EOF*/
            unwindscope();                                  /* unbind all locals */
        }
        for(;;)                                             /* back to top level  */
        {   clearerr(stdin);                                /* two EOFs in a row  */
            ScanReset();                                    /* reset [ ] handling */
            processfile(piporthold,ECHO,"-->");             /* back to stdin      */
            if (!GetOption(IGNOREEOF))                      /* EOF, if !ignore    */
                takelispexit();                             /* the exit else loop */
        }
    }
#else

   /*
    | Look up the atom 'func' which is expected to be the name of a function that the user
    | is trying to lidomac(a). We look it up first, if not found we try to autoload it and
    | then Try to auto load a function. This is identical to the 'autoloadatom' funciton
    | in lieval.c except that it returns TRUE or FALSE for the lidomac and lidomaca
    | functions.
    */
    static struct alphacell *lookupmacro(func)
    char *func;
    {   extern struct alphacell *autoloadhold; char *str;
        extern struct conscell *getprop();
        struct conscell *s; struct alphacell *at;
        at = lookupatom(func, INTERNED);                     /* try to find this atom 'func' */
        if (at == NULL) return(NULL);                        /* if not found nothing say so */
        if (at->fntype != FN_NONE) return(at);               /* if is function return atom now */
        s = getprop(at, autoloadhold);                       /* if not a function check autoload property */
        if ((s == NULL)||(!GetString(s,&str))) return(NULL); /* if no autoload string property it is not function */
        if (!setjmp(env)) {                                  /* <-- IERROR TARGET */
            initerrors();                                    /* install error handlers */
            loadfile(str);                                   /* has string autload property, try load the file */
        } else {
            printf("file %s could not be read to resolve autoload of atom %s\n", str, at->atom);
            at = NULL;
            ScanReset();                                     /* reset [ ] handling */
            unwindscope();
        }
        deiniterrors();
        if (at && at->fntype != FN_NONE)                     /* and try again to see if it is a function */
            return(at);                                      /* yes autoload bound it to a function... */
        return(NULL);                                        /* no, return NULL */
    }

   /*
    | Trap functions that will be called if set before and after an interactive
    | session with the user. Used on some systems to create new windows for the
    | session and to close it afterwards.
    */
    static int (*beforeInteraction)() = NULL;                /* called before user interaction. */
    static int (*afterInteraction)()  = NULL;                /* caleed after user interaction.  */

   /*
    | Set to true when system is initialized and runnable.
    */
    static int liprimed = 0;

   /*
    | This function if non NULL will be called to request permission to enter the debugger. If
    | 0 is returned the debugger is entered.
    */
    static int (*debugIntercept)() = NULL;

   /*
    | This function is called to prime the LISP data structures prior to running the
    | interpreter.
    */
    static void liprime()
    {
        InitMarkStack();                   /* allocate and zero out the mark stack */
        initmem();                         /* set up memory manager */
        initatoms();                       /* zero out atom hash table  */
        InstallSpecialAtoms();             /* put 't', 'lambda' etc. in oblist */
        InstallBuiltInFunctions();         /* putd 'eval','apply' */
        SsInstall();                       /* install all the user functions */
    }

   /*
    | Initialize the interpreter for use as a macro language of some other
    | package. Fn is the name of the file that is to be loaded for this
    | application.
    */
    int liinit(fn1,fn2)
    char *fn1,*fn2;
    {
        if (!liprimed) { liprime(); liprimed = 1; }
        if (!setjmp(env)) {                                 /* <-- IERROR TARGET */
            initerrors();                                   /* init stack & break handlers */
            loadfile(fn1);                                  /* load auto start up file */
            loadfile(fn2);                                  /* load application start up file */
            deiniterrors();                                 /* restore exception handlers */
            return(0);                                      /* all done so return to caller */
        }                                                   /* BREAK HANDLER LOOP */
        else                                                /* error enter break */
        {
           if (!debugIntercept || (*debugIntercept)()) {
               if (beforeInteraction) (*beforeInteraction)();
               ScanReset();                                 /* reset [ ] handling */
               if (!GetOption(AUTORESET))                   /* enter bk level if  */
                   processfile(piporthold,ECHO,"er>");      /* option & do til EOF*/
               unwindscope();                               /* unbind all locals */
               for(;;) {                                    /* back to top level  */
                   clearerr(stdin);                         /* two EOFs in a row  */
                   ScanReset();                             /* reset [ ] handling */
                   processfile(piporthold,ECHO,"-->");      /* back to stdin      */
                   if (!GetOption(IGNOREEOF)) break;        /* EOF, if !ignore    */
               }
               if (afterInteraction) (*afterInteraction)();
           } else {
               ScanReset();
               unwindscope();
               clearerr(stdin);
           }
        }
        deiniterrors();                                  /* restore exception handlers */
        return(0);
    }

   /*
    | Make LISP interpreter safe. This means that SIGINTS are handled by whoever
    | called us but we do not respond to them.
    */
    void lisafe(enable)
    int enable;
    {   disableerrors(0, !enable);
    }

   /*
    | This function will perform one of two actions depending on the value of 'fn.
    | If fn is NULL then it will create an interactive session with the interpreter.
    | If fn is non null then it will completely reboot the interpreter, reload the
    | default start up file and then load 'fn' and finally return to the application.
    | These are used to implement the actions 'gotoLISP' and 'rebootLISP' in FUNDES.
    */
    void limacro(fn1,fn2)
    char *fn1,*fn2;
    {   printf("%s%s%s", "\nPC-LISP V" , VERSION, " Copyright (C) Peter J.Ashwood-Smith, 1989-1992\n");
        printf("Press CONTROL-D to return to PC-LISP application.\n");
        if (fn1 != NULL) {
           if ( reboot_ok() ) {
              InitMarkStack();
              initmem();
              initatoms();                                /* zero out atom hash table  */
              InstallSpecialAtoms();                      /* put 't', 'lambda' etc. in oblist */
              InstallBuiltInFunctions();                  /* putd 'eval','apply' */
              SsInstall();                                /* install all the user functions */
           } else
              printf("CANNOT REBOOT LISP EXCEPT FROM TOP LEVEL, RELOADING INSTEAD!\n");
        }
        if (!setjmp(env)) {                               /* <-- IERROR TARGET */
           initerrors();                                  /* install error handlers */
           if (fn1 != NULL) {
               buresetio(NULL);                           /* close any opened LISP files */
               loadfile(fn1);                             /* load auto start up file */
               loadfile(fn2);                             /* load application start up file */
           }
        } else {
           ScanReset();                                   /* reset [ ] handling */
           if (beforeInteraction) (*beforeInteraction)();
           if (!GetOption(AUTORESET))                     /* enter bk level if  */
               processfile(piporthold,ECHO,"er>");        /* option & do til EOF*/
           unwindscope();                                 /* unbind all locals */
        }
        if (fn1 == NULL) {                                /* if not restart enter interpreter */
           if (beforeInteraction) (*beforeInteraction)();
           for(;;) {                                      /* back to top level  */
               clearerr(stdin);                           /* two EOFs in a row  */
               ScanReset();                               /* reset [ ] handling */
               processfile(piporthold,ECHO,"-->");        /* back to stdin      */
               if (!GetOption(IGNOREEOF)) break;          /* EOF, if !ignore    */
           }
           if (afterInteraction) (*afterInteraction)();
        }
        deiniterrors();                                   /* restore exception handlers */
    }

   /*
    | Validate the spec as "%s "a %d .." that will be passed to lidomac and
    | lidomaca above. Return 0 if it is ok.
    */
    int livalspc(spec)
    char *spec;
    {    while(*spec != '\0') {       /* loop through %s %d .. spec */
           if (*spec == '%') {        /* if it is a type spec */
              spec++;
              switch(*spec++) {       /* decide on type */
                 case 'f' :
                 case 'd' :
                 case 's' :
                 case 'a' :           /* these are all legal */
                 case '*' :
                    break;
                 default:
                    return(-1);       /* error, unknown type ?? */
              }
           } else
              if (*spec++ != ' ')     /* if non blank and not % its an error */
                  return(-1);
         }
         return(0);
    }

   /*
    | livalmac(mac) - will return 0 if and only if the macro 'mac' exists and is
    | of the proper type to be executed, otherwise it returns -1.
    */
    int livalmac(mac)
    char *mac;
    {
        struct alphacell *fname;
        fname = lookupmacro(mac);                        /* find the function's atom may autoload */
        if (fname == NULL) return(-1);                   /* make sure it exists */
        if ((fname->celltype != ALPHAATOM) ||            /* and is defined as a function */
            (fname->fntype == FN_NONE))
           return(-1);
        return(0);
    }


#   include <varargs.h>
   /*
    | lidomac(func, "%s %d %d %a %f...", .....)
    |
    | Will cause the function 'func' to be invoked with the given
    | arguments and argument types. There is a varargs printf style
    | spec which defines the number and kind of args being passed.
    | As in printf, %s is a string, %d is an integer, %f is a float
    | but, unlike printf, %a means an atom. Note that we cannot
    | handle the %* spec because we do not have any way of knowing
    | how many args remain to be processed. This function will return
    | -1 if 'func' was not found etc, 0 if the function returns nil,
    | 1 if it returns true, or the value of the fixnum, if it returns
    | a fixnum.
    */
    int lidomac(va_alist)
    va_dcl
    {   va_list args;
        char *func;                                      /* name of function to be called */
        char *spec;                                      /* spec string "%s ... " */
        double f_parm;                                   /* current float parm */
        long i_parm, n;                                  /* current integer parm */
        char *s_parm, *str;                              /* current string parm */
        struct alphacell *fname;                         /* atom for 'func' to be invoked */
        struct conscell  *form;                          /* argument list for func */
        struct conscell  *tail;                          /* tail of the arg list 'form' */
        struct conscell  *cons;                          /* new cons cell to append to form */
        struct conscell  *arg;                           /* cell holding arg to append */
        struct conscell  *rcnil;

        va_start(args);                                  /* start var args scanning */
        func = va_arg(args,char *);                      /* get first parm the function name */
        spec = va_arg(args,char *);                      /* get second parm the function spec */
        fname = lookupmacro(func);                       /* find the function's atom */
        if (fname == NULL) return(-1);                   /* make sure it exists */
        if ((fname->celltype != ALPHAATOM) ||            /* and is defined as a function */
            (fname->fntype == FN_NONE))
           return(-1);

        push(form); push(arg); xpush(fname);             /* protect from GC */
        while(*spec != '\0') {                           /* loop through %s %d .. spec */
           if (*spec++ == '%') {                         /* if it is a type spec */
              arg = NULL;                                /* extract the arg for type spec */
              switch(*spec++) {                          /* decide on type */
                 case 'f' :
                    f_parm = va_arg(args,double);        /* extract float type */
                    arg = newrealop(f_parm);             /* and build a cell to hold it */
                    break;
                 case 'd' :
                    i_parm = va_arg(args,long);          /* extract integer type */
                    arg = newintop(i_parm);              /* and build a cell to hold it */
                    break;
                 case 's' :
                    s_parm = va_arg(args,char *);        /* extract a string type */
                    arg = LIST(insertstring(s_parm));    /* and build a cell to hold it */
                    break;
                 case 'a' :
                    s_parm = va_arg(args,char *);        /* extract an atom type (string)*/
                    arg = LIST(CreateInternedAtom(s_parm)); /* and intern and get ptr to it */
                    break;
                 default:
                    va_end(args);                        /* error, unknown type ?? */
                    xpop(3);
                    return(-1);
              }
              if (form == NULL) {                        /* if first element make */
                 tail = form = cons = new(CONSCELL);     /* (cons arg nil) */
                 cons->carp = arg;
              } else {
                 cons = new(CONSCELL);                   /* otherwise append to */
                 cons->carp = arg;                       /* tail of the list */
                 tail->cdrp = cons;
                 tail = cons;
              }
           }
        }

        if (!setjmp(env)) {                              /* <-- IERROR TARGET */
           initerrors();                                 /* initialize exception handlers */
           cons = new(CONSCELL);                         /* build (fname args) */
           cons->carp = LIST(fname);                     /* list so that (showstack) */
           cons->cdrp = form;                            /* will register it */
           expush(cons);
           rcnil = apply(fname,form);                    /* run func on the arg list */
           emytop += 1;
           xpop(3);
           deiniterrors();                               /* restore exception handlers */
           if (rcnil == NULL) return(0);                 /* undef => -1 return, nil => 0 return */
           if (GetFix(rcnil, &n)) return(n);             /* fixnum? return it */
           if (GetString(rcnil, &str)) return((int)str); /* string? return it */
           return(1);                                    /* something else, return 1 for true */
        } else {                                         /* error enter break */
           if (!debugIntercept || (*debugIntercept)()) {
               if (beforeInteraction) (*beforeInteraction)();
               ScanReset();                              /* reset [ ] handling */
               if (!GetOption(AUTORESET))                /* enter bk level if  */
                   processfile(piporthold,ECHO,"er>");   /* option & do til EOF*/
               unwindscope();                            /* unbind all locals */
               for(;;) {                                 /* back to top level  */
                   clearerr(stdin);                      /* two EOFs in a row  */
                   ScanReset();                          /* reset [ ] handling */
                   processfile(piporthold,ECHO,"-->");   /* back to stdin      */
                   if (!GetOption(IGNOREEOF)) break;     /* EOF, if !ignore    */
               }
               if (afterInteraction) (*afterInteraction)();
           } else {
               ScanReset();
               unwindscope();
               clearerr(stdin);
           }
        }
        va_end(args);
        deiniterrors();                                  /* restore exception handlers */
        return(0);
    }

   /*
    | lidomaca(func, "%s %d %d %a %f...", argc, argv)
    |
    | Will cause the function 'func' to be invoked with the given
    | arguments and argument types. There is a varargs printf style
    | spec which defines the number and kind of args being passed.
    | As in printf, %s is a string, %d is an integer, %f is a float
    | but, unlike printf, %a means an atom. This routine is similar
    | to lidomac except that it takes an argc argv vector of string
    | args and loops through it and will convert the strings to the
    | proper type before building the form list for the call. Note
    | that the argv vector comes from FLEX hence the non dynamic
    | nature. This function will return -1 if 'func' was not found etc,
    | 0 if the function returns nil, 1 if it returns true, or the value
    | of the fixnum, if it returns a fixnum.
    */
    int lidomaca(func, spec, argc, argv)
    char *func, *spec, argv[][81]; int argc;
    {   double f_parm;                                   /* current float parm */
        long i_parm, n;                                  /* current integer parm */
        char *s_parm, *argvs, *str;                      /* current string parm and arg */
        struct alphacell *fname;                         /* atom for 'func' to be invoked */
        struct conscell  *form;                          /* argument list for func */
        struct conscell  *tail;                          /* tail of the arg list 'form' */
        struct conscell  *cons;                          /* new cons cell to append to form */
        struct conscell  *arg;                           /* cell holding arg to append */
        struct conscell  *rcnil;                         /* return from function */

        int i = 0;
        char dum;                                        /* dummy for sscanf trailing junk detection */

        fname = lookupmacro(func);                       /* find the function's atom autoload if necessary */
        if (fname == NULL) return(-1);                   /* make sure it exists */
        if ((fname->celltype != ALPHAATOM) ||            /* and is defined as a function */
            (fname->fntype == FN_NONE))
           return(-1);

        push(form); push(arg); xpush(fname);             /* protect from GC */
        while(*spec != '\0') {                           /* loop through %s %d .. spec */
           if (*spec++ == '%') {                         /* if it is a type spec */
              argvs = (i >= argc) ? "" : &argv[i][0];    /* extract the arg for type spec */
              arg = NULL;
              switch(*spec++) {                          /* decide on type */
                 case 'f' :
                    if (sscanf(argvs,"%lf%c",&f_parm,&dum)!=1)  /* extract the float parm */
                        goto xer;
                    arg = newrealop(f_parm);             /* and build a cell to hold it */
                    i += 1;
                    break;
                 case 'd' :
                    if (sscanf(argvs,"%ld%c",&i_parm,&dum)!=1)  /* extract the integer parm */
                        goto xer;
                    arg = newintop(i_parm);              /* and build a cell to hold it */
                    i += 1;
                    break;
                 case 's' :
                    s_parm = argvs;                      /* extract a string type */
                    arg = LIST(insertstring(s_parm));    /* and build a cell to hold it */
                    i += 1;
                    break;
                 case 'a' :
                    s_parm = argvs;                         /* extract an atom type (string)*/
                    arg = LIST(CreateInternedAtom(s_parm)); /* and intern and get ptr to it */
                    i += 1;
                    break;
                 case '*' :                              /* %* ==> concat rest of args */
                    {  char work[MAXATOMSIZE];
                       int n = 0;
                       if (i >= argc) break;
                       work[0] = '\0';                   /* loop concatenating the strings */
                       for(;;) {
                            n += strlen(&argv[i][0])+1;  /* making sure they do not overflow */
                            if (n >= MAXATOMSIZE - 1)    /* the work buffer */
                                goto xer;
                            strcat(work,&argv[i][0]);
                            i += 1;                      /* if we are done then get out */
                            if (i >= argc) break;
                            strcat(work," ");            /* else append a blank and keep going */
                       }
                       arg = LIST(insertstring(work));
                    }
                    break;
                 default:
                    goto xer;
              }
              if (form == NULL) {                        /* if first element make */
                 tail = form = cons = new(CONSCELL);     /* (cons arg nil) */
                 cons->carp = arg;
              } else {
                 cons = new(CONSCELL);                   /* otherwise append to */
                 cons->carp = arg;                       /* tail of the list */
                 tail->cdrp = cons;
                 tail = cons;
              }
           }
        }
        if (i != argc) goto xer;                         /* did we extract all the parms passed ? */
        if (!setjmp(env)) {                              /* <-- IERROR TARGET */
           initerrors();                                 /* initialize exception handlers */
           cons = new(CONSCELL);                         /* and build (fname args) */
           cons->carp = LIST(fname);                     /* tail of the list */
           cons->cdrp = form;                            /* for better (showstack) on err */
           expush(cons);
           rcnil = apply(fname,form);                    /* run func on the arg list */
           emytop += 1;
           xpop(3);
           deiniterrors();                               /* restore exception handlers */
           if (rcnil == NULL) return(0);                 /* undef => -1 return, nil => 0 return */
           if (GetFix(rcnil, &n)) return(n);             /* fixnum? return it */
           if (GetString(rcnil, &str)) return((int)str); /* string? return it */
           return(1);                                    /* something else, return 1 for true */
        } else {                                         /* error enter break */
           if (!debugIntercept || (*debugIntercept)()) {
               if (beforeInteraction) (*beforeInteraction)();
               ScanReset();                              /* reset [ ] handling */
               if (!GetOption(AUTORESET))                /* enter bk level if  */
                   processfile(piporthold,ECHO,"er>");   /* option & do til EOF*/
               unwindscope();                            /* unbind all locals */
               for(;;) {                                 /* back to top level  */
                   clearerr(stdin);                      /* two EOFs in a row  */
                   ScanReset();                          /* reset [ ] handling */
                   processfile(piporthold,ECHO,"-->");   /* back to stdin      */
                   if (!GetOption(IGNOREEOF)) break;     /* EOF, if !ignore    */
               }
               if (afterInteraction) (*afterInteraction)();
           } else {
               ScanReset();
               unwindscope();
               clearerr(stdin);
           }
        }
        deiniterrors();                                  /* restore exception handlers */
        return(0);
  xer:  xpop(3);
        return(-1);
    }

   /*
    | litrap(before, after)
    |
    | Sets function pointers which will be called before and after each interactive
    | session with the user. This is used on some platforms to create a new window
    | for the session and to close it afterwards.
    */
    int litrap(before, after)
    int (*before)(), (*after)();
    {
        beforeInteraction = before;
        afterInteraction  = after;
        return(0);
    }

   /*
    | This function sets up the LISP debug error trap. When an error occurs in the
    | interpreter the passed function will be called, if it returns '1' then the debugger
    | will be entered, otherwise the lidomac/lidomaca function will return without
    | entering the debugger. This function returns the old trap function so it can
    | be restored later if necessary.
    */
    int (*(litrapd(intercept)))()
        int (*intercept)();
    {
        int (*ret)() = debugIntercept;
        debugIntercept = intercept;
        return(ret);
    }

   /*
    | liadpred(name, func)
    |
    | Installs a built in function called 'name' which takes no arguements and will
    | return t or NIL depending on if 'func' returns 1 or 0 respectively. This is
    | a simple mechanism for an application to install a predicate function into the
    | interpreter without using the stub compiler. The function 'func' will be called
    | with the string 'name' as an argument to allow for one handler per class of
    | predicate if desired. Because liadpred may get called before liinit we must do
    | the initialize now if the system has not been primed.
    */
    void liadpred(name, func)
    char *name; int (*func)();
    {
        if (!liprimed) { liprime(); liprimed = 1; }
        funcinstall(FN_BUPRED, func, name, NULL);
    }

#endif

