

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"


/*************************************************************************
 ** butype: Given a parameter return its type. 'list, 'symbol, 'flonum  **
 ** 'port, or 'other if we do not know what it is. Note that the null   **
 ** list is of type list. Note symbolp when tested by (type).           **
 *************************************************************************/
struct conscell *butype(form)
struct conscell *form;
{      struct alphacell *r;
       if ((form != NULL)&&(form->cdrp == NULL))
       {   if (form->carp == NULL)                      /* empty list */
               return(LIST(CreateInternedAtom("list")));
           switch(form->carp->celltype)
           {   case CONSCELL  : r = CreateInternedAtom("list");
                                break;
               case ALPHAATOM : r = CreateInternedAtom("symbol");
                                break;
               case STRINGATOM: r = CreateInternedAtom("string");
                                break;
               case FIXATOM   : r = CreateInternedAtom("fixnum");
                                break;
               case FIXFIXATOM: r = CreateInternedAtom("fixfix");
                                break;
               case REALATOM  : r = CreateInternedAtom("flonum");
                                break;
               case FILECELL  : r = CreateInternedAtom("port");
                                break;
               case HUNKATOM  : r = CreateInternedAtom("hunk");
                                break;
               case ARRAYATOM : r = CreateInternedAtom("array");
                                break;
               case CLISPCELL : r = CreateInternedAtom("clisp");
                                break;
               default        : r = CreateInternedAtom("other");
                                break;
           };
           return(LIST(r));
       };
       ierror("type");
}
