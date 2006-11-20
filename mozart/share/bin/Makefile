# Generated automatically from Makefile.in by configure.
#  Authors:
#    Michael Mehl (mehl@dfki.de)
# 
#  Contributors:
#    optional, Contributor's name (Contributor's email address)
# 
#  Copyright:
#    Michael Mehl (mehl@dfki.de)
# 
#  Last change:
#    $Date$ by $Author$
#    $Revision$
# 
#  This file is part of Mozart, an implementation 
#  of Oz 3:
#     http://www.mozart-oz.org
# 
#  See the file "LICENSE" or
#     http://www.mozart-oz.org/LICENSE.html
#  for information on usage and redistribution 
#  of this file, and for a DISCLAIMER OF ALL 
#  WARRANTIES.

SRCTOP=		/home/ado51/mozart_geoz_source/mozart
BUILDTOP=	/home/ado51/mozart_geoz_source/mozart
PREFIX=		/home/ado51/mozart_geoz
SRCDIR= 	/home/ado51/mozart_geoz_source/mozart/share/bin
VPATH=	 	/home/ado51/mozart_geoz_source/mozart/share/bin
BINDIR=		$(PREFIX)/bin


LN_S=		ln -s
INSTALL=	/usr/bin/install -c
INSTALL_DIR=	/home/ado51/mozart_geoz_source/mozart/mkinstalldirs
INSTALL_BIN=	$(INSTALL) -m 555
INSTALL_FILE=	$(INSTALL) -m 444

SCRIPTS=	oz ozplatform
LINKS=		ozengine text2pickle pickle2text oldpickle2text oztool
LIB_SCRIPTS=	$(addprefix $(BINDIR)/,$(SCRIPTS))
LIB_LINKS=	$(addprefix $(BINDIR)/,$(LINKS))
WRAPPERS0=	ozwrapper.bin ozwrapperw.bin
WRAPPERS=	$(addprefix $(BINDIR)/,$(WRAPPERS0))


.PHONY: bootstrap all install clean veryclean distclean depend check

all:

oz:
	true

bootstrap:

install: $(BINDIR) $(LIB_SCRIPTS) $(LIB_LINKS) $(WRAPPERS)


$(LIB_SCRIPTS): $(BINDIR)/% : %
	$(INSTALL_BIN) $< $(BINDIR)

$(LIB_LINKS): $(BINDIR)/oz
	cd $(BINDIR); rm -f $@; $(LN_S) oz $@

$(WRAPPERS): $(BINDIR)/%: $(SRCTOP)/platform/mswindows/%
	$(INSTALL_FILE) $< $@

$(BINDIR):
	$(INSTALL_DIR) $@

veryclean: clean

distclean: clean
	rm -f Makefile


clean:
	rm -f *.o *.exe *.obj *.sym *.map *.mk* *.lk* *.err oz.res setup.res version.h

Makefile: Makefile.in ../config.status
	cd .. && ./config.status

../config.status: ../configure
	cd .. && ./config.status --recheck

check:

depend:
