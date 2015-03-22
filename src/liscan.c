/* EDITION AD02, APFUN PAS.770 (91/12/12 15:13:22) -- CLOSED */                 

/****************************************************************************
 **             PC-LISP (C) 1986 Peter Ashwood-Smith                       **
 **             MODULE: SCAN                                               **
 **------------------------------------------------------------------------**
 **   The lisp scanner is just a simple function which when called returns **
 ** a value of 0..7. The value corresponds to the type of input token which**
 ** was recognized. Lisp requires that brackets, atoms, real atoms, dots,  **
 ** and square brackets be recognized. The end of file is also considered  **
 ** a token whose value is zero. The call to 'scan' takes one parameter 'r'**
 ** where 'r' is just a pointer to a buffer where the token text may be    **
 ** stored. For example if the next token at the standard input is '-2.4'  **
 ** the string "-2.4\0" will be placed starting at location 'r' and the    **
 ** value 7 will be returned to indicate a real number atom was scanned.   **
 **                                                                        **
 **      Note to facilitate the lisp convention of a square bracket closing**
 ** all open lists there is a procedure which will compute the current     **
 ** nesting level and return the correct number of ')' tokens whenever a   **
 ** `]` token is encountered. In addition the [ character can be used to   **
 ** mark the point at which closing is to stop. These are meta brackets.   **
 **                                                                        **
 **         Token # 0           EOF                                        **
 **         Token # 1           (                                          **
 **         Token # 2           )                                          **
 **         Token # 3           |.....literal atom.....|                   **
 **         Token # 4           ]                                          **
 **         Token # 5           .                                          **
 **         Token # 6           atom                                       **
 **         Token # 7           real                                       **
 **         Token # 8           READ MACRO CHARACTER TRIGGER               **
 **         Token # 9           [                                          **
 **         Token # 10          string   ie " ... stuff ... "              **
 **         Token # 11          SPLICE READ MACRO CHARACTER TRIGGER        **
 **         Token # 12          unknown, char >= 128 seen (and unread)     **
 **                                                                        **
 **     Note that these are the internal token numbers. The only tokens    **
 ** which are not returned are 3 and 4. Token 3 is filtered to be just the **
 ** literal stuff between the bar  symbols. Token 4 becomes a stream of ). **
 ** Token number 3 represents both a literal or a string atom internally   **
 ** both the '|' and '"' characters are valid end delimiters of a literal  **
 ** sequence of characters as far as the DFA is concerned, however when    **
 ** we have the token we will remove the ending delimiters and depending   **
 ** on the delimiter '|' or '"' we return token 6 or token 10 accordingly. **
 *****************************************************************************/

#include        <stdio.h>
#include        "lisp.h"

#define         LPR     0               /* left round bracket */
#define         RPR     1               /* right round bracket*/
#define         HSH     2               /* lit atom delimiter */
#define         SQR     3               /* the ']' character  */
#define         DOT     4               /* The dot or period  */
#define         OTH     5               /* Format characters  */
#define         DIG     6               /* a digit 0..9       */
#define         SGN     7               /* a sign + or -      */
#define         LET     8               /* a letter a..Z etc. */
#define         CUL     9               /* comment delim left = ';' */
#define         CUR     10              /* comment delim right= '\n'*/
#define         EFF     11              /* new EOF char type  */
#define         MC0     12              /* read macro character trigger */
#define         SQL     13              /* the '[' char       */
#define         MC1     14              /* splice read macro character trigger*/
#define         ECP     15              /* escape character is usually '\' */
#define         EEE     16              /* either 'e' or 'E' */

struct asciitypes
{      unsigned type : 8;               /* type class of char */
};

/*****************************************************************************
 ** asciitypes table is a mapping from the character values -1 to 127 ie it **
 ** maps 255 (-1 or EOF) up to the DEL character onto the correct type for  **
 ** for the transition table. Note that we map EOF to -1 to first elem. EOF **
 ** Note also that NUL is mapped to EFF to allow the DFA to run on strings. **
 *****************************************************************************/
