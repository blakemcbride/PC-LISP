/****************************************************************************
 **            PC-LISP (C) 1986 Peter Ashwood-Smith.                       **
 **            MODULE: MMAN                                                **
 **------------------------------------------------------------------------**
 **   The memory management unit. There are basically 3 different type of  **
 ** object that we manipulate. The first is a variable size string of bytes**
 ** which is allocated from the heap space and is compacted periodically.  **
 ** The second is the largest cell structure called the alpha cell. These  **
 ** are allocated by calling 'newalpha()'. The last is a conglomerate of   **
 ** the other lisp cells and is as large as the largest of them. Each of   **
 ** these three kinds of object are allocated in their own set of blocks of**
 ** contiguous memory. Memory blocks are stored in block[0]..block[bc].    **
 ** They are all of size blocksize which is < 32K in length and must be    **
 ** divisible by the alphacell size and the larger of the rest of the cell **
 ** sizes to insure that even if all block are the same length all of each **
 ** block will be used for storage. This does not apply to the heap blocks.**
 ** The markers (HeapFirstBlock, HeapLastBlock) point into the array block **
 ** and describe the range of blocks which are used for the heap. The same **
 ** is true of AlphaFirstBlock, .... CellLastBlock integer variables.      **
 ** Each block of space which is allocated to the heap consists first of a **
 ** structure called HeapControl. It contains the necessary pointers to the**
 ** rest of that heap block. Within the rest of the contiguous space for   **
 ** the heap are variable length blocks whose first two bytes are control  **
 ** bytes and are not visible outside this module. See heapget().          **
 ****************************************************************************/

#include        <stdio.h>
#include        <assert.h>
#include        "lisp.h"

 static void AdjustCurrentHeap();

 struct conscell  *lifreecons;                        /* head of avail lists */
 static struct alphacell *freealpha;                  /* for cell and alpha */

 static int    cellsize;                              /* size of bigger cell */
 static long   heapout    = 0L;                       /* bytes of heap in use*/
 static long   totalcells = 0L;                       /* total cells in sys */
 static long   totalalpha = 0L;                       /* total alpha in sys */
 static long   totalheap  = 0L;                       /* total heap in sys */
        long   gccount    = 0L;                       /* number of gathers   */
        int    marking    = 0;                        /* 1 if marking */
 static long   hccount    = 0L;                       /* count of heap compactions */

 static int    blocksize;                             /* size of all blocks */
 static char  *ablock[MAXABLOCKS];                    /* array of alpha block ptrs */
 static char  *cblock[MAXCBLOCKS];                    /* array of cell block ptrs */
 static char  *hblock[MAXHBLOCKS];                    /* array of heap block ptrs */

                                                      /* indexes into (a|c|h)block[]*/
 static int    HeapLastBlock;                         /* last block in hblock[] */
 static int    AlphaLastBlock;                        /* last block in ablock[] */
 static int    CellLastBlock;                         /* last block in cblock[] */

 struct HeapControl                                   /* heap block header */
 {      char *start;                                  /* start of block    */
        char *end;                                    /* end of block      */
        char *next;                                   /* next free byte    */
        long free;                                    /* num of free bytes */
 };

 static struct HeapControl *CurrentHeap;              /* current work heap */
 static int    CurrentBlock;                          /* block of curr heap*/

 static int cellthresh = 25;                          /* percentage free cells */
 static int atomthresh = 25;                          /* percentage free atoms */

static void CompactHeapBlock();

/*************************************************************************
 ** figure out size of cell and size of block. The size of the cell is  **
 ** the size of the largest of the cons,real,fix,string and file cells. **
 ** Alpha is not included in this list because it is allocated separately*
 ** to these other cells. We compute the blocksize as being the closest **
 ** integer less than or equal to BLOCKSIZE that is divisible by both   **
 ** the size of the alphacell and the size of the other cells. We do    **
 ** this calculation by simply dividing it by the product of the two    **
 ** sizes and then remultiplying. There are better ways but... later.   **
 *************************************************************************/
 int figurecellsize()
 {   int temp;
     temp = max(sizeof(struct filecell),sizeof(struct conscell));
     temp = max(sizeof(struct realcell),temp);
     temp = max(sizeof(struct fixcell),temp);
     temp = max(sizeof(struct stringcell),temp);
     temp = max(sizeof(struct arraycell),temp);
     temp = max(sizeof(struct fixfixcell),temp);
     temp = max(sizeof(struct clispcell),temp);
     return(temp);
 }

 int figureblocksize()
 {   int f,d;
     f = figurecellsize() * sizeof (struct alphacell);
     d = BLOCKSIZE / f;
     return(d * f);
 }

/*************************************************************************
 ** initmem() - will allocate 1 block of each basic type, heap, alpha   **
 ** and cell. These are put into the hblock, ablock and cblock arrays   **
 ** respectively. New blocks will be added to each of these lists as is **
 ** necessary when garbage collection fails to reclaim enough storage.  **
 ** Once allocated we loop through setting up the free lists.           **
 *************************************************************************/
void initmem()
{   register int i,bc; register char *m;
    int diff;

   /*
    | Figure out what are reasonable sizes for the cons,string etc mem
    | blocks and how big the blocks are to be.
    */
    cellsize = figurecellsize();
    blocksize = figureblocksize();

   /*
    | Threshold values. These affect the heuristics of the garbage collector.
    | In particular, when we run out of cells we garbage collect and keep
    | track of how many we get back, if we have less than 'cellthresh'% free
    | after a GC we allocate blocks until we do have 'cellthresh'% free cells.
    | The same is true of atoms. A user primitive can alter these values to
    | experiment with different heuristics.
    */
    cellthresh = 25;
    atomthresh = 25;

   /*
    | Free up any blocks left in the hblock, cblock and ablock arrays.
    | For the moment we DO NOT DO THIS as it seems to aggrivate a memory
    | problem which I have not been able to track down, for the moment
    | rebootLISP will cause allocation of new memory.
    */
    for(bc = 0; bc < MAXHBLOCKS; bc++) {
     /*  if (hblock[bc] != NULL) free(hblock[bc]); */
         hblock[bc++] = NULL;
    }
    for(bc = 0; bc < MAXCBLOCKS; bc++) {
     /*  if (cblock[bc] != NULL) free(cblock[bc]); */
         cblock[bc++] = NULL;
    }
    for(bc = 0; bc < MAXABLOCKS; bc++) {
     /*  if (ablock[bc] != NULL) free(ablock[bc]); */
         ablock[bc++] = NULL;
    }

   /*
    | Allocate 1 block for each group and make sure we actually got them.
    | If not issue an error and abort.
    */
    ablock[0] = calloc((unsigned)blocksize,1);
    cblock[0] = calloc((unsigned)blocksize,1);
    hblock[0] = calloc((unsigned)blocksize,1);

   /*
    | Make sure that the given blocksize is making effecient use of memory
    | and not causing calloc to allocate excessivly large blocks. Eg if we
    | want ~16K blocks we do not want calloc giving us 32K blocks! This will
    | happen on some systems so if things are more than 200 bytes off, we ask
    | the developer to tune the values.
    */
    diff = cblock[0] - ablock[0];
    diff = blocksize - abs(diff);

    if (diff > 200)
        UpError("BLOCKSIZE causes > 200 bytes wasted per block, retune it!\n");

    if ((ablock[0] == NULL)||(cblock[0] == NULL)||(hblock[0] == NULL))
        UpError("not enough memory to run");

   /*
    | Set up indexes to the heap blocks all are set to the first and
    | currently only block.
    */
    HeapLastBlock   = 0;
    AlphaLastBlock  = 0;
    CellLastBlock   = 0;

   /*
    | Initialize the counts of how much memory we have to 0, also set the
    | free lists to NULL prior to constructing them.
    */
    heapout = totalcells = totalalpha = totalheap=0L;
    freealpha = NULL;
    lifreecons  = NULL;

   /*
    | Loop through each of the ablock blocks and build a list of free alpha
    | cells for use by new(ALPHACELL).
    */
    for(i=0; i <= AlphaLastBlock; i++)
    {   for(m=ablock[i]; m<(ablock[i]+blocksize); m+=sizeof(struct alphacell))
        {   ALPHA(m)->celltype = ALPHAATOM;
            ALPHA(m)->markbit = CLEAR;
            ALPHA(m)->permbit = NOT_PERM;
            ALPHA(m)->valstack = LIST(freealpha);
            ALPHA(m)->atom = NULL;
            freealpha = ALPHA(m);
        }
        totalalpha += blocksize / sizeof(struct alphacell);
    }

   /*
    | Loop through each of the cblock blocks and build a list of free cons
    | cells for use by new(CONSCELL);
    */
    for(i=0; i <= CellLastBlock; i++)
    {   for(m=cblock[i]; m<(cblock[i]+blocksize); m+=cellsize)
        {   LIST(m)->celltype = CONSCELL;
            LIST(m)->travbit = LIST(m)->markbit = CLEAR;
            LIST(m)->carp = lifreecons;
            LIST(m)->cdrp = NULL;
            lifreecons = LIST(m);
        }
        totalcells += blocksize / cellsize;
    }

   /*
    | Loop through each heap block and set its header up as a heap control
    | area whose start points to the first free byte, end points to the end
    | and whose next and free controll allocation.
    */
    for(i=0; i <= HeapLastBlock; i++)
    {   m = hblock[i];
        ((struct HeapControl *)m)->start=m + sizeof(struct HeapControl);
        ((struct HeapControl *)m)->end  =m + blocksize;
        ((struct HeapControl *)m)->next =m + sizeof(struct HeapControl);
        ((struct HeapControl *)m)->free =blocksize-sizeof(struct HeapControl);
        heapout += sizeof(struct HeapControl);
    };

   /*
    | Compute how much memory of each class we currently have available and
    | store it in static globals for use by the query routines and in deciding
    | when to allocate more space. etc.
    */
    totalheap = (long) blocksize * (long)(HeapLastBlock + 1);

   /*
    | Initialize the current heap pointer for use by the allocation of heap
    | space.
    */
    CurrentBlock = 0;
    CurrentHeap  = (struct HeapControl *)hblock[CurrentBlock];
}

/*
 |  Try to expand memory by a alpha bytes, c cons bytes and h heap bytes.
 |  0 means all was ok. -1 means an error occured allocating the memory.
 */
int liexpmem(a,c,h)
    long int a,c,h;
{
    register int i, k; register char *m;

   /*
    | Check the arguments, negative values mean, give me everything I can get so
    | become the block counts remaining for each type. Positive values are taken
    | as the number of bytes to increment the given pool by so are converted to
    | blockcounts.
    */
    if (a < 0)
        a =  MAXABLOCKS - AlphaLastBlock;
    else
        a = (a + blocksize) / blocksize;
    if (c < 0)
        c = MAXCBLOCKS - CellLastBlock;
    else
        c = (c + blocksize) / blocksize;
    if (h < 0)
        h = MAXHBLOCKS - HeapLastBlock;
    else
        h = (h + blocksize) / blocksize;

   /*
    | If request would blow limit, reduce request to just reach the limit.
    */
    if (h + HeapLastBlock  > MAXHBLOCKS) h = MAXHBLOCKS - HeapLastBlock;
    if (c + CellLastBlock  > MAXCBLOCKS) c = MAXCBLOCKS - CellLastBlock;
    if (a + AlphaLastBlock > MAXABLOCKS) a = MAXABLOCKS - AlphaLastBlock;

   /*
    | Allocate each of the blocks, if we ever get a NULL back return -1.
    */
    k = AlphaLastBlock;
    for(i = 0; i < a; i++)
        if ((ablock[++k] = calloc((unsigned)blocksize,1)) == NULL) return(-1);
    k = HeapLastBlock;
    for(i = 0; i < h; i++)
        if ((hblock[++k] = calloc((unsigned)blocksize,1)) == NULL) return(-2);
    k = CellLastBlock;
    for(i = 0; i < c; i++)
        if ((cblock[++k] = calloc((unsigned)blocksize,1)) == NULL) return(-3);

   /*
    | Loop through each of the ablock blocks and build a list of free alpha
    | cells for use by new(ALPHACELL).
    */
    k = AlphaLastBlock + 1;
    for(i = 0; i < a; i++) {
        for(m=ablock[k]; m<(ablock[k]+blocksize); m+=sizeof(struct alphacell)) {
            ALPHA(m)->celltype = ALPHAATOM;
            ALPHA(m)->markbit = CLEAR;
            ALPHA(m)->permbit = NOT_PERM;
            ALPHA(m)->valstack = LIST(freealpha);
            ALPHA(m)->atom = NULL;
            freealpha = ALPHA(m);
        }
        k += 1;
        totalalpha += blocksize / sizeof(struct alphacell);
    }

   /*
    | Loop through each of the cblock blocks and build a list of free cons
    | cells for use by new(CONSCELL);
    */
    k = CellLastBlock + 1;
    for(i = 0; i < c; i++) {
        for(m=cblock[k]; m<(cblock[k]+blocksize); m+=cellsize) {
            LIST(m)->celltype = CONSCELL;
            LIST(m)->travbit = LIST(m)->markbit = CLEAR;
            LIST(m)->carp = lifreecons;
            LIST(m)->cdrp = NULL;
            lifreecons = LIST(m);
        }
        k += 1;
        totalcells += blocksize / cellsize;
    }

   /*
    | Loop through each heap block and set its header up as a heap control
    | area whose start points to the first free byte, end points to the end
    | and whose next and free controll allocation.
    */
    k = HeapLastBlock + 1;
    for(i = 0; i < h; i++) {
        m = hblock[k];
        ((struct HeapControl *)m)->start=m + sizeof(struct HeapControl);
        ((struct HeapControl *)m)->end  =m + blocksize;
        ((struct HeapControl *)m)->next =m + sizeof(struct HeapControl);
        ((struct HeapControl *)m)->free =blocksize-sizeof(struct HeapControl);
        heapout += sizeof(struct HeapControl);
        k += 1;
    }

   /*
    | Increase the heap blocks to the proper limits.
    */
    HeapLastBlock   += h;
    AlphaLastBlock  += a;
    CellLastBlock   += c;

   /*
    | Recompute the total heap space available and return.
    */
    totalheap = (long) blocksize * (long)(HeapLastBlock + 1);
    return(0);
}

/*************************************************************************
 ** new(t) : making a request for a new memory cell of type 't'. We get **
 ** next available cell from the available list  and return it. If we   **
 ** run out, we initiate the marking and gathering algorithms to collect**
 ** the garbage. We then try to honour the request again, if we have no **
 ** luck we must be out of memory. When this happens we try to allocate **
 ** a new cell block in the cblock array and then allocate all of the   **
 ** cells as free cons cells. After the gather we look at how many cells**
 ** were actually reclaimed and if more than the minimum continuation   **
 ** value 'mingather' were gathered then we allow the new to continue.  **
 ** On the other hand if less than mingather were found then we will    **
 ** allocate a new block anyway in anticipation of running out.         **
 *************************************************************************/
struct conscell *newcons(t)
int t;
{      register struct conscell *r;
       register char *m; long int n;

      /*
       | If compiled with -DGCFREQ=N the new() routine will force a GC
       | every N new's. This is used to cause a higher frequency of GC
       | and thus a higher probability of falling in a GC window. This
       | makes debugging the mark stack use much easier and allows an
       | exhaustive check to PROOVE that a built in function has no GC
       | window.
       */
#      if defined(GCFREQ)
          static int nextGc = 0;
          if ((nextGc = (nextGc + 1) % GCFREQ) == 0) goto Gc;
#      endif

retry: if (lifreecons != NULL) {
           r = lifreecons;
           lifreecons = lifreecons->carp;
           r->celltype = t;
           r->carp = NULL;                      /* the cdrp is already NULL */
           return(r);
       }
       marking = 1; mark(); gather(&n, NULL); marking = 0;

      /*
       | Garage collection is done, we have gathered 'n' free cells, so, if
       | we have at least 'celthresh' percent free cells then we continue,
       | otherwise we allocate a block and come back here. This means that
       | the LISP interpreter will always maintain 'celthresh' percent of
       | overhead in anticipation of its need. Just before we exit, we check
       | if the user wants gcprinting, if so we print the memory statistics.
       */
again:
       if ((lifreecons != NULL) && (((n * 100)/totalcells) > cellthresh)) {
           r = lifreecons;
           lifreecons = lifreecons->carp;
           r->celltype = t;
           r->carp = NULL;                      /* the cdrp is already NULL */
           if (TestForNonNil("$gcprint",0)) printstats();
           SetLongVar("$gccount$",gccount);
           return(r);
       }

      /*
       | We're out of memory so try to allocate a new block and store its
       | base address in cblock[]. If there are no more slots in cblock or
       | there is no more memory and we are totally out of memory then throw
       | the cons cell space exhausted error otherwise we cannot maintain a
       | 25% free threshold so just let the system thrash till it really runs
       | out.
       */
       if ( (CellLastBlock >= (MAXCBLOCKS-1)) || (! (m = calloc((unsigned)blocksize,1)))) {
           if (! lifreecons ) gerror("cons cell space exhausted");
           goto retry;
       }

      /*
       | We got the memory so remember it in our cell block array.
       */
       cblock[++CellLastBlock] = m;

      /*
       | We got a block in 'm' so loop through it formatting the cells
       | as CONSCELL and then put them on the free list. When done we
       | jump back to the 'again' label to see if we have reached the
       | celthresh percentage of free memory which we are supposed to
       | maintain.
       */
       for(; m<(cblock[CellLastBlock]+blocksize); m+=cellsize) {
           LIST(m)->celltype = CONSCELL;
           LIST(m)->travbit = LIST(m)->markbit = CLEAR;
           LIST(m)->carp = lifreecons;
           LIST(m)->cdrp = NULL;
           lifreecons = LIST(m);
       }
       n += blocksize / cellsize;
       totalcells += blocksize / cellsize;
       goto again;
}

/**********************************************************************
 ** newalpha()  Allocate an alpha cell from the list of alpha cells. **
 ** If we run out do a mark and gather and try again. Free alphas    **
 ** are linked on the unused 'valstack' field. If after garbage has  **
 ** been collected there is still no space then try to allocate a new**
 ** alpha block cell on the ablock array. If this fails then throw   **
 ** the out of alpha space error.                                    **
 **********************************************************************/
struct alphacell *newalpha()
{      register struct alphacell *r;
       register char *m; long int n;

      /*
       | If compiled with -DGCFREQ=N the new() routine will force a GC
       | every N new's. This is used to cause a higher frequency of GC
       | and thus a higher probability of falling in a GC window. This
       | makes debugging the mark stack use much easier and allows an
       | exhaustive check to PROOVE that a built in function has no GC
       | window.
       */
#      if defined(GCFREQ)
          static int nextGc = 0;
          if ((nextGc = (nextGc + 1) % GCFREQ) == 0) goto Gc;
#      endif

retry: if  (freealpha != NULL) {
            r = freealpha;
            freealpha = ALPHA(freealpha->valstack);
            r->celltype = ALPHAATOM;
            return(r);
       }

       marking = 1; mark(); gather(NULL, &n); marking = 0;

      /*
       | Garage collection is done, we have gathered 'n' free cells, so, if
       | we have at least 'alphathresh' percent free cells then we continue,
       | otherwise we allocate a block and come back here. This means that
       | the LISP interpreter will always maintain 'celthresh' percent of
       | overhead in anticipation of its need. When we finally get out of
       | new we check to see if the user wants gcprinting and if so dump it.
       */
again:
       if ((freealpha != NULL) && (((n * 100)/totalalpha) > atomthresh)) {
            r = freealpha;
            freealpha = ALPHA(freealpha->valstack);
            r->celltype = ALPHAATOM;
            if (TestForNonNil("$gcprint",0)) printstats();
            SetLongVar("$gccount$",gccount);
            return(r);
       }

      /*
       | We're out of memory so try to allocate a new block and store its
       | base address in ablock[]. If there are no more slots in ablock or
       | there is no more memory and we are totally out of memory then throw
       | the alpha cell space exhausted error otherwise we cannot maintain a
       | 25% free threshold so just let the system thrash till it really runs
       | out..
       */
       if ( (AlphaLastBlock >= (MAXABLOCKS-1)) || (! (m = calloc((unsigned)blocksize,1)))) {
           if (! freealpha ) gerror("alpha cell space exhausted");
           goto retry;
       }

      /*
       | We got the memory so remember it in our alpha block array.
       */
       ablock[++AlphaLastBlock] = m;

      /*
       | We got a block in 'm' so loop through it formatting the cells
       | as ALPHA and then put them on the free list. When done we
       | jump back to the 'again' label to try to allocate a cell again.
       | which should not fail again.
       */
       for(; m<(ablock[AlphaLastBlock]+blocksize); m+=sizeof(struct alphacell)) {
           ALPHA(m)->celltype = ALPHAATOM;
           ALPHA(m)->markbit = CLEAR;
           ALPHA(m)->permbit = NOT_PERM;
           ALPHA(m)->valstack = LIST(freealpha);
           ALPHA(m)->atom = NULL;
           freealpha = ALPHA(m);
       }
       n += blocksize / sizeof(struct alphacell);
       totalalpha += blocksize / sizeof(struct alphacell);
       goto again;
}

/**********************************************************************
 ** heapget(n) Allocate n bytes from the heap and return a pointer to**
 ** this storage area. We check the current heap to see if it has    **
 ** n+2 bytes of storage. The extra bytes are used to store control  **
 ** info about the block we are allocating. These bytes are the size **
 ** which is stored in the first 2 bytes, the heap block which this  **
 ** block of memory belongs to, then then entire 'n' bytes. The value**
 ** returned is pointr part of the block and the control data is not **
 ** known to anyone but the heap management routines. Note that we   **
 ** use the second byte (the block number) to indicate if the block  **
 ** is in use or not. A value of 255 in this block number means that **
 ** the block is free otherwise the block is in use. We are careful  **
 ** to check that the largest block number does not exceed 254 in the**
 ** initmem() routine at the start of this module after step (3).    **
 **                                                                  **
 **                (pointer to block)                                **
 **                         |                                        **
 **                         V                                        **
 **   SIZEH SIZEL | BLOCK | DATA1......DATAN-2                       **
 **                                                                  **
 ** SIZE is a byte whose value is 3..    which indicates the total   **
 ** number of bytes in the block. BLOCK is a byte whose value if less**
 ** than 255 indicates which of the elements of the array 'block[]'  **
 ** contains a pointer to the HeapControl in which this block lies.  **
 ** A value other than 255 also indicates that the block is in use.  **
 ** So, a value of 255 means that the block is free. Next come the   **
 ** Data bytes. The pointer to this object is pointing to the first  **
 ** of the data bytes.                                               **
 **********************************************************************/
