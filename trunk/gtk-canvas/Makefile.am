## Process this file with automake to produce Makefile.in.

# Actually the earliest release that should be OK should be 1.2e, I think.
# Still, 1.3 is the first "released" automake that is OK.
AUTOMAKE_OPTIONS = 1.3


if COMPILE_TEST_GTK_CANVAS
test_gtk_canvas = test
else
test_gtk_canvas =
endif

SUBDIRS =  libart_lgpl \
	gtk-canvas\
	test

include_HEADERS = gtk-canvas.h

EXTRA_DIST =	


release:
	$(MAKE) dist distdir=$(PACKAGE)-$(VERSION)


