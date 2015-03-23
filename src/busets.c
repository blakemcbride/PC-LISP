/* */
/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include "lisp.h"

/*
 |  BUSETS
 |  ~~~~~~
 |  This module handles the set primitives. These are functions that treat a
 |  a list as a set of elements. In otherwords they do not honour the order of
 |  the list and will treat a single occurrence of an element as equivalent to
 |  multiple occurrences of the same element in the set. The sets returned do
 |  not have an element in them more than once and nil is explicitly removed
 |  from all returned sets. These functions will handle reasonably large sets
 |  effeciently if the set-create operation is used to create hashed sets
 |  rather than using the list form everywhere. The list form is however more
 |  effecient for small 'literal' sets being or-ed or and-ed with a larger set.
 |
 |         (set-create list)                  -> S
 |         (set-list S|list)                  -> list
 |         (set-and s1|l1 s2|l2 s3|l3 ... )   -> s1^s2^s3 ...
 |         (set-or s1|l1 s2|l2 s3|l3 ....)    -> s2 U s2 U s3 ...
 |         (set-diff s1|l1 s2|l2 s3|l3 ....)  -> ((s1 - s2) - s3) ..
 |         (set-member S e)                   -> e member of S ? t : nil
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
 | absense of the element from the symbol table.
 */
#define st_size    123                                          /* hash table size max hunk */
#define st_new()   inserthunk(st_size)                          /* create a new set hunk */

/*
 | Add 'e' to the set 'h' in bucket 'i'. If absent return 1 else return 0.
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
           if (equal(l->carp, e))                               /* look for match */
               return(0);                                       /* found, report not absent */
       }
       l = new(CONSCELL);                                       /* not found so */
       l->cdrp = (*bp);                                         /* add to head of */
       l->carp = e;
       *bp = l;                                                 /* collision chain */
       return(1);                                               /* and report absent */
}

/*
 | Add 'e' to the set 'h'. If absent return 1 else return 0. 'nil' not added.
 | Compute hash index of the element and call st_add_at.
 */
static int st_add(h,e)
       struct hunkcell *h;
       struct conscell *e;
{
       if (e == NULL) return(0);                                /* nil not added */
       return(st_add_at(h, liushash(e) % st_size, e));           /* st_add_at does work */
}

/*
 | Check for 'e' in the set 'h' at bucket 'i'. If present return 1 else return 0.
 */
static int st_test_at(h,i,e)
       struct hunkcell *h;
       struct conscell *e;
       int i;
{
       struct conscell **bp,*l;
       if (e == NULL) return(1);                                /* nil always present */
       bp = GetHunkIndex(h, i);                                 /* ptr to ptr to head of collisioin chain */
       for(l = *bp; l != NULL; l = l->cdrp) {                   /* scan collision chain */
           if (equal(l->carp, e))                               /* look for match */
               return(1);                                       /* found, report is present */
       }
       return(0);                                               /* not found, report absent */
}

/*
 | Check for presense of 'e' in the hashed hunk set 'h'.
 | Compute hash index of the element and call st_test_at.
 */
static int st_test(h,e)
       struct hunkcell *h;
       struct conscell *e;
{
       if (e == NULL) return(1);                                /* nil always in a set */
       return(st_test_at(h, liushash(e) % st_size, e));         /* st_test_at does work */
}

/*
 | Make a set given that 'l' is the input set of elements to put in the set.
 | If the set ends up being empty then return nil.
 */
static struct hunkcell *st_make(l)
       struct conscell *l;
{
       struct hunkcell *h; int siz;
       if (l == NULL) return(NULL);                             /* (enset nil) -> nil */
       push(h);                                                 /* push output list and st for GC */
       h = st_new();
       for(siz = 0; l != NULL; l = l->cdrp) {                   /* walk through input list */
           if (l->celltype != CONSCELL) goto er;                /* no 'dotted pairs' */
           siz += st_add(h,l->carp);                            /* add element to the set */
       }
       if (siz == 0) h = NULL;                                  /* if empty set return nil */
       fret(h,1);                                               /* return built set */
er:    ierror("set-create");
}

/*
 | (set-create list) -> set
 | ~~~~~~~~~~~~~~~~~~~~~~~~
 | Given a list as an input parameter add all elements in the list to a new
 | set and return the new set. If no set is created we return NULL.
 */
struct hunkcell *busetcreate(form)
       struct conscell *form;
{
       if ((form == NULL)||(form->cdrp != NULL)) goto er;
       return(st_make(form->carp));
er:    ierror("set-create");
}

/*
 | (set-list set|list) -> list
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | Given a set as an input parameter traverse and create a list of all the elements
 | in the set. We do this by walking through the hunk and building a list of all
 | the elements in each overflow bucket.  WATCH OUT FOR GARBAGE COLLECTION IN THE
 | RECURSIVE CALL TO OURSELVES!!!
 */
