# Generated automatically from Makefile.in by configure.
#
#  Authors:
#    Michael Mehl <mehl@dfki.de>
#    Leif Kornstaedt <kornstae@ps.uni-sb.de>
#
#  Copyright:
#    Michael Mehl, 1998
#    Leif Kornstaedt, 1998
#
#  Last change:
#    $Date$ by $Author$
#    $Revision$
#
#  This file is part of Mozart, an implementation of Oz 3:
#    http://www.mozart-oz.org
#
#  See the file "LICENSE" or
#    http://www.mozart-oz.org/LICENSE.html
#  for information on usage and redistribution
#  of this file, and for a DISCLAIMER OF ALL
#  WARRANTIES.
#

SRCTOP		= /home/ado51/mozart_geoz_source/mozart
BUILDTOP=	/home/ado51/mozart_geoz_source/mozart
PREFIX=		/home/ado51/mozart_geoz

VPATH		= /home/ado51/mozart_geoz_source/mozart/share/elisp
SRCDIR		= /home/ado51/mozart_geoz_source/mozart/share/elisp
ELISPDIR	= $(PREFIX)/share/elisp

EMACS		= emacs

INSTALL		= /usr/bin/install -c
INSTALL_FILE	= $(INSTALL) -m 444
INSTALL_DIR	= /home/ado51/mozart_geoz_source/mozart/mkinstalldirs

COMPILE_ELISP	= yes
ELFILES		= oz.el mozart.el oz-extra.el oz-server.el
ELCFILES	= $(addsuffix c,$(ELFILES))
ifeq ($(COMPILE_ELISP),yes)
TARGETS		= $(ELFILES) $(ELCFILES)
else
TARGETS		= $(ELFILES)
endif

LIBFILES	= $(addprefix $(ELISPDIR)/,$(TARGETS)) \

CYGPATH		= $(SRCTOP)/share/lib/cygpath.sh
CYG_SRCDIR	= $(shell $(CYGPATH) $(SRCDIR))

.PHONY: bootstrap all install clean veryclean distclean tags check depend

all: $(TARGETS)

bootstrap: all

Makefile: Makefile.in ../config.status
	cd .. && ./config.status

mozart.el: mozart.el.in ../config.status
	cd .. && ./config.status

../config.status: ../configure
	cd .. && ./config.status --recheck

mozart.elc: oz.elc

$(ELCFILES): %.elc : %.el
	$(EMACS) --eval "(setq load-path (cons \"$(CYG_SRCDIR)\" load-path))" -q -batch -f batch-byte-compile $(shell $(CYGPATH) "$<") && \
	if [ $<c != $@ ]; then mv $<c $@; fi

install: all $(ELISPDIR) $(LIBFILES)

$(ELISPDIR):
	$(INSTALL_DIR) $@

$(ELISPDIR)/%.el: %.el
	$(INSTALL_FILE) $< $@

$(ELISPDIR)/%.elc: %.elc
	$(INSTALL_FILE) $< $@

clean:
	-rm -f *.elc *~ TAGS

veryclean: clean

distclean: clean
	-rm -f Makefile mozart.el

tags:
	etags $(ELFILES)

check:

depend:
