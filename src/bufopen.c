

/*
 | PC-LISP (C) 1984-1989 Peter J.Ashwood-Smith
 */
#include <stdio.h>
#include <math.h>
#include "lisp.h"

/*************************************************************************
 ** bufopen: Open file with mode returns a new opened filecell. Note    **
 ** that since a string or atom is allowed for either parameter we may  **
 ** have to make an atom before linking into the file cell fname field. **
 ** Note that the insert/new order avoids having to push the fcell.     **
 ** After the file is successfully opened, we call routine buresetlog   **
 ** with parameter fd and 1 to tell it we just opened 'fd'. It will use **
 ** this information if ever a (resetio) is done by the user. bufclose  **
 ** will call buresetlog with fd and 0 to indicate a close.             **
 **               ************************************                  **
 ** NOTE that CreateInterned and MakePort allocate space hence strings  **
 ** may be relocated. Because of this the name must be copied to a temp **
 ** buffer first. This is true of all calls to GetString so be carefull,**
 ** this took me several days to track down with a very spurious error. **
 *************************************************************************/
struct conscell *bufopen(form)
struct conscell *form;
{      char *fname, *fmode; FILE *fd,*fopen();
       char name[MAXATOMSIZE + 1];
       if ((form != NULL)&&(GetString(form->carp,&fname))) {
           form = form->cdrp;
           strcpy(name, fname);                          /* 'fname' could get relocated! so copy it */
           if ((form!=NULL)&&(GetString(form->carp,&fmode))&&(form->cdrp==NULL)) {
               if ((fd = fopen(fname,fmode)) == NULL) {
                   errno = 0;                            /* don't want apply to catch it */
                   return(NULL);
               }
               buresetlog(fd,1);
               return(LIST(MakePort(fd,CreateInternedAtom(name))));
           }
       }
       ierror("fileopen");  /*  doesn't return  */
       return NULL;   /*  keep compiler happy  */
}
