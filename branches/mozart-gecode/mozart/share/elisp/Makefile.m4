SHELL = /bin/sh

.SUFFIXES: .el .elc

ELS  = comint.el oz.el ozstartup.el blink-paren.el
ELCS = $(ELS:.el=.elc)

define(OzDir,/usr/share/gs/soft/oz)
define(OzLib,OzDir/lib)
define(OzInc,OzDir/`include')
define(OzLisp,OzLib/elisp)


help:
	@echo ""
	@echo "The following targets are available"
	@echo "(use 'make <target>' for installation)"
	@echo "	elcs		{scheidhr,mehl}@dfki.uni-sb.de"
	@echo ""

Makefile: Makefile.m4
	m4 Makefile.m4 > Makefile


elcs:    $(ELCS)

install: elcs
	install -m 444 $(ELS) $(ELCS) OzLisp

oz.el: comint.el

blink-paren.elc: blink-paren.el

.el.elc:
	@echo "(setq load-path (cons \".\" load-path))\
               (byte-compile-file \"$<\")" >> compile
	lemacs -batch -f batch-byte-compile $<
#	rm -f compile
