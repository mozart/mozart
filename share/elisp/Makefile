SHELL = /bin/sh









help:
	@echo ""
	@echo "The following targets are available"
	@echo "(use 'make <target>' for installation)"
	@echo "	oz		mehl@dfki.uni-sb.de"
	@echo ""

Makefile: Makefile.m4
	m4 Makefile.m4 > Makefile


oz:  /usr/share/gs/soft/oz/elisp/oz.el

/usr/share/gs/soft/oz/elisp/oz.el:   oz.el
	install -m 664 $< $@

	echo "(byte-compile-file \"/usr/share/gs/soft/oz/elisp/oz.el\")" > compile
	lemacs -batch -load compile
	rm compile
