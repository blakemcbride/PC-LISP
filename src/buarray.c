

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** (array name kind -dims-) Will construct an array called 'name' and  **
 ** associate it with atom 'name'. It may have any kind of element so   **
 ** we ignore the 'kind' field. The -dims- are dimensions and have to   **
 ** be evaluated so we evlis them. Next we compute the size of the array**
 ** as the product of all the -dims- checking that all the elements of  **
 ** the -dims- list are numbers. We can now construct the info list for **
 ** the array by consing the size onto the dims list making (size -dims)**
 ** We then allocate the array cell and link the info list to it. All   **
 ** that remains to be done is to allocate the hunks and link them to   **
 ** create the artificially contiguous  memory for the array elements.  **
 ** And finally to bind the array to its name either globally or locally**
 ** depending on the if there is a value stack or not. (just like setq) **
 ** Note because of AllocArrayTree returns NULL for an array of size 1  **
 ** (used for leaf case), it is no good for a root case of size 1 so we **
 ** do the size <= ARRAYM case manually rather than call AllocArrayTree.**
 *************************************************************************/
struct conscell *buarray(form)
struct conscell *form;
{      long int size,temp;
       struct alphacell *name;
       struct conscell  *t;
       struct arraycell *ar;
       struct hunkcell  *ArrayAllocTree();
       xpush(form); push(t); push(ar);
       if ((form!=NULL)&&(form->carp != NULL))
       {   name = ALPHA(form->carp);
           form = form->cdrp;
           if ((name->celltype == ALPHAATOM)&&(form != NULL))
           {   form = form->cdrp;
               if (form != NULL)
               {   form = evlis(form);               /* eval dimensions */
                   if (form == NULL) goto ERR;
                   size = 1L;
                   for(t=form; t != NULL; t = t->cdrp)
                   {   if (!GetFix(t->carp,&temp)) goto ERR;
                       size *= temp;
                   };
                   if (size < 1) goto ERR;            /* no 0 and 1 arrays */
                   t = new(CONSCELL);
                   t->carp = newintop(size);
                   t->cdrp = form;
                   ar = ARRAY(new(ARRAYATOM));
                   ar->info = t;
                   ar->base = NULL;
                   if (size <= ARRAYM)
                       ar->base = inserthunk(size);       /* size = 1 fix */
                   else
                       ar->base = ArrayAllocTree(size);
                   if (name->valstack == NULL)
                   {   bindvar(name,ar);
                       name->botvaris = GLOBALVAR;
                   }
                   else
                       name->valstack->carp = LIST(ar);
                   xpop(3);
                   return(LIST(ar));
               };
           };
       }
ERR:   ierror("array");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}

