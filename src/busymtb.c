/* EDITION AC04, APFUN PAS.792 (92/03/03 16:29:32) -- CLOSED */                 
/* */
/*
 | PC-LISP (C) 1989-1992 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include "lisp.h"

/*
 |  BUSYMTB
 |  ~~~~~~~
 |  This file contains the definitions of the symbol table manipulation primitives
 |  symtab-*. They basically manipulate an arbitrarily large set of (k.e) pairs
 |  which is unique in 'k'. These symbol tables can be created, listed, tested
 |  destructively added to, removed from and queried as to their size. They are
 |  very similar to the set-* primitives in implementation except that the symtabs
 |  are destructively modified unlike the set-*'s which are constructive.
 |
 |         (symtab-create ((k.e)...))            -> ST
 |         (symtab-list   ST)                    -> ((k.e) ... )
 |         (symtab-member ST k)                  -> (k.e) | nil
 |         (symtab-add    ST k e)                -> ST | nil if clash
 |         (symtab-remove ST k)                  -> (k.e) | nil if not found
 |         (symtab-size   ST)                    -> (length(symtab-list ST))
 |
 |  In order to handle the above functions quickly we use a symbol table keyed
 |  on the hash value of an arbitrary S-expression. Basically the hash table is
 |  just a hunk of 123 elements each of which points to an overflow list. eg:
 |
 |             shash(1) & shash(nil)
 |                      v
 |  hunk --> [ | | | | |*| | | | | | | ...................]
 |                      |
 |                      [*| ]-->[*| ]--[ |0]
 |                       |       |
 |                       1      nil
 */

/*
 | Hunk hash table functions, create a new one, free an old one and test for
 | absense of the element from the symbol table. A special bit in the hunk
 | allows us to make a symtab use 'eq' or 'equal' for collision resolution.
 | The compiler sets this bit the '1' for literal set maintenance but by
 | default the original mode using 'equal' is used.
 */
#define st_size         123                                          /* hash table size max hunk */
#define st_new()        inserthunk(st_size)                          /* create a new set hunk */
#define st_free(h)      removehunk(h)
#define s_equal(h,a,b) (HUNK(h)->symtbeq ? eq((a),(b)) : equal((a),(b)))
				
/*
 | Add 'e' = (k,d) to the symtab 'h' in bucket 'i'. If absent return 1 else return 0 to
 | indicate that a clash occured.
 */
static int st_add_at(h,i,e)
       struct hunkcell *h;
       struct conscell *e;
       int i;
{
       struct conscell **bp,*l;
       if (e == NULL) return(0);                                /* nil always present */
       bp = GetHunkIndex(h, i);                                 /* ptr to ptr to head of collisioin chain */
       for(l = *bp; l != NULL; l = l->cdrp) {                   /* scan collision chain */
	   if (l->celltype != CONSCELL) goto clash;
	   if (l->carp == NULL) goto clash;
	   if (l->carp->celltype != CONSCELL) goto clash;
	   if (s_equal(h, l->carp->carp, e->carp))              /* look for match */
  clash:       return(0);                                       /* found, report not absent */
       }
       l = new(CONSCELL);                                       /* not found so */
       l->cdrp = (*bp);                                         /* add to head of */
       l->carp = e;
       *bp = l;                                                 /* collision chain */
       return(1);                                               /* and report absent */
}

/*
 | Add 'e'=(k.d) to the symtab 'h'. Hash on k then insert into the overflow chain
 | for this overflow bucket. Returning 1 indicates addition ok, 0 indicates a clash.
 */
#define st_add(h, e) ( st_add_at(h, liushash(e->carp) % st_size, e))

/*
 | Check for ('k',d) in the symtab 'h' at bucket 'i'. If present return (k e) else NULL.
 */
static struct conscell *st_test_at(h,i,k)
       struct hunkcell *h;
       struct conscell *k;
       int i;
{
       struct conscell **bp,*l;
       bp = GetHunkIndex(h, i);                                 /* ptr to ptr to head of collisioin chain */
       for(l = *bp; l != NULL; l = l->cdrp) {                   /* scan collision chain */
	   if (l->celltype != CONSCELL) goto nf;                /* must be an overflow list */
	   if (l->carp == NULL) goto nf;
	   if (l->carp->celltype != CONSCELL) goto nf;
	   if (s_equal(h, l->carp->carp, k))                    /* look for match */
	       return(l->carp);                                 /* found, report is present */
       }
 nf:   return(NULL);                                            /* not found, report absent */
}

/*
 | Check for presense of (k.e) in the hashed hunk symbol table 'h'. If found we
 | return (k.e) else NULL.
 */
#define st_test(h, k) (st_test_at(h, liushash(k) % st_size, k))

/*
 | (symtab-create ((k.e)....) [t]) -> symtab
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | Given a list of (k.e) symbol table entries will create a hashed symbol table
 | of the data.
 */
struct hunkcell *busymtcreate(form)
       struct conscell *form;
{
       struct conscell *l;
       struct hunkcell *h;
       if (form == NULL) goto er;
       l = form->carp;
       push(h);                                                    /* push output list and st for GC */
       h = st_new();
       h->symtbeq = 0;                                             /* use 'equal' by default */
       if ((form = form->cdrp) != NULL) {                          /* unless optional parameter is provided */
           h->symtbeq = (form->carp != NULL) ? 1 : 0;              /* whose value is non NIL in which case use */
           if (form->cdrp != NULL) goto er;                        /* the 'eq' function for collision detection */
       }
       for( ; l != NULL; l = l->cdrp) {                            /* walk through input list */
	   if (l->celltype != CONSCELL) goto er;                   /* no 'dotted pairs' */
	   if ((l->carp == NULL)||(l->carp->celltype != CONSCELL)) /* entry must be a list */
	      goto er;
	   st_add(h, l->carp);                                     /* add element to the set */
       }
       fret(h,1);                                                  /* return built set */
er:    ierror("symtab-create");
}

/*
 | (symtab-list symtab) -> list
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | Given a symtab as an input parameter traverse and create a list of all the elements
 | in the symbol table. We do this by walking through the hunk and building a list of all
 | the elements in each overflow bucket.
 */
struct conscell *busymtlist(form)
       struct conscell *form;
{
       struct conscell *out,*n,*o;                              /* output list */
       struct hunkcell *h; int i;                               /* input symtab hunk */
       if ((form == NULL) || (form->cdrp != NULL)) goto er;     /* if arg error throw err */
       h = HUNK(form->carp);                                    /* get the input list 'in' */
       if (h == NULL) return(NULL);                             /* (symtab-list nil) -> nil */
       if (h->celltype != HUNKATOM) goto er;
       push(out);                                               /* push output list and st for GC */
       for(i=0; i < st_size; i++) {                             /* for each bucket */
	   for(o = *GetHunkIndex(h,i);o!= NULL;o=o->cdrp) {     /* for each overflow */
	       if (o->celltype != CONSCELL) goto er;            /* must be overflow list */
	       n = new(CONSCELL);                               /* tack to front of out */
	       n->cdrp = out;                                   /* list */
	       n->carp = o->carp;
	       out = n;
	   }
       }
       fret(out,1);                                             /* return built set */
er:    ierror("symtab-list");
}

/*
 | (symtab-size symtab) -> N
 | ~~~~~~~~~~~~~~~~~~~~~~~~~
 | Given a symtab as an input parameter traverse and count the number of elements in it
 | and then return this count.
 */
struct conscell *busymtsize(form)
       struct conscell *form;
{
       struct conscell *n,*o;                                   /* output list */
       struct hunkcell *h; int i;                               /* input symtab hunk */
       long int count = 0L;
       if ((form == NULL) || (form->cdrp != NULL)) goto er;     /* if arg error throw err */
       h = HUNK(form->carp);                                    /* get the input list 'in' */
       if (h == NULL) return(newintop(count));                  /* (symtab-size nil) -> 0 */
       if (h->celltype != HUNKATOM) goto er;
       for(i=0; i < st_size; i++) {                             /* for each bucket */
	   for(o = *GetHunkIndex(h,i);o!= NULL;o=o->cdrp) {     /* for each overflow */
	       if (o->celltype != CONSCELL) goto er;            /* must be overflow list */
	       count += 1;                                      /* add 1 to running tally */
	   }
       }
       return(newintop(count));                                 /* return tally */
er:    ierror("symtab-size");
}