static struct asciitypes chartype[129] =
{
  /*EOF*/ {EFF },
  /*NUL*/ {EFF }, /*SOH*/ {OTH }, /*STX*/ {OTH }, /*ETX*/ {OTH },
  /*EOT*/ {EFF }, /*ENQ*/ {OTH }, /*ACQ*/ {OTH }, /*BEL*/ {OTH },
  /*BS */ {OTH }, /*HT */ {OTH }, /*LF */ {CUR }, /*VT */ {OTH },
  /*FF */ {OTH }, /*CR */ {CUR }, /*SO */ {OTH }, /*SI */ {OTH },
  /*DLE*/ {OTH }, /*DC1*/ {OTH }, /*DC2*/ {OTH }, /*DC3*/ {OTH },
  /*DC4*/ {OTH }, /*NAK*/ {OTH }, /*SYN*/ {OTH }, /*ETB*/ {OTH },
  /*CAN*/ {OTH }, /*EM */ {OTH }, /*SUB*/ {OTH }, /*ESC*/ {OTH },
  /*FS */ {OTH }, /*GS */ {OTH }, /*RS */ {OTH }, /*US */ {OTH },
  /*SP */ {OTH }, /*!  */ {LET }, /*"  */ {HSH }, /*#  */ {LET },
  /*$  */ {LET }, /*%  */ {LET }, /*&  */ {LET }, /*'  */ {MC0 },
  /*(  */ {LPR }, /*)  */ {RPR }, /**  */ {LET }, /*+  */ {SGN },
  /*,  */ {OTH }, /*-  */ {SGN }, /*.  */ {DOT }, /*sl */ {LET },
  /*0  */ {DIG }, /*1  */ {DIG }, /*2  */ {DIG }, /*3  */ {DIG },
  /*4  */ {DIG }, /*5  */ {DIG }, /*6  */ {DIG }, /*7  */ {DIG },
  /*8  */ {DIG }, /*9  */ {DIG }, /*:  */ {LET }, /*;  */ {CUL },
  /*<  */ {LET }, /*=  */ {LET }, /*>  */ {LET }, /*?  */ {LET },
  /*at */ {LET }, /*A  */ {LET }, /*B  */ {LET }, /*C  */ {LET },
  /*D  */ {LET }, /*E  */ {EEE }, /*F  */ {LET }, /*G  */ {LET },
  /*H  */ {LET }, /*I  */ {LET }, /*J  */ {LET }, /*K  */ {LET },
  /*L  */ {LET }, /*M  */ {LET }, /*N  */ {LET }, /*O  */ {LET },
  /*P  */ {LET }, /*Q  */ {LET }, /*R  */ {LET }, /*S  */ {LET },
  /*T  */ {LET }, /*U  */ {LET }, /*V  */ {LET }, /*W  */ {LET },
  /*X  */ {LET }, /*Y  */ {LET }, /*Z  */ {LET }, /*[  */ {SQL },
  /*rsl*/ {ECP }, /*]  */ {SQR }, /*^  */ {LET }, /*_  */ {LET },
  /*'  */ {LET }, /*a  */ {LET }, /*b  */ {LET }, /*c  */ {LET },
  /*d  */ {LET }, /*e  */ {EEE }, /*f  */ {LET }, /*g  */ {LET },
  /*h  */ {LET }, /*i  */ {LET }, /*j  */ {LET }, /*k  */ {LET },
  /*l  */ {LET }, /*m  */ {LET }, /*n  */ {LET }, /*o  */ {LET },
  /*p  */ {LET }, /*q  */ {LET }, /*r  */ {LET }, /*s  */ {LET },
  /*t  */ {LET }, /*u  */ {LET }, /*v  */ {LET }, /*w  */ {LET },
  /*x  */ {LET }, /*y  */ {LET }, /*z  */ {LET }, /*{  */ {LET },
  /*|  */ {HSH }, /*}  */ {LET }, /*~  */ {LET }, /*DEL*/ {OTH }
};

/***************************************************************************
 ** Set the scan class of a character to be macro 0 or macro 1. These are **
 ** read macro trigger tokens. Type 0 returns token #8 (MACRO) and type 1 **
 ** returns token #11. These are read macro trigger characters classes and**
 ** cause the scanner to return the character in the buffer immediately   **
 ** after it has been read. This allows the TakeSexpression function in   **
 ** main.c to envoke a read or spliced read macro when necessary.         **
 ***************************************************************************/