char *heapget(n)
int  n;
{    register char *r;
     n += 4;
     if (n >= blocksize) fatalerror("heapget");
     if (CurrentHeap->end - CurrentHeap->next < n)
         AdjustCurrentHeap(n);
     r = CurrentHeap->next;
     CurrentHeap->next += n;
     CurrentHeap->free -= n;
     heapout += n;
     *r++ = (n>>8)&0xff;                /* set HIGH BYTE OF SIZE field and advance */
     *r++ = n&0xff;                     /* set LOW BYTE OF SIZE field and advance */
     *r++ = (CurrentBlock>>8) & 0xff;   /* set HIGH BYTE OF BLOCK field (~255 means block in use) */
     *r++ = (CurrentBlock & 0xff);      /* set LOW BYTE OF BLOCK field (says block in use) */
     return(r);                         /* return pointer to data area.    */
}

/*
 | Extract the SIZE of a heap block in bytes. Basically a two byte fetch and
 | compute as a short value.
 */
#define SIZE(f)  ((((*f) & 0xff) << 8) + (*(f+1) & 0xff))
#define BLOCK(f) ((((*f) & 0xff) << 8) + (*(f+1) & 0xff))

/***************************************************************************
 ** freeheapblock(b). A block as describ in heapget() documentation above **
 ** has been freed. First we back up the pointer once and get the BLOCK   **
 ** value. We then set the field to 255 to indicate that it is free. Next **
 ** we back up twice more retreiving the SIZE   field. We then use the    **
 ** value that was in the block field to get the HeapControl structure    **
 ** which is responsible for this block. We the increment the free field  **
 ** for this Heap block by the size of the freed block. We are now done.  **
 ***************************************************************************/
static void freeheapblock(b)
unsigned char *b;
{    register int siz,blk;
     b -= 2;                                        /* backup to BLOCK field */
     blk = BLOCK(b);                                /* retrieve BLOCK field */
     *b = 255;                                      /* set it free */
     b -= 2;                                        /* back up to front */
     siz = SIZE(b);                                 /* load HIGH BYTE OF SIZE unsigned */
     assert(siz >= 0);
     ((struct HeapControl *)hblock[blk])->free+=siz;/* update free count*/
     heapout -= siz;                                /* update total free count */
}

/***************************************************************************
 ** ScanToBusyFrom(f,t). Scan forwards from 'f' until a block that is busy**
 ** is found, or we pass 't'. If we pass 't' return NULL, otherwise return**
 ** pointer to the first character in the Busy block.                     **
 ***************************************************************************/
#define ScanToBusyFrom(ff,t,r)                            \
{    register char *f = (ff);                             \
     while((f < t)&&((*(f+2) & 0xff)==255)) f += SIZE(f); \
     r = (f < t) ? f : NULL;                              \
}

/***************************************************************************
 ** ScanToFreeFrom(f,t). Scan forwards from 'f' until a block that is free**
 ** is found, or we pass 't'. If we pass 't' return NULL, otherwise return**
 ** pointer to the first character in the free block.                     **
 ***************************************************************************/
#define ScanToFreeFrom(ff,t,r)                            \
{    register char *f = (ff);                             \
     while((f < t)&&((*(f+2) & 0xff)!=255)) f += SIZE(f); \
     r = (f < t) ? f : NULL;                              \
}

/***************************************************************************
 ** InformRelocating(f,t,s) Inform each owner of blocks in 'f' that they  **
 ** are being relocated backwards by 's' bytes. We call the FindReferent  **
 ** function in main.c to get us the pointer to the pointer to the block  **
 ** of storage that we are about to relocate, we then shift back the      **
 ** referents pointer by the amount of the relocation compression 's'.    **
 ** Note that the two parameters to FindReferent are the pointer to the   **
 ** heap space that the Referent owns, and the length of the block that   **
 ** he owns. The length of the block as far as FindReferent is concerned  **
 ** does not include the header info so we subtract 2 from the size field.**
 ***************************************************************************/
static void InformRelocating(f,t,s)
char *f,*t; int s;
{    register char **p;
     register int len;
     while((f < t)&&((*(f+2) & 0xff)!=255))
     {     len = SIZE(f);
           if ((p = FindReferent(f + 4, len - 4)) == NULL)
                fatalerror("InformRelocating");
           *p -= s;                   /* shift back the referents pointer */
           f += len;                  /* f = f + length(BLOCK(f)) */
     };
}

/***************************************************************************
 ** RelocateGroup(t,f,s): Relocate a group from 'f' to 't' whose size is  **
 ** 's' bytes in total. This is just a memcpy but we must be sure that an **
 ** overlapped source/destination will work so we code it ourselves.      **
 ***************************************************************************/
#define RelocateGroup(tt,ff,ss)    \
{    register char *t,*f; int s;   \
     t = (tt); f = (ff); s = (ss); \
     while(s--) *t++ = *f++;       \
}