/*
 | (symtab-member ST k) -> (k.e) | NIL
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | Given a symbol table ST check for the presence of entry (k.*) in it and if found
 | return (k.e) otherwise return NULL.
 */
struct conscell *busymtmember(form)
       struct conscell *form;
{
       struct conscell *k;
       struct hunkcell *h;
       if (form != NULL) {                             /* if empty arg list throw err */
	   h = HUNK(form->carp);                       /* extract first arg as hunk H */
	   form = form->cdrp;                          /* wind in arg list */
	   if (form != NULL) {                         /* if no more args throw err */
	       k = form->carp;                         /* extract 2nd arg as key to find */
	       if (form->cdrp == NULL) {               /* if > 2 args throw err */
		   if (h == NULL) return(NULL);        /* if empty ST return NIL */
                   if (h->celltype != HUNKATOM) goto er;
		   return(st_test(h, k));              /* otherwise look it up in hunk */
	       }
	   }
       }
er:    ierror("symtab-member");
}

/*
 | (symtab-add ST k e) -> ST | nil
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | Given a symbol table ST a key 'k' and entry data 'e' add the entry (k.e) to
 | the table destructively and return the symbol table or nil of the add caused a
 | clash.
 */
struct hunkcell *busymtadd(form)
       struct conscell *form;
{
       struct conscell *k;
       struct conscell *e;
       struct hunkcell *h;
       struct conscell *pair;
       if (form != NULL) {                                 /* if empty arg list throw err */
	   h = HUNK(form->carp);                           /* extract first arg as hunk H */
	   form = form->cdrp;                              /* wind in arg list */
	   if (form != NULL) {                             /* if no more args throw err */
	       k = form->carp;                             /* extract 2nd arg as key to find */
	       form = form->cdrp;                          /* wind in arg list */
	       if (form != NULL) {                         /* if no more args throw err */
		   e = form->carp;
		   if (form->cdrp == NULL) {               /* if > 3 args throw err */
		       if (h == NULL) return(NULL);        /* if empty ST return NIL */
                       if (h->celltype != HUNKATOM) goto er;
		       push(pair);                         /* push (k.e) pair for GC marking */
		       pair = new(CONSCELL);               /* allocate () empty list */
		       pair->carp = k;                     /* make (k) */
		       pair->cdrp = e;                     /* make it (k.e) */
		       if (st_add(h,pair) == 0)            /* add (k.e) to H, if ok ret H else NULL */
			   h = NULL;
		       fret(h,1);
		   }
	       }
	   }
       }
er:    ierror("symtab-add");
}

/*
 | (symtab-remove ST k) -> (k.e) | NIL
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | Given a symbol table ST check for the presence of entry (k.*) in it and if found
 | return (k.e) and destructively remove (k.e) from the table, otherwise if not found
 | in the table then return NULL.
 */
struct conscell *busymtremove(form)
       struct conscell *form;
{
       struct conscell *k;
       struct hunkcell *h;
       struct conscell **bp,*l;
       if (form != NULL) {                                      /* if empty arg list throw err */
	   h = HUNK(form->carp);                                /* extract first arg as hunk H */
	   form = form->cdrp;                                   /* wind in arg list */
	   if (form != NULL) {                                  /* if no more args throw err */
	       k = form->carp;                                  /* extract 2nd arg as key to find */
	       if (form->cdrp == NULL) {                        /* if > 2 args throw err */
		   if (h == NULL) return(NULL);                 /* if empty ST return NIL */
                   if (h->celltype != HUNKATOM) goto er;
		   bp = GetHunkIndex(h, liushash(k) % st_size); /* ptr to ptr to head of collisioin chain */
		   for(; *bp != NULL; bp = &(l->cdrp)) {        /* scan collision chain */
		       if ((l = *bp)->celltype != CONSCELL) goto er;
		       if (l->carp == NULL) goto er;
		       if (l->carp->celltype != CONSCELL) goto er;
		       if (s_equal(h, l->carp->carp, k)) {      /* look for match */
			   (*bp) = l->cdrp;                     /* bypass it in overflow (ie unlink) */
			   return(l->carp);                     /* return the (k.e) entry we just removed */
		       }
		   }
		   return(NULL);                                /* entry not found, return NULL */
	       }
	   }
       }
er:    ierror("symtab-remove");
}
