/* Rectangle and ellipse item types for GtkCanvas widget
 *
 * GtkCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 * Copyright (C) 1998 The Free Software Foundation
 *
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 */

#ifndef GTK_CANVAS_RECT_ELLIPSE_H
#define GTK_CANVAS_RECT_ELLIPSE_H

#include <gtk-canvas/gtk-canvas-defs.h>
#include "gtk-canvas/gtk-canvas.h"

#include <libart_lgpl/art_svp.h>

BEGIN_GTK_CANVAS_DECLS


/* Base class for rectangle and ellipse item types.  These are defined by their top-left and
 * bottom-right corners.  Rectangles and ellipses share the following arguments:
 *
 * name			type		read/write	description
 * ------------------------------------------------------------------------------------------
 * x1			double		RW		Leftmost coordinate of rectangle or ellipse
 * y1			double		RW		Topmost coordinate of rectangle or ellipse
 * x2			double		RW		Rightmost coordinate of rectangle or ellipse
 * y2			double		RW		Bottommost coordinate of rectangle or ellipse
 * fill_color		string		W		X color specification for fill color,
 *							or NULL pointer for no color (transparent)
 * fill_color_gdk	GdkColor*	RW		Allocated GdkColor for fill
 * outline_color	string		W		X color specification for outline color,
 *							or NULL pointer for no color (transparent)
 * outline_color_gdk	GdkColor*	RW		Allocated GdkColor for outline
 * fill_stipple		GdkBitmap*	RW		Stipple pattern for fill
 * outline_stipple	GdkBitmap*	RW		Stipple pattern for outline
 * width_pixels		uint		RW		Width of the outline in pixels.  The outline will
 *							not be scaled when the canvas zoom factor is changed.
 * width_units		double		RW		Width of the outline in canvas units.  The outline
 *							will be scaled when the canvas zoom factor is changed.
 */


#define GTK_CANVAS_TYPE_CANVAS_RE            (gtk_canvas_re_get_type ())
#define GTK_CANVAS_RE(obj)            (GTK_CHECK_CAST ((obj), GTK_CANVAS_TYPE_CANVAS_RE, GtkCanvasRE))
#define GTK_CANVAS_RE_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GTK_CANVAS_TYPE_CANVAS_RE, GtkCanvasREClass))
#define GTK_CANVAS_IS_CANVAS_RE(obj)         (GTK_CHECK_TYPE ((obj), GTK_CANVAS_TYPE_CANVAS_RE))
#define GTK_CANVAS_IS_CANVAS_RE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_CANVAS_TYPE_CANVAS_RE))


typedef struct _GtkCanvasRE      GtkCanvasRE;
typedef struct _GtkCanvasREClass GtkCanvasREClass;

struct _GtkCanvasRE {
	GtkCanvasItem item;

	double x1, y1, x2, y2;		/* Corners of item */
	double width;			/* Outline width */

	guint fill_color;		/* Fill color, RGBA */
	guint outline_color;		/* Outline color, RGBA */

	gulong fill_pixel;		/* Fill color */
	gulong outline_pixel;		/* Outline color */

	GdkBitmap *fill_stipple;	/* Stipple for fill */
	GdkBitmap *outline_stipple;	/* Stipple for outline */

	GdkGC *fill_gc;			/* GC for filling */
	GdkGC *outline_gc;		/* GC for outline */

	/* Antialiased specific stuff follows */

	ArtSVP *fill_svp;		/* The SVP for the filled shape */
	ArtSVP *outline_svp;		/* The SVP for the outline shape */

	/* Configuration flags */

	unsigned int fill_set : 1;	/* Is fill color set? */
	unsigned int outline_set : 1;	/* Is outline color set? */
	unsigned int width_pixels : 1;	/* Is outline width specified in pixels or units? */
};

struct _GtkCanvasREClass {
	GtkCanvasItemClass parent_class;
};


/* Standard Gtk function */
GtkType gtk_canvas_re_get_type (void);


/* Rectangle item.  No configurable or queryable arguments are available (use those in
 * GtkCanvasRE).
 */


#define GTK_CANVAS_TYPE_CANVAS_RECT            (gtk_canvas_rect_get_type ())
#define gtk_canvas_RECT(obj)            (GTK_CHECK_CAST ((obj), GTK_CANVAS_TYPE_CANVAS_RECT, GtkCanvasRect))
#define GTK_CANVAS_RECT_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GTK_CANVAS_TYPE_CANVAS_RECT, GtkCanvasRectClass))
#define GTK_CANVAS_IS_CANVAS_RECT(obj)         (GTK_CHECK_TYPE ((obj), GTK_CANVAS_TYPE_CANVAS_RECT))
#define GTK_CANVAS_IS_CANVAS_RECT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_CANVAS_TYPE_CANVAS_RECT))


typedef struct _GtkCanvasRect GtkCanvasRect;
typedef struct _GtkCanvasRectClass GtkCanvasRectClass;

struct _GtkCanvasRect {
	GtkCanvasRE re;
};

struct _GtkCanvasRectClass {
	GtkCanvasREClass parent_class;
};


/* Standard Gtk function */
GtkType gtk_canvas_rect_get_type (void);


/* Ellipse item.  No configurable or queryable arguments are available (use those in
 * GtkCanvasRE).
 */


#define GTK_CANVAS_TYPE_CANVAS_ELLIPSE            (gtk_canvas_ellipse_get_type ())
#define GTK_CANVAS_ELLIPSE(obj)            (GTK_CHECK_CAST ((obj), GTK_CANVAS_TYPE_CANVAS_ELLIPSE, GtkCanvasEllipse))
#define GTK_CANVAS_ELLIPSE_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GTK_CANVAS_TYPE_CANVAS_ELLIPSE, GtkCanvasEllipseClass))
#define GTK_CANVAS_IS_CANVAS_ELLIPSE(obj)         (GTK_CHECK_TYPE ((obj), GTK_CANVAS_TYPE_CANVAS_ELLIPSE))
#define GTK_CANVAS_IS_CANVAS_ELLIPSE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_CANVAS_TYPE_CANVAS_ELLIPSE))


typedef struct _GtkCanvasEllipse GtkCanvasEllipse;
typedef struct _GtkCanvasEllipseClass GtkCanvasEllipseClass;

struct _GtkCanvasEllipse {
	GtkCanvasRE re;
};

struct _GtkCanvasEllipseClass {
	GtkCanvasREClass parent_class;
};


/* Standard Gtk function */
GtkType gtk_canvas_ellipse_get_type (void);


END_GTK_CANVAS_DECLS

#endif
