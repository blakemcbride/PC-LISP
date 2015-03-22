/* EDITION AC03, APFUN PAS.821 (92/04/23 15:06:42) -- CLOSED */                 
/* --- */

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bugo: is the (go) function which is called from within a prog. The  **
 ** single parameter should be an atom whose binding is the location we **
 ** are to jump to. bugo does not evaluate its arguments automatically  **
 ** rather we must get the valstack and construct the return expression **
 ** of the form (go . (eval arg)) ourselves. We also check the lexical  **
 ** level at which the label was bound and make sure it matches the     **
 ** current lexical level. If not this is a jump out of a prog body.    **
 ** Once constructed the (go . <location>) structure is returned and if **
 ** prog ever sees one it will jump to the given location.              **
 *************************************************************************/
struct conscell *bugo(form)
struct conscell *form;
{      register struct conscell *f;
       if ((form == NULL)||(form->cdrp != NULL)) ierror("go/wrong # of args");
       form = form->carp;
       if ((form == NULL)||(form->celltype != ALPHAATOM)) ierror("go/arg not atom");
       form = ALPHA(form)->valstack;
       if (form == NULL) ierror("go/label not found");
       if (form->linenum != lillev) ierror("go/label out of scope");
       f = new(CONSCELL);
       f->carp = (struct conscell *) gohold;
       f->cdrp = form->carp;
       return(f);
}