/***************************************************************************
 ** AdjustCurrentHeap(n) - a request for 'n'-2 bytes has been made by the **
 ** lisp interpreter via the heapget() routine above. However the current **
 ** heap block as described by CurrentHeap and CurrentBlock does not have **
 ** enough free space at the end of the block. It may have enough in a    **
 ** free block somewhere or in the total of the sizes of its free blocks. **
 ** The HeapControl field 'free' reflects the total number of bytes that  **
 ** are free scattered about in the entire heap block. We thus make a scan**
 ** of all the heapblocks (CurrentBlock+1...LastHeapBlock...CurrentBlock  **
 ** looking for a free field > n. When we find one we that has enough room**
 ** to service our request we check to see if the space it has free is in **
 ** the ->next .. ->end part of the heap space. If so we do not have to   **
 ** compact to get it so we can return. If not the free space is made up  **
 ** of lots of small fragments so we must call CompactCurrentHeap() to    **
 ** make all the small fragments into one big space in ->next .. ->end. If**
 ** after a complete trip through the heaps we find no space, we will call**
 ** the garbage collector to reclaim alpha and conscells. Since some alpha**
 ** cells are bound to be freed up we will regain some heap space so we   **
 ** go looking for it. We should find it this time, otherwise there is no **
 ** way to service the request so we abort with out of heap space message.**
 ** Note that the scan sequence is important. We want to avoid compacting **
 ** if possible so we always start at CurrentBlock+1. This will have the  **
 ** most free space in it since it was least recently the CurrentBlock.   **
 ***************************************************************************/
static void AdjustCurrentHeap(n)
int n;
{       register int i;
        register char *m=NULL;

        for(i=CurrentBlock+1; i <= HeapLastBlock; i++)
            if (((struct HeapControl *)hblock[i])->free > n)
               goto gotit;
        for(i=0; i <= CurrentBlock; i++)
            if (((struct HeapControl *)hblock[i])->free > n)
               goto gotit;

        marking = 1;                    /* not enough free space so lets */
        mark();                         /* gather all free cells and get */
        gather(NULL,NULL);              /* some free alpha print names */
        marking = 0;

       /*
        | If garbage collection freed up 25% or more of the memory then just find
        | an appropriate heap block and continue.
        */
        if ((((totalheap - heapout) * 100) / totalheap) > atomthresh) {
            for(i=CurrentBlock+1; i <= HeapLastBlock; i++)
               if (((struct HeapControl *)hblock[i])->free > n)
                  goto gotaftergc;
            for(i=0; i <= CurrentBlock; i++)
               if (((struct HeapControl *)hblock[i])->free > n)
                  goto gotaftergc;
        }

       /*
        | Garbage collection did not yield enough memory so allocate blocks until
        | we are back to the proper 25% free margin. Or, there is 25% free but it is
        | spread in such a way that no block has enough memory to yeild to the request
        | in this case just one block will be allocated.
        */
        do {

          /*
           | We're out of memory so try to allocate a new block and store its
           | base address in hblock[]. If there are no more slots in hblock or
           | there is no more memory then we cannot maintain a 25% free threshold
           | so allow thrashing until we really run out of memory.
           */
           if ((HeapLastBlock >= (MAXHBLOCKS-1)) || (!(m = calloc((unsigned)blocksize,1)))) {
               for(i = HeapLastBlock; i >= 0; i--)
                   if (((struct HeapControl *)hblock[i])->free > n) goto gotit;
               gerror("heap space exhausted");
           }

          /*
           | We got a block so add set a memory block pointer to it.
           */
           hblock[++HeapLastBlock] = m;

          /*
           | We got a block so format it as a heap control block and make it
           | the current block. Note we set i so that the 'gotit' label will
           | set the current block properly.
           */
           ((struct HeapControl *)m)->start=m + sizeof(struct HeapControl);
           ((struct HeapControl *)m)->end  =m + blocksize;
           ((struct HeapControl *)m)->next =m + sizeof(struct HeapControl);
           ((struct HeapControl *)m)->free =blocksize-sizeof(struct HeapControl);
           heapout += sizeof(struct HeapControl);
           totalheap += (long) blocksize;

        } while((((totalheap - heapout) * 100) / totalheap) <= atomthresh);
        i = HeapLastBlock;

       /*
        | Does user want to know when GC occurs? If so tell him now and up the
        | GC count because we just did a GC above.
        */
gotaftergc:

        if (TestForNonNil("$gcprint",0)) printstats();
        SetLongVar("$gccount$",gccount);

gotit:  CurrentBlock = i;
        CurrentHeap = (struct HeapControl *)hblock[i];
        if (CurrentHeap->end - CurrentHeap->next < n)
            CompactHeapBlock(CurrentHeap);
}

/***************************************************************************
 ** CompactHeapBlock(b): We have been asked to move all the free space    **
 ** in the heap whose control structure is pointed to by 'b' to the end   **
 ** of the array. We do this by advancing 3 pointers 'x','y', and 'z' and **
 ** relocating memory between them. 'x' advances to the first free block. **
 ** 'y' to the next busy block after 'x', and 'z' to the next free block  **
 ** after 'y'. We can then more z-y bytes from x to y. Here is a picture. **
 ** where B represents a Busy block and X a free block.                   **
 **                                                                       **
 **      | B0| X | X | B1| B2| B3| X | X | X | X | B4| X ...............  **
 **          ^       ^           ^                                        **
 **          x       y           z                                        **
 **                                                                       **
 ** After first sequence of busy blocks has been relocated. Note that ~~~ **
 ** represents a part of memory in which the heap structure is unsound.   **
 **                                                                       **
 **      | B0| B1| B2| B3|~~~~~~~| X | X | X | X | B | X ...............  **
 **          ^       ^           ^                                        **
 **          x       y           z                                        **
 **                                                                       **
 ** Next we must reposition x,y and z for the next relocation. We start   **
 ** by setting 'x' to the start of the first free block. In this case it  **
 ** is at 'x' + 'z'-'y'. This will always be the case from now on. Next   **
 ** 'y' is advanced to the next busy block, and finally 'z' is advanced   **
 ** from the position 'y' is at to the next free block. The result is:    **
 **                                                                       **
 **      | B0| B1| B2| B3|~~~~~~~| X | X | X | X | B | X ...............  **
 **                      ^                       ^   ^                    **
 **                      x                       y   z                    **
 **                                                                       **
 ** When we are done 'x' will represent the new 'next' value for the heap.**
 ** Note that before we relocate a block we must inform anyone who has a  **
 ** pointer to it that it is being relocated backward by 'z-y' bytes. The **
 ** only place where pointers to the heap occur is in the atom fields.    **
 ***************************************************************************/
