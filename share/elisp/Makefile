.SUFFIXES: .el .elc

ELS  = comint.el oz.el ozstartup.el blink-paren.el oz1.el
ELCS = $(ELS:.el=.elc)

help:
	@echo ""
	@echo "The following targets are available"
	@echo "(use 'make <target>' for installation)"
	@echo "	install		{scheidhr,mehl}@dfki.uni-sb.de"
	@echo ""

install: $(ELCS)
	install -m 444 $(ELS) $(ELCS) $(OZHOME)/lib/elisp

oz.el: comint.el

blink-paren.elc: blink-paren.el

.el.elc:
	@echo "(setq load-path (cons \".\" load-path))\
               (byte-compile-file \"$<\")" > compile
	lemacs -batch -f batch-byte-compile $<
	rm -f compile

clean:
	rm -f $(ELCS) *~

