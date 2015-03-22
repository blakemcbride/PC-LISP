/* EDITION AB01, APFUN MR.68 (90/04/18 09:23:32) -- CLOSED */                   
#include        <stdio.h>
#include        <math.h>
#include        "lisp.h"

#if    GRAPHICSAVAILABLE
/*************************************************************************
 ** Following are routines that are specific to graphics. They will vary**
 ** from machine to machine. This set up is for calls to the BIOS of an **
 ** MSDOS machine and uses base routines for moving/setting the cursor  **
 ** and plotting bits on the screen. These routines are located in the  **
 ** extra.asm module. The line drawing is done in C using a routine from**
 ** Dr. Dobbs.                                                          **
 *************************************************************************/
 static int CurrentMode = 2;
 extern     scrmde();          /* graphics functions  set mode */
 extern     scrspt();          /* set color palatte */
 extern     scrscp();          /* set cursor position */
 extern     scrsct();          /* set cursor type */
 extern     scrsap();          /* set active page */
 extern     scrwdot();         /* write dot */
 extern     scrline();         /* draw a line on screen */

/*************************************************************************
 ** videoreset() : We are being asked to reset the video mode to a sane **
 ** setting. We check that we are not already in mode 2 (80x25 B&W) and **
 ** not we set the mode, otherwise we ignore the request.               **
 *************************************************************************/
 resetvideo()
 {     if (CurrentMode != 2)
            scrmde(2);
 }

/*************************************************************************
 ** buscrmde(m) : Will call BIOS routine to set the CRT mode to 'm' we  **
 ** call the function _scrmde(m) which is located in supta.asm module   **
 ** We also log the mode change so that when the resetvideo() call is   **
 ** made we know if a call to _scrmde() is really necessary. We do not  **
 ** want to call scrmde() unnecessarily because it clears the screen.   **
 *************************************************************************/
struct conscell *buscrmde(form)
struct conscell *form;
{      long int mode;
       if ((form!=NULL)&&(GetFix(form->carp,&mode))&&(form->cdrp == NULL))
       {    if (mode >= 0L)
            {    CurrentMode = mode;
                 scrmde((int)mode);
                 return(LIST(thold));
            };
       };
       ierror("#scrmde#");
}

/*************************************************************************
 ** buscrsap(p) : Will call BIOS routine to set the CRT active page to  **
 ** 'p'. The function _scrsap(p) which is located in supta.asm module   **
 *************************************************************************/
struct conscell *buscrsap(form)
struct conscell *form;
{      long int page;
       if ((form!=NULL)&&(GetFix(form->carp,&page))&&(form->cdrp == NULL))
       {  scrsap((int) page);
          return(LIST(thold));
       };
       ierror("#scrsap#");
}

/*************************************************************************
 ** buscrspt(bh,bl,al) Will call BIOS service routine to set the color  **
 ** pallette according to the parameters bh,bl,al.                      **
 *************************************************************************/
struct conscell *buscrspt(form)
struct conscell *form;
{      long int a,b,c;
       if ((form!=NULL)&&(GetFix(form->carp,&a)))
       {    form = form->cdrp;
            if ((form!=NULL)&&(GetFix(form->carp,&b)))
            {   form = form->cdrp;
                if ((form!=NULL)&&(GetFix(form->carp,&c))&&(form->cdrp==NULL))
                {   scrspt((int)a,(int)b,(int)c);
                    return(LIST(thold));
                };
            };
       };
       ierror("#scrspt#");
}

/*************************************************************************
 ** buscrsct(a,b): Will call BIOS routine to set the CRT cursor type to **
 ** be 'a'-'b'. This is defined in the BIOS manual. From-to blink etc.  **
 *************************************************************************/
struct conscell *buscrsct(form)
struct conscell *form;
{      long int a,b;
       if ((form!=NULL)&&(GetFix(form->carp,&a)))
       {    form = form->cdrp;
            if ((form!=NULL)&&(GetFix(form->carp,&b))&&(form->cdrp==NULL))
            {   scrsct((int)a,(int)b);
                return(LIST(thold));
            };
       };
       ierror("#scrsct#");
}

/*************************************************************************
 ** buscrscp(p,r,c) Will call BIOS routine to set the CRT cursor posit- **
 ** ion to (r,c) on page 'p'. We call _scrscp() in supta.asm to do this.**
 *************************************************************************/