static void CompactHeapBlock(b)
 struct HeapControl *b;
 {      register char *x,*y,*z,*end;
        hccount += 1;
        end = b->next;
        ScanToFreeFrom(b->start,end,x);
        if (x == NULL) return;
        ScanToBusyFrom(x,end,y);
        while(y != NULL)
        {   ScanToFreeFrom(y,end,z);
            if (z == NULL) z = end;
            InformRelocating(y,z,y-x);
            RelocateGroup(x,y,z-y);
            x = x + (z - y);
            ScanToBusyFrom(z,end,y);
        };
        b->next = x;                          /* reset heap next free spot. */
        b->free = b->end - b->next;           /* reset total free bytes. */
 }

/***************************************************************************
 ** unmark() : Backoff the mark() procedure by resetting all of the cell  **
 ** mark bits back to CLEAR. This is necessary if the mark() procedure was**
 ** unable to complete due to a stack overflow. After the stack overflow  **
 ** we unmark all the cells and restart the mark() & gather() routines.   **
 ** If we were to restart the mark() procedure without undoing the marking**
 ** already done we would never reach the list where the stack overflow   **
 ** occured because the mark bits would be set and would stop the recurs- **
 ** from entering the critical area.                                      **
 ***************************************************************************/
void unmark()
{   register int i; register char *m;
    for(i=0; i <= AlphaLastBlock; i++)
    {   for(m=ablock[i]; m<(ablock[i]+blocksize); m+=sizeof(struct alphacell))
            ((struct alphacell *)m)->markbit = CLEAR;
    };
    for(i=0; i <= CellLastBlock; i++)
    {   for(m=cblock[i]; m<(cblock[i]+blocksize); m+=cellsize)
            ((struct conscell *)m)->markbit = CLEAR;
    };
}

/***************************************************************************
 ** gather() : The second half of mark&gather algorithm. Ther marking alg **
 ** in the parser, will set the markbit field in all cells that are to be **
 ** kept. We traverse first the cell space and link all cells that have   **
 ** the mark bit CLEAR onto a lifreecons list. When this is done we do the  **
 ** same thing to the alpha space. We traverse it and gather cells that   **
 ** do not have the markbit SET. We however check the permbit before we   **
 ** gather an alpha cell because it could be a permanent alphaatom in     **
 ** which case we do not gather it. If all is ok we link the alphacell to **
 ** the freealpha list. Next since the cell has been freed we wish to     **
 ** remove it from the alphatable in module main.c and free up any space  **
 ** that was used for the 'atom' field of the atom. Note that we only do  **
 ** these two things if the atom field is non NULL. If it is NULL it means**
 ** that the cell was free to begin with and hence is not in the alphatab **
 ** or holds any heap space for its print name. After freeing it up we set**
 ** the name field back to NULL to avoid the same problem next time. We   **
 ** return the number of cells actually reclaimed, this allows us the new **
 ** routines to decide it is worth allocating a new block or not, ie if we**
 ** GC but do not gather much garbage then we will run out again soon so  **
 ** we allocate another block right away in anticipation. This speeds up  **
 ** things a great deal.                                                  **
 ***************************************************************************/
int gather(cgot,agot)
      long int *cgot,*agot;
{     register char *r,*memend; register int i;
      register int gotcount;
      gccount++;
      lifreecons = NULL;
      gotcount = i = 0;
      while(i <= CellLastBlock) {
            r = cblock[i++];
            memend = r + blocksize;
            while( r < memend) {
                if (LIST(r)->markbit == CLEAR) {
                    switch(LIST(r)->celltype) {
                        case STRINGATOM:
                             removestring(r);
                             freeheapblock(STRING(r)->atom);
                             break;
                        case HUNKATOM:
                             removehunk(r);
                             freeheapblock(HUNK(r)->atom);
                             break;
                        case CLISPCELL:
                             free(CLISP(r)->code - sizeof(int));               /* not stored in hunk yet so free/calloc */
                             free((char *)(CLISP(r)->literal) - sizeof(int));  /* these arrays whose size is at -1 offset */
                             break;
                    }
                    LIST(r)->carp = lifreecons;
                    LIST(r)->cdrp = NULL;            /* new() expects this */
                    LIST(r)->celltype = CONSCELL;    /* stop double removal */
                    lifreecons = LIST(r);
                    gotcount++;                      /* track how many we got back */
                }
                LIST(r)->markbit = CLEAR;
                r += cellsize;
            }
      }
      if (cgot) *cgot = gotcount;                    /* tell caller how many we got */
      freealpha = NULL;
      gotcount = i = 0;
      while(i <= AlphaLastBlock) {
            r = ablock[i++];
            memend = r + blocksize;
            while( r < memend) {
                if (ALPHA(r)->markbit == CLEAR) {
                    if (ALPHA(r)->permbit == NOT_PERM) {
                        if (ALPHA(r)->atom != NULL) {
                             removeatom(r);
                             freeheapblock(ALPHA(r)->atom);
                        }
                        ALPHA(r)->valstack = LIST(freealpha);
                        freealpha = ALPHA(r);
                        ALPHA(r)->atom = NULL;
                    }
                    gotcount++;                      /* track how many we got back */
                }
                ALPHA(r)->markbit = CLEAR;
                r += sizeof(struct alphacell);
            }
      }
      if (agot) *agot = gotcount;                    /* tell caller how many we got */
      return(gotcount);
}

