OZHOME=/usr/share/gs/soft/oz


.SUFFIXES: .el .elc

ELS  = oz.el ozstartup.el
ELCS = $(ELS:.el=.elc)

help:
	@echo ""
	@echo "The following targets are available"
	@echo "	install		{scheidhr,mehl}@dfki.uni-sb.de"
	@echo ""

install: $(ELCS)
	install -m 444 $(ELS) $(ELCS) $(OZHOME)/lib/elisp

.el.elc:
	@echo "(setq load-path (cons \".\" load-path))\
               (byte-compile-file \"$<\")" > compile
	lemacs -batch -f batch-byte-compile $<
	rm -f compile

clean:
	rm -f $(ELCS) *~

