# PC-LISP top-level Makefile.
#
# Documentation targets only at this level (the interpreter has its own
# Makefile under src/). Build with:
#
#    make pc-lisp.pdf      # the manual as a PDF, with TOC at the front
#    make pc-lisp.ps       # the manual as PostScript, with TOC at the front
#    make doc              # both
#    make clean            # remove all build artifacts
#
# The TOC is built in two passes. Pass 1 renders the manual with -ms's
# native trailing TOC and we capture that page's text. Pass 2 sources
# the captured TOC as a typeset block at the front of the document and
# suppresses the trailing .TC.

GROFF   ?= groff
PDFROFF ?= pdfroff

DOC_SRC := pc-lisp.ms
TOC     := pc-lisp.toc
PS      := pc-lisp.ps
PDF     := pc-lisp.pdf

.PHONY: doc clean

doc: $(PS) $(PDF)

# Pass 1 produces an intermediate text rendering containing the native
# trailing TOC. We extract those lines verbatim, prefix each with a
# zero-width "no-op" so groff treats them as text, and bracket the
# block in .nf / .fi (no-fill mode) so the original spacing -- which
# already aligns titles, dot leaders, and page numbers -- is preserved
# exactly as -ms laid it out.
$(TOC): $(DOC_SRC)
	@echo '  GROFF [pass 1] $@'
	@printf '.SH\nTable of Contents\n.LP\n.nf\n' > $@
	@$(GROFF) -ms -rTPASS=1 -Tutf8 -P-cbou $(DOC_SRC) 2>/dev/null \
	| awk '/Table of Contents/{f=1; next} f && /[A-Za-z]/' \
	| sed -e 's/^/\\\&/' \
	      -e 's/\xe2\x80\x90/-/g' \
	  >> $@
	@printf '.fi\n.bp\n' >> $@

# Pass 2 renders normally with TPASS=2; the document sources $(TOC)
# at the front of the body and skips the built-in trailing .TC.
$(PS): $(DOC_SRC) $(TOC)
	@echo '  GROFF [pass 2] $@'
	@$(GROFF) -ms -rTPASS=2 $(DOC_SRC) > $@

$(PDF): $(DOC_SRC) $(TOC)
	@echo '  PDFROFF       $@'
	@$(PDFROFF) -ms -rTPASS=2 $(DOC_SRC) > $@

clean:
	rm -f $(PS) $(PDF) $(TOC)
