## Process this file with automake to produce Makefile.in

INCLUDES = -I.. -I$(srcdir)/.. \
	-I$(includedir) $(GDK_IMLIB_CFLAGS) 
	

noinst_PROGRAMS = test-gtkcanvas

test_gtkcanvas_SOURCES =		\
	canvas.c		\
	canvas-primitives.c	\
	canvas-arrowhead.c	\
	canvas-features.c	\
	canvas-fifteen.c	\
	test-gtkcanvas.c		\
	test-gtkcanvas.h

EXTRA_DIST = 		\
	bomb.xpm	\
	flower.png	\
	toroid.png

test_gtkcanvas_LDADD = \
	$(top_builddir)/gtk-canvas/libgtk-canvas.la 	\
	$(top_builddir)/libart_lgpl/libart_lgpl.la $(LIBGTK_CANVAS_LIBS)

# the program has to be run from $srcdir to find its image files
test: test-gtkcanvas
	blddir=`pwd`; top_blddir=`cd $(top_builddir) && pwd`; cd $(srcdir); \
	$(SHELL) $$top_blddir/libtool --mode=execute $$blddir/test-gtkcanvas

test-debug: test-gtkcanvas
	blddir=`pwd`; top_blddir=`cd $(top_builddir) && pwd`; cd $(srcdir); \
	$(SHELL) $$top_blddir/libtool --mode=execute gdb $$blddir/test-gtkcanvas

