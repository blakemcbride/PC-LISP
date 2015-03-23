#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lisp.h"

/*************************************************************************
 ** (filestat <path>) returns a list containing 8 fixnums which represent*
 ** respecitvely the mode, nlink, uid, gid, size, atime, mtime and ctime**
 ** from the files stat strucuture. If the file cannot be found it will **
 ** return nil.                                                         **
 *************************************************************************/
struct conscell *bufilestat(form)
struct conscell *form;
{      char *path; struct stat s_buf;
       struct conscell *h, *l;
       if ((form != NULL)&&(form->cdrp == NULL)) {
          if (GetString(form->carp, &path)) {
                if (stat(path, &s_buf) != 0) { errno = 0; return(NULL); }

               /*
                | Build the list backwards keeping 'h' ALWAYS pointing to
                | the head of the list so that if GC occurs all cells will
                | be marked.
                */
                push(h);
                h = l = new(CONSCELL);
                FIX(l->carp = new(FIXATOM))->atom = s_buf.st_ctime;
                (l = new(CONSCELL))->cdrp = h;
                h = l;
                FIX(l->carp = new(FIXATOM))->atom = s_buf.st_mtime;
                (l = new(CONSCELL))->cdrp = h;
                h = l;
                FIX(l->carp = new(FIXATOM))->atom = s_buf.st_atime;
                (l = new(CONSCELL))->cdrp = h;
                h = l;
                FIX(l->carp = new(FIXATOM))->atom = s_buf.st_size;
                (l = new(CONSCELL))->cdrp = h;
                h = l;
                FIX(l->carp = new(FIXATOM))->atom = s_buf.st_gid;
                (l = new(CONSCELL))->cdrp = h;
                h = l;
                FIX(l->carp = new(FIXATOM))->atom = s_buf.st_uid;
                (l = new(CONSCELL))->cdrp = h;
                h = l;
                FIX(l->carp = new(FIXATOM))->atom = s_buf.st_nlink;
                (l = new(CONSCELL))->cdrp = h;
                h = l;
                FIX(l->carp = new(FIXATOM))->atom = s_buf.st_mode;
                fret(h,1);
          }
       }
       ierror("filestat");
}