struct conscell *busetlist(form)
       struct conscell *form;
{
       struct conscell *out,*n,*o;                              /* output list */
       struct hunkcell *h; int i;                               /* input set hunk */
       if ((form == NULL)||(form->cdrp != NULL)) goto er;       /* if arg error throw err */
       h = HUNK(form->carp);                                    /* get the input list 'in' */
       if (h == NULL) return(NULL);                             /* (set-list nil) -> nil */
       if (h->celltype == CONSCELL) {                           /* if already a list turn into set */
          h = busetcreate(form);                                /* before listing it out */
          if (h == NULL) return(NULL);
       }
       if (h->celltype != HUNKATOM) goto er;
       push(out); xpush(h);                                     /* push input & output list for GC!! */
       for(i=0; i < st_size; i++) {                             /* for each bucket */
           for(o = *GetHunkIndex(h,i);o!= NULL;o=o->cdrp) {     /* for each overflow */
               n = new(CONSCELL);                               /* tack to front of out */
               n->cdrp = out;                                   /* list */
               n->carp = o->carp;
               out = n;
           }
       }
       fret(out,2);                                             /* return built set */
er:    ierror("set-list");
}

/*
 | (set-or s1|l1 s2|l2 s3|l3  ...) -> set
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | Given sets as input parameters, build a set which contains the union
 | of all of the input lists or sets.
 */
struct hunkcell *busetor(form)
       struct conscell *form;
{
       struct conscell *in,*o;                                  /* input list & hunk overflow */
       struct hunkcell *h;                                      /* and pointer to build set */
       int i, siz;                                              /* size of built set 'h' */
       if (form == NULL) return(NULL);
       push(h);
       h = st_new();
       for(siz = 0; form != NULL; form = form->cdrp) {
          in = form->carp;                                      /* get the set or list object */
          if (in == NULL) continue;                             /* nil's have no effect */
          if (in->celltype == CONSCELL) {                       /* if set to be or'ed is a list */
             for(; in != NULL; in = in->cdrp) {                 /* walk through input list */
                if (in->celltype != CONSCELL) goto er;          /* no 'dotted pairs' */
                siz += st_add(h, in->carp);                     /* add element to the output set */
             }
          } else {                                              /* input set not list */
             if (in->celltype != HUNKATOM) goto er;             /* so must be a hunk */
             for(i=0; i < st_size; i++) {                       /* for each bucket */
                for(o = *GetHunkIndex(in,i);o!= NULL;o=o->cdrp) /* for each overflow */
                   siz += st_add_at(h, i, o->carp);             /* add to output set */
             }
          }
       }
       if (siz == 0) h = NULL;                                  /* if empty set return nil */
       fret(h,1);                                               /* return built set */
er:    ierror("set-or");
}

/*
 | (set-and s1|l1 s2|l2 ...) -> set
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | Given sets or lists as input parameters, build a set which contains the intersection
 | of all of the input lists or sets. If nil is ever present in the list of input
 | sets we return nil immediately.
 */
struct hunkcell *busetand(form)
       struct conscell *form;
{
       struct conscell *f,*o,*s;
       struct hunkcell *h,*r;
       int              i, pres, siz;

      /*
       | Sanity: if no args or a nil in args (set-and) returns 'nil. Also make
       | sure all the argument list is really a list.
       */
       if (form == NULL) return(NULL);
       for(f = form; f != NULL; f = f->cdrp) {
           if (f->celltype != CONSCELL) goto er;
           if (f->carp == NULL) return(NULL);
           if ((f->carp->celltype != HUNKATOM) &&
               (f->carp->celltype != CONSCELL)) goto er;
       }

      /*
       | Get the first argument and we will use it as the pool from which to
       | get all elements to test against the other arguments. If the first
       | argument is a list we get an equivalent set for it to begin with.
       */
       push(h);
       h = HUNK(form->carp);                                    /* h is set we will scan */
       form = form->cdrp;                                       /* advance to next argument */
       if (h->celltype == CONSCELL)                             /* if scan set not a set (ie is a list) */
           h = st_make(h);                                      /* make a temporary set to work with */
       if ((form == NULL)||(h == NULL)) fret(h,1);              /* if no more arguments or empty just return it */