ScanSetSynClassMacro(c,kind)
int c,kind;
{   if ((c < 1)||(c > 127))
        fatalerror("ScanSetSynClassMacro");
    chartype[c+1].type = (kind == 0) ? MC0 : MC1;
}

/***************************************************************************
 ** >>[S0]+---(EFF)-------------------------------------> Return Tok 0    **
 **       |                                                               **
 **       +---(()---------------------------------------> Return Tok 1    **
 **       |                                                               **
 **       +---())---------------------------------------> Return Tok 2    **
 **       |                                                               **
 **       +---("or|)----[7]----("or|)-------------------> Return Tok 3    **
 **       |              \                                                **
 **       |               (ECP)-->[8]--(*)-->[7]   {handle \n \| \" etc.} **
 **       |                                                               **
 **       +---(])---------------------------------------> Return Tok 4    **
 **       |                                                               **
 **       +---([)---------------------------------------> Return Tok 13   **
 **       |                                                               **
 **       +---(.)-->[1]---(DIG)--->[2]---(other)--------> Return Tok 7    **
 **       |                   +--->\ /                                    **
 **       |         (DIG)     |   (DIG|EEE|SGN)  {float with E-nn E+nn}   **
 **       |          / \__(EEE|DOT)                                       **
 **       +---(DIG)--[3]--------+-(other)---------------> Return Tok 7    **
 **       |           ^         |                                         **
 **       |           |(DIG^)   +-(SGN|LET)->[5] {1a,1- atoms, not 1e!}   **
 **       |           |                                                   **
 **       +---(SGN)--[4]--------------------------------> Return Tok 6    **
 **       |                                                               **
 **       +---(LET)--[5]----------(other)---------------> Return Tok 6    **
 **       |          |  \                                                 **
 **       |        (ECP) (SGN,DIG,LET,DOT)-->[5]                          **
 **       |           |                                                   **
 **       |           V                                                   **
 **       +---(ECP)---[9]-->(*)-->[5]           {the back slash /}        **
 **       |                                                               **
 **       +---(;)--[6]--(\n)-------[0] etc...    comment skipper          **
 **       |        \ /                                                    **
 **       |      (other)                                                  **
 **       |                                                               **
 **       +------(MC0)----------------------------------> Return Tok 8    **
 **       |                                                               **
 **       +------(MC1)----------------------------------> Return Tok 11   **
 **                                                                       **
 **    -- Not noted but states 1,2,3 & 4 jump to state 9 on receipt of    **
 **       the ECP (escape) character. This forces things like 2.0e\1 to   **
 **       be symbols rather than flonum atoms. (All escapes cause symbol) **
 ***************************************************************************/


#define         R       0               /* action.type = return a token    */
#define         S       1               /* action.type = state transition  */
#define         P       0               /* action.flag = read ahead +      */
#define         M       1               /* action.flag = do not read ahead */

struct action
{      unsigned type   : 1  ;           /* action is Ret tok or Transition */
       unsigned number : 6  ;           /* new state number or token value */
       unsigned flag   : 1  ;           /* read ahead or don't read ahead  */
};

