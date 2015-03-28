
SRC = buabs.c buacos.c buadd1.c bualphp.c buand.c buappend.c buarg.c buargq.c \
buarray.c buarrayd.c buarrayp.c buascii.c buasembl.c buasin.c buassoc.c \
buatan.c buatom.c buattach.c bubaktra.c buboole.c buboundp.c bucadar.c \
bucar.c bucaseq.c bucdr.c buchix.c buclmemu.c bucmdlna.c bucompil.c \
buconcat.c bucond.c bucons.c bucopy.c bucopysy.c bucos.c bucxr.c budeclar.c \
budefine.c budefun.c budelete.c budiff.c budisasm.c budivide.c budrain.c \
bueq.c buequal.c buequals.c buevenp.c buexec.c buexit.c buexp.c buexplod.c \
buexpmem.c buexpt.c bufact.c bufclose.c bufilepo.c bufilest.c bufillar.c \
bufix.c bufixp.c buflatc.c buflatsi.c buflattn.c bufloat.c bufloatp.c \
bufopen.c buforeac.c bugc.c bugensym.c buget.c bugetdat.c bugetd.c \
bugetenv.c bugetlen.c bugo.c bugpname.c bugrapic.c bugreap.c bugthan.c \
buhashst.c buhsize.c buhtolis.c buhunk.c buhunkp.c buimplod.c buintern.c \
bulast.c buldiff.c bulength.c bulessp.c bulinenu.c bulistar.c bulist.c \
bulistif.c bulistp.c buload.c bulog10.c bulog.c bulsh.c bulthan.c bumacroe.c \
bumakhun.c bumaknam.c bumakunb.c bumap.c bumax.c bumember.c bumemq.c \
bumemsta.c bumemusa.c bumin.c buminus.c buminusp.c bumod.c bunconc.c \
bunexplo.c bunot.c bunth.c bunthcdr.c bunthcha.c bunull.c bunumbp.c \
buoblist.c buoddp.c buonemin.c buoneplu.c buor.c bupairli.c buparset.c \
bupatom.c bupeepho.c buplist.c buplus.c buplusp.c buportp.c buppform.c \
buprinc.c buprint.c buprintf.c buprints.c buproces.c buproduc.c buprog.c \
buput.c buputd.c buquote.c buquotie.c burandom.c buread.c bureadc.c bureadln.c \
bureadst.c buremob.c burempro.c burepeat.c bureseti.c bureturn.c burevers.c \
burplaca.c burplacd.c burplacx.c buscanf.c buselect.c busetarg.c buset.c \
busetpli.c busetq.c busets.c busetsyn.c bushowst.c busin.c busizeof.c \
busleep.c busopen.c busort.c busqrt.c busstatu.c bustrcom.c bustrfnd.c \
bustring.c bustrlen.c bustrpad.c bustrtrm.c busub1.c busubst.c busubstr.c \
busum.c busymtb.c busystim.c busysunl.c butildee.c butimeev.c butimes.c \
butimest.c butolowr.c butouppr.c butrace.c butruena.c butype.c buuconca.c \
buuntrac.c buwhile.c buzaplin.c buzerop.c liarrays.c libio.c liceval.c \
lierror.c lieval.c liinst.c limain.c limkstak.c limman.c liscan.c \
lithrow.c litrace.c liutils.c

CFLAGS = -Wall -Wno-parentheses -O3

OBJ = $(SRC:.c=.o)

pc-lisp : $(OBJ)
	gcc -o $@ $^ -lm


$(OBJ) : lisp.h

clean:
	rm -f *.o *.bak *~ *.obj

realclean: clean
	rm -f pc-lisp pc-lisp.exe