struct conscell *buscrscp(form)
struct conscell *form;
{      long int p,r,c;
       if ((form!=NULL)&&(GetFix(form->carp,&p)))
       {    form = form->cdrp;
            if ((form!=NULL)&&(GetFix(form->carp,&r)))
            {   form = form->cdrp;
                if ((form!=NULL)&&(GetFix(form->carp,&c))&&(form->cdrp==NULL))
                {  scrscp((int)p,(int)r,(int)c);
                   return(LIST(thold));
                };
            };
       };
       ierror("#scrscp#");
}

/*************************************************************************
 ** buscrwdot(r,c,a) Will call BIOS routine to write a dot at location  **
 ** (r,c) using attribute 'a'.We call _scrwdot() in supta.asm to do this**
 *************************************************************************/
struct conscell *buscrwdot(form)
struct conscell *form;
{      long int r,c,a;
       if ((form!=NULL)&&(GetFix(form->carp,&r)))
       {    form = form->cdrp;
            if ((form!=NULL)&&(GetFix(form->carp,&c)))
            {   form = form->cdrp;
                if ((form!=NULL)&&(GetFix(form->carp,&a))&&(form->cdrp==NULL))
                {  scrwdot((int)r,(int)c,(int)a);
                   return(LIST(thold));
                };
            };
       };
       ierror("#scrwdot#");
}

/*************************************************************************
 ** buscrline(x1,y1,x2,y2,a) Will draw a line on the screen from (x1,y1)**
 ** to (x2,y2) using color attribute 'a'. We call the routine DrawLine()**
 ** to do the actual dot drawing for us. This is incredibly slow but it **
 ** is portable and will work under topview etc.... so here we are.     **
 *************************************************************************/
struct conscell *buscrline(form)
struct conscell *form;
{      long int x1,y1,x2,y2,attr;
       if ((form!=NULL)&&(GetFix(form->carp,&x1)))
       {    form = form->cdrp;
            if ((form!=NULL)&&(GetFix(form->carp,&y1)))
            {   form = form->cdrp;
                if ((form!=NULL)&&(GetFix(form->carp,&x2)))
                {   form = form->cdrp;
                    if ((form!=NULL)&&(GetFix(form->carp,&y2)))
                    {   form = form->cdrp;
                        if ((form!=NULL)&&(GetFix(form->carp,&attr))&&(form->cdrp==NULL))
                        {   DrawLine((int)x1,(int)y1,(int)x2,(int)y2,(int)attr);
                            return(LIST(thold));
                        };
                    };
                };
            };
       };
       ierror("#scrline#");
}

/*************************************************************************
 ** DrawLine(x1,y1,x2,y2,a)  Will draw a line on the screen from the    **
 ** point (x1,y1) to (x2,y2) using attribute a. From Dr. Dobbs May/86   **
 *************************************************************************/
DrawLine(x1,y1,x2,y2,a)
int x1,y1,x2,y2,a;
{   register int x,y,d,incr1,incr2,incr3,dx,dy,xend,yend;
    dx = x2 > x1 ? x2-x1 : x1-x2;
    dy = y2 > y1 ? y2-y1 : y1-y2;
    if (dy <= dx)
    {   if (x1>x2)
        { x = x2;
          y = y2;
          xend = x1;
          dy=y1-y2;
        }
        else
        { x = x1;
          y = y1;
          xend = x2;
          dy=y2-y1;
        };
        d = (dy<<1)-dx;
        incr1 = dy<<1;
        incr2 = (dy-dx)<<1;
        incr3 = (dy+dx)<<1;
        scrwdot(x,y,a);
        while(x < xend)
        {   x ++;
            if (d >= 0)
            {   if (dy <= 0)
                   d+=incr1;
                else
                { y+=1;
                  d+=incr2;
                };
            }
            else
            {   if (dy>=0)
                   d += incr1;
                else
                {  y -= 1;
                   d += incr3;
                };
            };
            scrwdot(x,y,a);
        };
        return;
    };
    if (y1>y2)
    { y = y2;
      x = x2;
      yend = y1;
      dx=x1-x2;
    }
    else
    { y = y1;
      x = x1;
      yend = y2;
      dx=x2-x1;
    };
    d = (dx<<1)-dy;
    incr1 = dx<<1;
    incr2 = (dx-dy)<<1;
    incr3 = (dx+dy)<<1;
    scrwdot(x,y,a);
    while(y < yend)
    {   y ++;
        if (d >= 0)
        {   if (dx <= 0)
               d+=incr1;
            else
            { x+=1;
              d+=incr2;
            };
        }
        else
        {   if (dx>=0)
               d += incr1;
            else
            {  x -= 1;
               d += incr3;
            };
        };
        scrwdot(x,y,a);
    };
}

/*************************************************************************
 ** End of the MSDOS dependent functions. There is an #if MSDOS above...**
 *************************************************************************/
#endif


