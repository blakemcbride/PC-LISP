

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** ExtractArray(list,where) Will get an arraycell from list and put in **
 ** where. If successfull it returns 1, otherwise it returns 0. To be   **
 ** like lisp we allow quoted arrays to be passed to the built in functs**
 ** so we either get the arraycell directly from list, or from the bind **
 ** ing of the atom 'list'.                                             **
 *************************************************************************/
int ExtractArray(list,where)
struct conscell *list,**where;
{      if (list != NULL)
       {   if (list->celltype == ARRAYATOM)
           {   *where = list;
                return(1);
           };
           if (list->celltype == ALPHAATOM)
           {   if (ALPHA(list)->valstack != NULL)
               {   list = ALPHA(list)->valstack->carp;
                   if (list->celltype == ARRAYATOM)
                   {   *where = list;
                        return(1);
                   };
               };
           };
       };
       return(0);
}

/*************************************************************************
 ** ArrayAllocTree(size): Recursively makes ARRAYN-ary tree using hunks **
 ** as the nodes such that there are exactly 'size' leaves. This will   **
 ** form the basis for an array. The last entry in an internal node is  **
 ** used to store a pointer to the remainder node which stores the left **
 ** over part. If the remainder is 0 no space is allocated for it, if   **
 ** the remainder is 1 the entry is used to store the leaf, otherwise   **
 ** the remainder entry points to a node of that many entries each of   **
 ** which points to the appropriate entry. Note that an array of size 1 **
 ** is illegal, this is adopted to simplify this allocator code so that **
 ** size 1 code returns NULL indicating that data is stored in root. We **
 ** must be careful to push the root so that it does not dissapear if   **
 ** GC occurs when linking in the subtrees.                             **
 *************************************************************************/
struct hunkcell *ArrayAllocTree(size)
long int size;
{    int rem; int i;
     struct hunkcell *r,**e;
     if (size <= ARRAYM)
     {   if (size > 1)
             return(inserthunk(size));
         return(NULL);
     };
     rem = size % ARRAYM;
     size /= ARRAYM;
     r = (rem > 0) ? inserthunk(ARRAYN) : inserthunk(ARRAYM);
     xpush(r);
     for(i=0;i<ARRAYM;i++)
     {    e = (struct hunkcell **) GetHunkIndex(r,i);
         *e = ArrayAllocTree(size);
     };
     if (rem > 1)                                        /* i == ARRAYM */
     {   e = (struct hunkcell **) GetHunkIndex(r,i);
        *e = inserthunk(rem);
     };
     xpop(1);
     return(r);
}

/*************************************************************************
 ** GetArrayIndex(hunk,loc,size) : Return a pointer to the cell where   **
 ** element 'loc' of ARRAN-ary tree is stored, ie the loc'th leaf. Where**
 ** the size of the tree is 'size'. Basically we do the same computation**
 ** as we used when buidling the tree. We get the quot and remainder of **
 ** the size w.r.t ARRAYM. This tells us if the element is in the rem   **
 ** ainder part, or in the quotient parts. We then select the correct   **
 ** on, update the loc, hunk and size fields and recurse. Eventually we **
 ** will reach a point where the size is less than ARRAYM in which case **
 ** we index into the hunk and that is the pointer that we return.      **
 *************************************************************************/
struct conscell **GetArrayIndex(hunk,loc,size)
struct hunkcell *hunk;
long int loc,size;
{    int rem; int subix;
     long int subsize;
     struct hunkcell **e;
     for(;;)
     {   if (size <= ARRAYM)
             return(GetHunkIndex(hunk,loc));
         rem = size % ARRAYM;
         subsize = size/ARRAYM;
         if (size - rem <= loc)
         {   e = (struct hunkcell **) GetHunkIndex(hunk,ARRAYM);
             if (rem == 1)
                 return((struct conscell **)e);
             return(GetHunkIndex(*e,loc - ARRAYM*subsize));
         };
         if (subsize == 1L)
             return(GetHunkIndex(hunk,loc));
         subix = loc / subsize;
         hunk = *((struct hunkcell **)GetHunkIndex(hunk,subix));
         loc = loc - subix*subsize;
         size = subsize;
     };
}

/*************************************************************************
 ** ArrayAccess(info,base,dlist) Will access the array described by info**
 ** and base and use dlist, a list of actual indecies to go into the    **
 ** array and then either return the element there, or store the last   **
 ** element in dlist there. ArrayAccess is called by apply when it has  **
 ** found a form like ([array] [element] n1 ....nN ).  dlist is then the**
 ** ([element] n1......nN)   list and info and base are the two fields  **
 ** of the array found in the form passed to apply. First we extract the**
 ** size of the array from the car of the info list. Then we check the  **
 ** length of dlist against the bounds list. If they are the same length**
 ** we go ahead and use Horners to compute the location of the elemtn.  **
 ** If the are not the same length, the first element is assumed to be  **
 ** the [element] to be stored we then compute Horner's on the rest of  **
 ** the dlist agains the info dimensions list and store the element.    **
 ** Bounds and parameter checks are done to make sure all is ok.        **
 *************************************************************************/
struct conscell *arrayaccess(info,base,dlist)
struct conscell *info,*dlist;
struct hunkcell *base;
{      long int size,loc,d,i; int IsStoreOperation;
       struct conscell **e,*element,*t1,*t2;
       size = FIX(info->carp)->atom;
       info = info->cdrp;
       if ((dlist != NULL)&&(info != NULL))
       {    t1 = dlist;
            t2 = info;
            element = dlist->carp;
            while((t1 != NULL)&&(t2 != NULL))
            {   t1 = t1->cdrp;
                t2 = t2->cdrp;
            };
            IsStoreOperation = (t1 != t2);
            if (IsStoreOperation) dlist = dlist->cdrp;
            loc = 0L;
            do
            {   if (!GetFix(dlist->carp,&i)) goto TFTM;
                d = FIX(info->carp)->atom;
                if ((i < 0L) || (i >= d))
                   gerror("array: subscript out of bounds");
                loc *= d;
                loc += i;
                dlist = dlist->cdrp;
                info = info->cdrp;
            }
            while((dlist != NULL)&&(info != NULL));
            if (info == dlist)
            {   e = GetArrayIndex(base,loc,size);
                if (IsStoreOperation) *e = element;
                return(*e);
            };
 TFTM:      gerror("array: too few/many subscripts provided");
       };
       gerror("array: subscript(s) missing");
}
