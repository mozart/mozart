/* Polygon item type for GtkCanvas widget
 *
 * GtkCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 * Copyright (C) 1998 The Free Software Foundation
 *
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 */

#ifndef GTK_CANVAS_POLYGON_H
#define GTK_CANVAS_POLYGON_H

#include <gtk-canvas/gtk-canvas-defs.h>
#include "gtk-canvas/gtk-canvas.h"

BEGIN_GTK_CANVAS_DECLS


/* Polygon item for the canvas.  A polygon is a bit different from rectangles and ellipses in that
 * points inside it will always be considered "inside", even if the fill color is not set.  If you
 * want to have a hollow polygon, use a line item instead.
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
 * fill_color		string			W		X color specification for fill color,
 *								or NULL pointer for no color (transparent).
 * fill_color_gdk	GdkColor*		RW		Allocated GdkColor for fill.
 * outline_color	string			W		X color specification for outline color,
 *								or NULL pointer for no color (transparent).
 * outline_color_gdk	GdkColor*		RW		Allocated GdkColor for outline.
 * fill_stipple		GdkBitmap*		RW		Stipple pattern for fill
 * outline_stipple	GdkBitmap*		RW		Stipple pattern for outline
 * width_pixels		uint			RW		Width of the outline in pixels.  The outline will
 *								not be scaled when the canvas zoom factor is changed.
 * width_units		double			RW		Width of the outline in canvas units.  The outline
 *								will be scaled when the canvas zoom factor is changed.
 */

#define GTK_CANVAS_TYPE_CANVAS_POLYGON            (gtk_canvas_polygon_get_type ())
#define GTK_CANVAS_POLYGON(obj)            (GTK_CHECK_CAST ((obj), GTK_CANVAS_TYPE_CANVAS_POLYGON, GtkCanvasPolygon))
#define GTK_CANVAS_POLYGON_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GTK_CANVAS_TYPE_CANVAS_POLYGON, GtkCanvasPolygonClass))
#define GTK_CANVAS_IS_CANVAS_POLYGON(obj)         (GTK_CHECK_TYPE ((obj), GTK_CANVAS_TYPE_CANVAS_POLYGON))
#define GTK_CANVAS_IS_CANVAS_POLYGON_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_CANVAS_TYPE_CANVAS_POLYGON))


typedef struct _GtkCanvasPolygon GtkCanvasPolygon;
typedef struct _GtkCanvasPolygonClass GtkCanvasPolygonClass;

struct _GtkCanvasPolygon {
	GtkCanvasItem item;

	int num_points;			/* Number of points in the polygon */
	double *coords;			/* Array of coordinates for the polygon's points.  X coords
					 * are in the even indices, Y coords are in the odd indices.
					 */

	double width;			/* Width of polygon's outline */

	guint fill_color;		/* Fill color, RGBA */
	guint outline_color;		/* Outline color, RGBA */

	gulong fill_pixel;		/* Color for fill */
	gulong outline_pixel;		/* Color for outline */

	GdkBitmap *fill_stipple;	/* Stipple for fill */
	GdkBitmap *outline_stipple;	/* Stipple for outline */

	GdkGC *fill_gc;			/* GC for filling */
	GdkGC *outline_gc;		/* GC for outline */

	guint fill_set : 1;		/* Is fill color set? */
	guint outline_set : 1;		/* Is outline color set? */
	guint width_pixels : 1;		/* Is outline width specified in pixels or units? */

	/* Antialiased specific stuff follows */
	guint32 fill_rgba;		/* RGBA color for filling */
	ArtSVP *fill_svp;		/* The SVP for the filled shape */
	guint32 outline_rgba;		/* RGBA color for outline */
	ArtSVP *outline_svp;		/* The SVP for the outline shape */
};

struct _GtkCanvasPolygonClass {
	GtkCanvasItemClass parent_class;
};


/* Standard Gtk function */
GtkType gtk_canvas_polygon_get_type (void);


END_GTK_CANVAS_DECLS

#endif
