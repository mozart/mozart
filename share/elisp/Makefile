SHELL = /bin/sh

.SUFFIXES: .el .elc

ELS  = comint.el oz.el ozstartup.el blink-paren.el
ELCS = $(ELS:.el=.elc)







help:
	@echo ""
	@echo "The following targets are available"
	@echo "(use 'make <target>' for installation)"
	@echo "	oz		mehl@dfki.uni-sb.de"
	@echo ""

Makefile: Makefile.m4
	m4 Makefile.m4 > Makefile


elcs:    $(ELCS)

install: elcs
	install -m 444 $(ELS) $(ELCS) /usr/share/gs/soft/oz/elisp

oz.el: comint.el

blink-paren.elc: blink-paren.el

.el.elc:
	@echo "(setq load-path (cons \".\" load-path))\
               (byte-compile-file \"$<\")" >> compile
	lemacs -batch -f batch-byte-compile $<
#	rm -f compile