static struct action newaction, transtable[17][10] =
{
/*DEL     state0   state1   state2   state3   state4   state5   state6   state7   state8   state9*/
/*-----------------------------------------------------------------------------------------------*/
/*(  */ {{R, 1,P},{R, 5,M},{R, 7,M},{R, 7,M},{R, 6,M},{R, 6,M},{S, 6,P},{S, 7,P},{S, 7,P},{S, 5,P}},
/*)  */ {{R, 2,P},{R, 5,M},{R, 7,M},{R, 7,M},{R, 6,M},{R, 6,M},{S, 6,P},{S, 7,P},{S, 7,P},{S, 5,P}},
/*"  */ {{S, 7,P},{R, 5,M},{R, 7,M},{R, 7,M},{R, 6,M},{R, 6,M},{S, 6,P},{R, 3,P},{S, 7,P},{S, 5,P}},
/*]  */ {{R, 4,P},{R, 5,M},{R, 7,M},{R, 7,M},{R, 6,M},{R, 6,M},{S, 6,P},{S, 7,P},{S, 7,P},{S, 5,P}},
/*.  */ {{S, 1,P},{R, 5,M},{R, 7,M},{S, 2,P},{R, 6,M},{S, 5,P},{S, 6,P},{S, 7,P},{S, 7,P},{S, 5,P}},
/*OTH*/ {{S, 0,P},{R, 5,M},{R, 7,M},{R, 7,M},{R, 6,M},{R, 6,M},{S, 6,P},{S, 7,P},{S, 7,P},{S, 5,P}},
/*DIG*/ {{S, 3,P},{S, 2,P},{S, 2,P},{S, 3,P},{S, 3,P},{S, 5,P},{S, 6,P},{S, 7,P},{S, 7,P},{S, 5,P}},
/*SGN*/ {{S, 4,P},{R, 5,M},{S, 2,P},{S, 5,P},{R, 6,M},{S, 5,P},{S, 6,P},{S, 7,P},{S, 7,P},{S, 5,P}},
/*LET*/ {{S, 5,P},{R, 5,M},{R, 7,M},{S, 5,P},{R, 6,M},{S, 5,P},{S, 6,P},{S, 7,P},{S, 7,P},{S, 5,P}},
/*;  */ {{S, 6,P},{R, 5,M},{R, 7,M},{R, 7,M},{R, 6,M},{R, 6,M},{S, 6,P},{S, 7,P},{S, 7,P},{S, 5,P}},
/*\n */ {{S, 0,P},{R, 5,M},{R, 7,M},{R, 7,M},{R, 6,M},{R, 6,M},{S, 0,P},{S, 7,P},{S, 7,P},{S, 5,P}},
/*EOF*/ {{R, 0,M},{R, 5,M},{R, 7,M},{R, 7,M},{R, 6,M},{R, 6,M},{S, 0,M},{R, 3,P},{S, 7,P},{S, 5,P}},
/*MC0*/ {{R, 8,P},{R, 5,M},{R, 7,M},{R, 7,M},{R, 6,M},{R, 6,M},{S, 6,P},{S, 7,P},{S, 7,P},{S, 5,P}},
/*[  */ {{R,13,P},{R, 5,M},{R, 7,M},{R, 7,M},{R, 6,M},{R, 6,M},{S, 6,P},{S, 7,P},{S, 7,P},{S, 5,P}},
/*MC1*/ {{R,11,P},{R, 5,M},{R, 7,M},{R, 7,M},{R, 6,M},{R, 6,M},{S, 6,P},{S, 7,P},{S, 7,P},{S, 5,P}},
/*ECP*/ {{S, 9,P},{S, 9,P},{S, 9,P},{S, 9,P},{S, 9,P},{S, 9,P},{S, 6,P},{S, 8,P},{S, 7,P},{S, 5,P}},
/*EEE*/ {{S, 5,P},{R, 5,M},{S, 2,P},{S, 2,P},{R, 6,M},{S, 5,P},{S, 6,P},{S, 7,P},{S, 7,P},{S, 5,P}}
};

/*************************************************************************
 ** These statics are for use by the scgetc and scungetc routines which **
 ** will get and unget chars from 'outBuf'. When outCounting is 1 then  **
 ** the scanner calls these instead of getc & ungetc which allows us to **
 ** scan expression from strings a la (readstr).                        **
 *************************************************************************/
static int outNext = 0;
static int outMax = 0;
static int outCounting = 0;
static char *outBuf = NULL;

/*************************************************************************
 ** Scan a character from outBuf. If we are about to go past end of buf **
 ** return EOF otherwise return next character and increment count.     **
 *************************************************************************/
int scgetc()
{       if (outNext > outMax) return(EOF);
        return(outBuf[outNext++]);
}

/*************************************************************************
 ** unscan a character from outBuf. If we are about to go past pos 0 do **
 ** nothing, otherwise back up once and put the character into the buff.**
 *************************************************************************/
int scungetc(c)
int c;
{       if (outNext > 0)
            outBuf[--outNext] = c;
}

/*************************************************************************
 ** These GETC and UNGETC macros. In normal operations mode !outCounting**
 ** they just call getc and ungetc respectively. In count down mode they**
 ** call the special scanner scgetc and scungetc routines which will get**
 ** characters from a buffer. This is for use by readstr.               **
 *************************************************************************/
#define GETC(fp)      (outCounting ? scgetc() : getc(fp))
#define UNGETC(ch,fp) (outCounting ? scungetc(ch) : ungetc(ch,fp))

