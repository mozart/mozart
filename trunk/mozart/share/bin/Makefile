SHELL = /bin/sh










help:
	@echo ""
	@echo "The following targets are available"
	@echo "(use 'make <target>' for installation)"
	@echo "	oz2lpr		mehl@dfki.uni-sb.de"
	@echo ""

Makefile: Makefile.m4
	m4 Makefile.m4 > Makefile


oz2lpr:	/usr/share/gs/soft/oz/bin/oz2lpr

/usr/share/gs/soft/oz/bin/oz2lpr:   oz2lpr.csh
	install -m 555 $< $@


