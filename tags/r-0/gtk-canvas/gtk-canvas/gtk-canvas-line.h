/* Line/curve item type for GtkCanvas widget
 *
 * GtkCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 * Copyright (C) 1998 The Free Software Foundation
 *
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 */

#ifndef GTK_CANVAS_LINE_H
#define GTK_CANVAS_LINE_H

#include <gtk-canvas/gtk-canvas-defs.h>
#include "gtk-canvas/gtk-canvas.h"


BEGIN_GTK_CANVAS_DECLS


/* Line item for the canvas.  This is a polyline with configurable width, cap/join styles, and arrowheads.
 * If arrowheads are enabled, then three values are used to specify their shape:
 *
 *	arrow_shape_a:  Distance from tip of arrowhead to the center point.
 *	arrow_shape_b:  Distance from tip of arrowhead to trailing point, measured along the shaft.
 *	arrow_shape_c:	Distance of trailing point from outside edge of shaft.
 *
 * The following object arguments are available:
 *
 * name			type			read/write	description
 * ------------------------------------------------------------------------------------------
 * points		GtkCanvasPoints*	RW		Pointer to a GtkCanvasPoints structure.
 *								This can be created by a call to
 *								gtk_canvas_points_new() (in gtk-canvas-util.h).
 *								X coordinates are in the even indices of the
 *								points->coords array, Y coordinates are in
 *								the odd indices.
 * fill_color		string			W		X color specification for line
 * fill_color_gdk	GdkColor*		RW		Pointer to an allocated GdkColor
 * fill_stipple		GdkBitmap*		RW		Stipple pattern for the line
 * width_pixels		uint			R		Width of the line in pixels.  The line width
 *								will not be scaled when the canvas zoom factor changes.
 * width_units		double			R		Width of the line in canvas units.  The line width
 *								will be scaled when the canvas zoom factor changes.
 * cap_style		GdkCapStyle		RW		Cap ("endpoint") style for the line.
 * join_style		GdkJoinStyle		RW		Join ("vertex") style for the line.
 * line_style		GdkLineStyle		RW		Line dash style
 * first_arrowhead	boolean			RW		Specifies whether to draw an arrowhead on the
 *								first point of the line.
 * last_arrowhead	boolean			RW		Specifies whether to draw an arrowhead on the
 *								last point of the line.
 * smooth		boolean			RW		Specifies whether to smooth the line using
 *								parabolic splines.
 * spline_steps		uint			RW		Specifies the number of steps to use when rendering curves.
 * arrow_shape_a	double			RW		First arrow shape specifier.
 * arrow_shape_b	double			RW		Second arrow shape specifier.
 * arrow_shape_c	double			RW		Third arrow shape specifier.
 */


#define GTK_CANVAS_TYPE_CANVAS_LINE            (gtk_canvas_line_get_type ())
#define GTK_CANVAS_LINE(obj)            (GTK_CHECK_CAST ((obj), GTK_CANVAS_TYPE_CANVAS_LINE, GtkCanvasLine))
#define GTK_CANVAS_LINE_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GTK_CANVAS_TYPE_CANVAS_LINE, GtkCanvasLineClass))
#define GTK_CANVAS_IS_CANVAS_LINE(obj)         (GTK_CHECK_TYPE ((obj), GTK_CANVAS_TYPE_CANVAS_LINE))
#define GTK_CANVAS_IS_CANVAS_LINE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_CANVAS_TYPE_CANVAS_LINE))


typedef struct _GtkCanvasLine GtkCanvasLine;
typedef struct _GtkCanvasLineClass GtkCanvasLineClass;

struct _GtkCanvasLine {
	GtkCanvasItem item;

	int num_points;		/* Number of points in the line */
	double *coords;		/* Array of coordinates for the line's points.  X coords are in the
				 * even indices, Y coords are in the odd indices.  If the line has
				 * arrowheads then the first and last points have been adjusted to
				 * refer to the necks of the arrowheads rather than their tips.  The
				 * actual endpoints are stored in the first_arrow and last_arrow
				 * arrays, if they exist.
				 */

	double width;		/* Width of the line */

	guint fill_color;	/* Fill color, RGBA */

	gulong fill_pixel;	/* Color for line */

	GdkBitmap *stipple;	/* Stipple pattern */

	GdkCapStyle cap;	/* Cap style for line */
	GdkJoinStyle join;	/* Join style for line */
	GdkLineStyle line_style;/* Style for the line */

	double shape_a;		/* Distance from tip of arrowhead to center */
	double shape_b;		/* Distance from tip of arrowhead to trailing point, measured along shaft */
	double shape_c;		/* Distance of trailing points from outside edge of shaft */

	double *first_coords;	/* Array of points describing polygon for the first arrowhead */
	double *last_coords;	/* Array of points describing polygon for the last arrowhead */

	int spline_steps;	/* Number of steps in each spline segment */

	GdkGC *gc;		/* GC for drawing line */

	guint width_pixels : 1;	/* Is the width specified in pixels or units? */
	guint first_arrow : 1;	/* Draw first arrowhead? */
	guint last_arrow : 1;	/* Draw last arrowhead? */
	guint smooth : 1;	/* Smooth line (with parabolic splines)? */

	/* Antialiased specific stuff follows */
	guint32 fill_rgba;		/* RGBA color for outline */
	ArtSVP *fill_svp;		/* The SVP for the outline shape */
	ArtSVP *first_svp;		/* The SVP for the first arrow */
	ArtSVP *last_svp;		/* The SVP for the last arrow */
};

struct _GtkCanvasLineClass {
	GtkCanvasItemClass parent_class;
};


/* Standard Gtk function */
GtkType gtk_canvas_line_get_type (void);


END_GTK_CANVAS_DECLS

#endif