/*************************************************************************
 ** The scanner keeps track of line number in lineNum, this routine will**
 ** change the line number and return the previous one.                 **
 *************************************************************************/

int liScanLineNum = 1;
int ScanSetLineNum(n) { int old = liScanLineNum; liScanLineNum = n; return(old); }

/*************************************************************************
 ** nexttoken(b): is just a Deterministic Finite state Autotata. We     **
 ** use the transition table to determine which state is the next one   **
 ** entered and if a character is to be read or not. Entering state 0   **
 ** causes the character storage buffer to be reset, otherwise charact  **
 ** ers are stored in the buffer. We return the token value and the buf **
 ** er when a complete LISP token is identified. The parameter 'b' is   **
 ** just a pointer to the start of a buffer where token text is stored. **
 ** The characters are read from open file 'fp'. fp is set by MAIN. Note**
 ** that in the scan loop we test the TEST BREAK.  This will cause the  **
 ** routine to abort if the user hits break during file input scanning. **
 ** it will not work with console i/o however because there is nothing  **
 ** to interrupt control would return here anyway. We use the ungetc to **
 ** put back the char because we may be called by (readc). for loop at  **
 ** start is a comment and white space skipper. This speeds up loading  **
 ** by 4 or 5%. We read OTHer characters until we have a non white char **
 ** if it is CUL (a comment left char) we read until we get a CUR (the  **
 ** comment right char). If it is not a CUL char we exit the white loop.**
 *************************************************************************/
 static int nexttoken(fp,b)
 FILE *fp;
 char *b;
 {    int state = 0; char *bx; int token = -1; int c; int ch;
      struct action *actptr; int n = MAXATOMSIZE;
      for(;;) {
          for(ch = GETC(fp); chartype[ch+1].type == OTH ; ch = GETC(fp));
          if (ch == CUL) {
              for(ch = GETC(fp); chartype[ch+1].type != CUR ; ch = GETC(fp));
              liScanLineNum += 1;
          } else
              break;
      }
      while ( token == -1) {
              if ((state == 0)||(state == 6)) {bx = b; n = MAXATOMSIZE; };
              if (ch >= 128) { token = 12; goto l12; }                   /* clisp? */
              c = (ch == EOF) ? -1 : (((int) ch) & 0x7f);
              actptr = &transtable[chartype[c+1].type][state];
              switch (actptr->type) {
                      case S : state =  actptr->number; break;
                      case R : token =  actptr->number; break;
              }
              if (actptr->flag == P) {
                  if (ch == '\n')
                      liScanLineNum += 1;
                  if (state != 9) {                        /* state number */
                     if (--n <= 0) {                       /* need room for '\0' */
                        b[ MAXATOMSIZE - 1 ] = '\0';
                        b[ 0 ] = '\n';                     /* get rid of leading " and force new line */
                        serror(NULL, "atom or string has too many characters or quote is missing", b, -1);
                     }
                     *bx++ = ch;                           /* 9 is \ handler */
                  }
                  if (fp != stdin)                         /* poll break status*/
                     TEST_BREAK();                         /* break hit ?? */
                  ch = GETC(fp);
              }
      }
      *bx = '\0';                                       /* terminate token text*/
      if ((token == 0)&&(fp == stdin))                  /* contortions to handle*/
          clearerr(fp);                                 /* stdin EOF because it */
      else                                              /* may occur several */
 l12:     UNGETC(ch,fp);                                /* times for er> exits */
#     if DEBUG
          printf("SCAN: tok = %s, liScanLineNum = %d\n", b, liScanLineNum);
#     endif
      return(token);                                    /* for next file.  */
 }

#define         OPENTOKEN               1               /* the ( token */
#define         CLOSETOKEN              2               /* the ) token */
#define         HASHTOKEN               3               /* the |...| token */
#define         CLALLTOKEN              4               /* the ] token */
#define         ALPHATOKEN              6               /* a literal token */
#define         QUOTETOKEN              8               /* the ' token     */
#define         STRINGTOKEN             10              /* a "..." token */
#define         OPALLTOKEN              13              /* the [ token */