/***************************************************************************
 ** FreeAlphaCount() and FreeCellCount() - will descend the freealpha or  **
 ** freecell list and count the number of cells. This value is returned.  **
 ** This is slower than keeping stats in new() and gather() but it speeds **
 ** up new() and gather() by a few percent so is a worthwhile speed up.   **
 ***************************************************************************/
long int FreeAlphaCount()
{    register long int i; register struct alphacell *t;
     for(t=freealpha,i=0L;t!=NULL;t=ALPHA(t->valstack),i++);
     return(i);
}
                                                        /* and free cells */
long int FreeConsCount()
{    register long int i; register struct conscell *t;
     for(t=lifreecons,i=0L;t!=NULL;t=t->carp,i++);
     return(i);
}

/***************************************************************************
 ** memorystatus(n), a window into the memory manager. It just returns    **
 ** one of the three percentages listed above. The parameter 'n' selects  **
 ** which percentage it will return. This is used by the built in function**
 ** memstat, which calls it three times to built its return list. Addition**
 ** options 3, 4 and 5 cause it to return the total number of bytes of    **
 ** cell alpha and heap space currently in use.                           **
 ***************************************************************************/
long int memorystatus(n)
int n;
{    switch(n)
     {  case 0 : return((long)(((totalcells-FreeConsCount())*100L)/totalcells));
        case 1 : return((long)(((totalalpha-FreeAlphaCount())*100L)/totalalpha));
        case 2 : return((long)((heapout*100L)/totalheap));
        case 3 : return((long)(totalcells * cellsize));
        case 4 : return((long)(totalalpha * sizeof(struct alphacell)));
        case 5 : return((long)totalheap);
       default : fatalerror("memorystatus");
     };
     return 0L;  /*  keep compiler happy  */
}

/***************************************************************************
 ** printstats() When $gcprint is non nil this procedure is called. It    **
 ** dumps the percentage of each celltype available, the percentage of    **
 ** heap that appears to be in use, and the number of gc's so far. This is**
 ** an expensive call as it counts the number of free cells of each kind! **
 ***************************************************************************/
void printstats()
{    printf("\n--- #gc=%ld, #hc=%ld, cons=%ld%% of %ld, alpha=%ld%% of %ld, heap=%ld%% of %ld---\n",
            gccount, hccount,
            memorystatus(0), memorystatus(3),
            memorystatus(1), memorystatus(4),
            memorystatus(2), memorystatus(5));
}

/***************************************************************************
 ** CopyCellIfPossible(dest,source) - Will copy the source cell over the  **
 ** dest cell if doing so will not corrupt the structure and integrity of **
 ** the LISP memory management. Dest and source cells must be  such that  **
 ** both are allocated from the same type of base cell. In particular we  **
 ** we can copy a fixnum, flonum, port  and list cell onto a list cell.   **
 ** We cannot copy a string, hunk or alphatom onto a list cell because.   **
 ** An alphacell is bigger than a list cell, and both a string and hunk   **
 ** are linked into the hashtable and copying the cell would require that **
 ** we delete the old one, which we cannot do because that would destroy  **
 ** any pointers that point to it. We could possibly copy a string or a   **
 ** hunk to get the desired effect (Macro Splicing) but that seems like a **
 ** lot of work for a small number of Macro Return cases. We return a 1   **
 ** if a copy was feasible, and a 0 if the copy is not feasable. We only  **
 ** copy the cell if the displace-macros variable is set non-nil, the def **
 ** ault is 0 (meaning if the variable is not present then assume it nil. **
 ***************************************************************************/
int CopyCellIfPossible(d,s)
char *d,*s;
{   register int n;
    if ((d != NULL)&&(s != NULL))
    {   if (LIST(d)->celltype == CONSCELL)
        {   switch(LIST(s)->celltype)
            {   case CONSCELL :
                case FIXATOM  :
                case FIXFIXATOM:
                case REALATOM :
                case FILECELL :
                case CLISPCELL:
                     if (TestForNonNil("displace-macros",0))
                     {   n = cellsize;
                         while(n--) *d++ = *s++;
                         return(1);
                     };
            };
        };
    };
    return(0);
}

/***************************************************************************
 ** lifreelist(l) - Will take a list of cons cells and free them, ie it   **
 ** adds them back to the lifreecons list. This is done when we KNOW for  **
 ** sure that some cells are infact garbage. This occurs in eval when the **
 ** evlis() call is made. The list is effectively garbage after the call  **
 ** has been made so lifreelist is called to gather it up immediately.    **
 ** This one little optimization results in a 40% improvment in the       **
 ** performance of the LISP interpreter. Note that this technique is also **
 ** used in popvariables to free up the shallow stack cons cells.         **
 ***************************************************************************/
void lifreelist(l)
struct conscell *l;
{   register struct conscell *f = lifreecons;
    while(l != NULL) {
        l->carp = f;
        f = l;
        l = l->cdrp;
        f->cdrp = NULL;
    }
    lifreecons = f;
}
