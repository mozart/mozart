PREFIX		= @prefix@
VPATH		= @srcdir@
SHDOCDIR	= $(PREFIX)/share/doc
DOCDIR		= $(PREFIX)/doc

INSTALL		= @INSTALL@
INSTALL_FILE	= $(INSTALL) -m 444
INSTALL_DIR	= @INSTALL_DIR@
HOMEURL         = @HOMEURL@

.PHONY: all quick bootstrap check install clean veryclean distclean depend

all bootstrap:: TeX.class

%.java: %.class
	java $<

Makefile: Makefile.in ../config.status
	../config.status

veryclean clean::
	-rm -f *~ TeX.class

distclean:: veryclean
	-rm -f Makefile