/******************************************************************************
 ** metastack handling functions: Keep track of the nesting level of the     **
 ** current meta [ on the top of the stack, popped when ] is encountered.    **
 ******************************************************************************/
 static int TopOfMetaStack = 0;
 static int MetaStack[MAXMETANEST];
 static int retcount = 0;
 static int nestlevel = 0;
                                                 /***/
 static int metapush(n)
 int n;
 {  if (TopOfMetaStack >= MAXMETANEST)
        serror(NULL,"meta [] nesting too deep",NULL,-1);
    MetaStack[TopOfMetaStack++] = n;
 }
                                                  /***/
 static int metapop()
 {  if (TopOfMetaStack <= 0) return(0);
    return(MetaStack[--TopOfMetaStack]);
 }
                                                  /***/
 static int metareset()
 {  nestlevel = retcount = TopOfMetaStack = 0;
 }

/******************************************************************************
 ** ShiftAndResolve(s) Will return either ALPHATOKEN or STRINGTOKEN depending**
 ** on the type of token represented in string 's'. This string contains the **
 ** delimiters | | or " " plus possible escape sequences using a character in**
 ** class ECP (usually the \ char). As a side effect, ShiftAndResolve will   **
 ** get rid of these delimiters, and convert the escape sequences \n,\t,\f,  **
 ** ,\b, and \r to the correct character newline,tab,formfeed,backspace or CR**
 ** For example, the string ["ab\ncd"] ===> [ab<lf>cd], note the string is   **
 ** shifted left once to get rid of the left most delimiter " and the copy   **
 ** caused the "\"+"n" sequence to be replaced by the LF character, also note**
 ** the right most " was overcopied with the end of string '\0' char.  NOTE: **
 ** since this expansion is non FRANZ, we only do it if option SMARTSLASH is **
 ** set otherwise we do the normal FRANZ thing.                              **
 ******************************************************************************/
 int ShiftAndResolve(s)
 char *s;
 {    register int c,ss=GetOption(SMARTSLASH);  /* do "\*" ==> '\*' txlat? */
      register char *d = s;                     /* dest of copy */
      register int delim = *s++;                /* delimiting character is */
      for(c = *s++; *s != '\0'; c = *s++, d++)
      {   if (chartype[(c & 0x7f)+1].type == ECP)
          {   c = *s++;                         /* get char after the \ */
              if (ss)
              {  if (c == 'n')                   /* it is \n (newline) */
                     c = '\n';                   /* alter the copied char */
                 else
                 if (c == 't')                   /* is it \t (tab) */
                     c = '\t';                   /* alter the copied char */
                 else
                 if (c == 'b')                   /* is it \b (backspace) */
                     c = '\b';                   /* alter the copied char */
                 else
                 if (c == 'r')                   /* is it \r (carriage ret) */
                     c = '\r';                   /* alter the copied char */
                 else
                 if (c == 'f')                   /* is it \f (formfeed) */
                     c = '\f';                   /* alter the copied char */
              };
          };
          *d = c;
      };                                       /* clobber the end delimiter */
      *d = '\0';
      return( delim == '|' ? ALPHATOKEN : STRINGTOKEN );
 }

/******************************************************************************
 ** ExpandEscapesInto(d,s) Will copy the source string 's' to the destination**
 ** string 'd' expanding any funny characters as it goes. It will expand the **
 ** tab,backspace,newline,formfeed,carriage return. Others are not done.     **
 ** We assume that the provided buffer is at least twice the size of the     **
 ** source buffer to allow for doubling of size after intoducing the \ char. **
 ** Note that if we find a " or | in the string we must escape it \" or \|.  **
 ** Since this expansion is non FRANZ, we only do it if option SMARTSLASH is **
 ** set otherwise we do the normal FRANZ thing.                              **
 ******************************************************************************/
ExpandEscapesInto(d,s)
char *d,*s;
{    register char c;
     register int ss = GetOption(SMARTSLASH);
     while(c = *s++)
     {   if ((c < ' ')||(c > 128)||(c == '|')||(c == '"')||(c == '\\'))
         {   if (ss)
             {   switch(c)
                 {   case '\r' : c = 'r'; break;
                     case '\b' : c = 'b'; break;
                     case '\t' : c = 't'; break;
                     case '\n' : c = 'n'; break;
                     case '\f' : c = 'f'; break;
                     case '\\' :
                     case '|'  :
                     case '"'  :          break;
                     default   : c = '?'; break;       /* some funny char ? */
                 };
             };
             *d++ = '\\';
             *d++ = c;
         }
         else
             *d++ = c;
     };
     *d = '\0';
}