      /*
       | We have a true scan set and there is more than one argument so we must
       | for every element in the scan set test for its presence in all the other
       | argument sets and if it is present in all we add it to the output set
       */
       push(r);                                                      /* mark the output hunk */
       r = st_new();                                                 /* create the output hunk */
       for(siz = i = 0; i < st_size; i++) {                          /* for each bucket in scan set */
           for(o = *GetHunkIndex(h,i); o != NULL; o=o->cdrp) {       /* for each overflow in this bucket */
               pres = 1;
               for(f = form; f != NULL && pres; f = f->cdrp) {       /* for each remaining argument */
                   if (f->carp->celltype == HUNKATOM)                /* if arg is a true set */
                       pres = st_test_at(f->carp, i, o->carp);       /* look only for element in bucket 'i' */
                   else {                                            /* else arg is a set, must scan */
                       pres = 0;                                     /* scan entire list for o->carp */
                       for(s=f->carp; s!=NULL && !pres; s=s->cdrp) {
                          if (s->celltype != CONSCELL) goto er;
                          pres = equal(s->carp, o->carp);            /* it is present if 'equal' */
                       }
                   }
               }
               if (pres) siz += st_add_at(r, i, o->carp);
           }
       }
       if (siz == 0) r = NULL;
       fret(r,2);
er:    ierror("set-and");
}

/*
 | (set-diff s1|l1 s2|l2 ...) -> set
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | Given sets or lists as input parameters, build a set which contains the value
 | of "((((s1 - s2) - s3) - s4) .....". This calculation is based on the fact that
 | ((s1 - s2) - s3) == s1 - (s2 U s3) so we proceed by taking the grand union of
 | s2... sN and then performing the difference of this set with s1.
 */
struct hunkcell *busetdiff(form)
       struct conscell *form;
{
       struct conscell *f,*o;
       struct hunkcell *r,*h,*d;
       int i, siz;

      /*
       | Sanity check the arguments. If first arg is NULL return NULL, if any other arg
       | is NULL return the first argument.
       */
       if (form == NULL) return(NULL);
       for(f = form; f != NULL; f = f->cdrp) {
           if (f->celltype != CONSCELL) goto er;
           if (f->carp == NULL) return(HUNK(form->carp));
           if ((f->carp->celltype != HUNKATOM) &&
               (f->carp->celltype != CONSCELL)) goto er;
       }

      /*
       | Get the first argument and we will use it as the pool from which to
       | get all elements to test against the other arguments. If the first
       | argument is a list we get an equivalent set for it to begin with.
       */
       push(r);
       r = HUNK(form->carp);                                    /* h is set we will scan */
       form = form->cdrp;                                       /* advance to next argument */
       if (r->celltype == CONSCELL)                             /* if scan set not a set (ie is a list) */
           r = st_make(r);                                      /* make a temporary set to work with */
       if (form == NULL) fret(r,1);                             /* if no more arguments just return scan set */

      /*
       | Get the grand union of the sets s2 ... sN and call it 'h', this is just
       | a temporary working set which we will actually diff with 1st set. If the
       | grand union of s2 ... sN is nil then the set difference is just s1 so we
       | can return s1 immediately.
       */
       push(h);
       h = busetor(form);
       if (h == NULL) fret(r,2);

      /*
       | Compute d = r - h where r = s1 and h = s2 U s3 U ... sN hence we are
       | computing s1 - (s2 U s3 U s4 ...) or (s1 - s2) - s3 ... We also track
       | the output size so that we can return NIL if nothing ever gets added
       | to the output set 'd'.
       */
       push(d);
       d = st_new();
       for(siz = i = 0; i < st_size; i++) {
           for(o = *GetHunkIndex(r, i); o != NULL; o = o->cdrp) {
               if (!st_test_at(h, i, o->carp))
                   siz += st_add_at(d, i, o->carp);
           }
       }

      /*
       | All done, if nothing in output set 'd' just free it up and return NULL
       | Also free up the working grand union set 'h' as we no longer need it.
       | Finally return the computed result set 'r' (or NULL if empty).
       */
       if (siz == 0) d = NULL;
       fret(d,3);
er:    ierror("set-diff");
}

/*
 | (set-member s1|l1 e) -> t | nil
 | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 | Given a set or a list return true or nil depending on if e is a member of the
 | the set s1 or the list l1.
 */
struct conscell *busetmember(form)
       struct conscell *form;
{
       struct conscell *s, *e;

      /*
       | Extract the set 's' and the element being tested 'e'.
       */
       if (form == NULL) goto er;
       s = form->carp;
       form = form->cdrp;
       if (form == NULL) goto er;
       e = form->carp;
       if (form->cdrp != NULL) goto er;

      /*
       | Trivial cases "(set-member s nil) -> t" and "(set-member nil e) -> nil"
       */
       if (e == NULL) return(LIST(thold));
       if (s == NULL) return(NULL);

      /*
       | First real set case, the set is a hashed hunk set so use the st_test
       | routine to do the dirty work.
       */
       if (s->celltype == HUNKATOM) {
           if (st_test(s,e)) return(LIST(thold));
           return(NULL);
       }

      /*
       | Simple list case, the set is just a list so walk through it and if
       | found return t else return nil.
       */
       for( ; s != NULL; s = s->cdrp) {
           if (s->celltype != CONSCELL) goto er;
           if (equal(s->carp, e)) return(LIST(thold));
       }
       return(NULL);
er:    ierror("set-member");
}

