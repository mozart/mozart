## Process this file with automake to produce Makefile.in

#
# The targets
#
lib_LTLIBRARIES = libgtk-canvas.la

#
# Install location for header files
#
libgtkcanvasincludedir = $(includedir)/gtk-canvas

INCLUDES = -I.. -I$(srcdir)/.. \
	-I$(includedir) $(GDK_IMLIB_CFLAGS) 

EXTRA_HEADERS = 

libgtk_canvas_la_SOURCES = \
	gtk-canvas.c			\
	gtk-canvas-image.c		\
	gtk-canvas-init.c		\
	gtk-canvas-load.c		\
	gtk-canvas-line.c		\
	gtk-canvas-rect-ellipse.c	\
	gtk-canvas-polygon.c		\
	gtk-canvas-text.c		\
	gtk-canvas-types.c               \
	gtk-canvas-util.c		\
	gtk-canvas-widget.c

## this lists all the non-generated headers
gtk_canvas_headers = \
	gtk-canvas.h			\
	gtk-canvas-defs.h		\
	gtk-canvas-image.h		\
	gtk-canvas-init.h		\
	gtk-canvas-line.h		\
	gtk-canvas-load.h		\
	gtk-canvas-rect-ellipse.h	\
	gtk-canvas-polygon.h		\
	gtk-canvas-text.h		\
	gtk-canvas-types.h		\
	gtk-canvas-util.h		\
	gtk-canvas-widget.h

libgtkcanvasinclude_HEADERS = \
	$(gtk_canvas_headers)	\
	gtk-canvastypebuiltins.h

BUILT_SOURCES = \
	gtk-canvastypebuiltins.h		\
	gtk-canvastypebuiltins_evals.c	\
	gtk-canvastypebuiltins_ids.c		\
	gtk-canvastypebuiltins_vars.c

# generate gnome.defs file from gnome-boxed.defs and *.h
gtk-canvas.defs: @REBUILD@ makeenums.pl gtk-canvas-boxed.defs $(gtk_canvas_headers)
	cd $(srcdir) \
	&& $(PERL) makeenums.pl defs $(gtk_canvas_headers) > gd.tmp \
	&& cat gtk-canvas-boxed.defs >> gd.tmp \
	&& mv gd.tmp gtk-canvas.defs

# generate type identifier header (GTK_TYPE_WIDGET_FLAGS)
gtk-canvastypebuiltins.h: @REBUILD@ gtk-canvas.defs maketypes.awk
	cd $(srcdir) \
	&& LC_COLLATE=C $(AWK) -f maketypes.awk gtk-canvas.defs macros > gtb.tmp \
	&& mv gtb.tmp gtk-canvastypebuiltins.h

# generate type identifier variables (GTK_TYPE_WIDGET_FLAGS)
gtk-canvastypebuiltins_vars.c: @REBUILD@ gtk-canvas.defs maketypes.awk
	cd $(srcdir) \
	&& LC_COLLATE=C $(AWK) -f maketypes.awk gtk-canvas.defs variables > gtbv.tmp \
	&& mv gtbv.tmp gtk-canvastypebuiltins_vars.c

# generate type entries for type-id registration
gtk-canvastypebuiltins_ids.c: @REBUILD@ gtk-canvas.defs maketypes.awk
	cd $(srcdir) \
	&& LC_COLLATE=C $(AWK) -f maketypes.awk gtk-canvas.defs entries > gtbi.tmp \
	&& mv gtbi.tmp gtk-canvastypebuiltins_ids.c

# generate enum value arrays
gtk-canvastypebuiltins_evals.c: @REBUILD@ makeenums.pl gtk-canvas.defs
	cd $(srcdir) \
	&& $(PERL) makeenums.pl arrays $(gtk_canvas_headers) > gtbe.tmp \
	&& mv gtbe.tmp gtk-canvastypebuiltins_evals.c

#######################


EXTRA_DIST =\
	AUTHORS					\
	gtk-canvas.defs 				\
	maketypes.awk				\
	makeenums.pl				\
	gtk-canvas-boxed.defs			\
	gtk-canvastypebuiltins_vars.c		\
	gtk-canvastypebuiltins_ids.c			\
	gtk-canvastypebuiltins_evals.c

libgtk_canvas_la_LDFLAGS = -version-info 1:1:0 -rpath $(libdir)
libgtk_canvas_la_LIBADD = $(LIBGTK_CANVAS_LIBS)