/******************************************************************************
 ** scan(fp,b) Returns    the next token from the function 'nexttoken' unless**
 ** we are closing the nesting (requested by a token ]) in the input stream. **
 ** If this is the case we return one ) for every call. In order to know how **
 ** many )'s are needed we keep track of the nesting level. The nesting level**
 ** will be increased by '(' tokens and decreased by ')' tokens. Note that we**
 ** keep track of nested ['s on the meta stack. Pop of empty stack is 0 This **
 ** allows us to correctly handle the close everything convention of single ]**
 ** If the current token is type 3 that is a |......| type of literal token  **
 ** or a string token like "....." then we call the function ShiftAndResolve **
 ** to get rid of the delimiters (shift) and to expand contract all \x esca  **
 ** pe sequences and to return ALPHATOKEN or STRINGTOKEN depending on the    **
 ** delimiters value.                                                        **
 *****************************************************************************/
 int scan(fp,r)
 FILE *fp;
 char *r;
 {    int next;
      if (retcount > 0)                                 /* if in close all */
      {   retcount--;                                   /* mode, return )  */
          return(CLOSETOKEN);
      };
      next = nexttoken(fp,r);                           /* not in close all */
      switch(next)                                      /* mode so get next */
      {    case OPENTOKEN  : nestlevel++;               /* it was ( count up*/
                             return(next);              /* and return it.   */
           case CLOSETOKEN : nestlevel--;               /* it was ) so count*/
                             return(next);
           case OPALLTOKEN : metapush(nestlevel);       /* save nesting */
                             nestlevel = 1;             /* we are 1 deep again*/
                             return(OPENTOKEN);
           case CLALLTOKEN : retcount = nestlevel - 1;  /* minus this ) */
                             nestlevel = metapop();     /* back to old depth */
                             return(CLOSETOKEN);
           case HASHTOKEN  : return(ShiftAndResolve(r));
           default         : return(next);
      };
 }

/******************************************************************************
 ** isalphatoken(s) Will return 't if the string 's could be picked up as one**
 ** complete alpha token by the scanner. This is used to control the printing**
 ** of '|'s around alpha tokens by the printatom routine. This is easy to    **
 ** check by running the DFA on string 's' and seeing if it runs strlen(s)   **
 ** times and if in the token returned is #6 (ALPHATOKEN).                   **
 ******************************************************************************/
 int isalphatoken(s)
 char *s;
 {    int state = 0, token = -1, c, n = 0;
      struct action *actptr;
      char ch, *t = s;
      ch = *t++;
      while( token == -1 )
      {       if ((state == 0)||(state == 6)) n = 0;
              c = ((int) ch);
              if ((c > 127 )||(c < 0)) return(0);
              actptr = &transtable[chartype[c+1].type][state];
              switch (actptr->type)
              {       case S : state =  actptr->number; break;
                      case R : token =  actptr->number; break;
              };
              if (actptr->flag == P)
              {   if (state == 9) return(0);
                  n += 1;                      /* state 9 handels the \ char */
                  ch = *t++;
              };
      };
      return(((n==strlen(s))&&(token == ALPHATOKEN)));
}

/**************************************************************************
 ** ScanFromBuffer(buf,n) - will cause scanning to continue from 'buffer'**
 ** from which up to N characters may be scgetc'ed. If it is exhaused it **
 ** feeds EOF's to the scanner. This is for use by readstr.              **
 **************************************************************************/
ScanFromBuffer(buf,n)
char *buf; int n;
{     outBuf = buf;
      outNext = 0;
      outMax = n-1;
      outCounting = 1;
}

/**************************************************************************
 ** ScanReset() - will reset the scanner. This is called when an error   **
 ** occurs. It set the outcount mode off and resets the meta '[]' system.**
 ** It is also called by the readstr routine to end scanning from a string*
 ** and to find if any characters are left unscanned in the bufffer.     **
 **************************************************************************/
int ScanReset()
{     int r = outMax - outNext;   /* there is an ungetc ' ' on EOF */
      metareset();
      outNext = outMax = outCounting = 0;
      outBuf = NULL;
      return(r);
}
