/* Line/curve item type for GtkCanvas widget
 *
 * GtkCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 * Copyright (C) 1998 The Free Software Foundation
 *
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 */

#include <config.h>
#include <math.h>
#include <string.h>
#include "libart_lgpl/art_vpath.h"
#include "libart_lgpl/art_svp.h"
#include "libart_lgpl/art_svp_vpath.h"
#include "libart_lgpl/art_svp_vpath_stroke.h"
#include "gtk-canvas-line.h"
#include "gtk-canvas-util.h"
#include "gtk-canvastypebuiltins.h"

#define noVERBOSE

#define DEFAULT_SPLINE_STEPS 12		/* this is what Tk uses */
#define NUM_ARROW_POINTS     6		/* number of points in an arrowhead */
#define NUM_STATIC_POINTS    256	/* number of static points to use to avoid allocating arrays */


#define GROW_BOUNDS(bx1, by1, bx2, by2, x, y) {	\
	if (x < bx1)				\
		bx1 = x;			\
						\
	if (x > bx2)				\
		bx2 = x;			\
						\
	if (y < by1)				\
		by1 = y;			\
						\
	if (y > by2)				\
		by2 = y;			\
}


enum {
	ARG_0,
	ARG_POINTS,
	ARG_FILL_COLOR,
	ARG_FILL_COLOR_GDK,
	ARG_FILL_COLOR_RGBA,
	ARG_FILL_STIPPLE,
	ARG_WIDTH_PIXELS,
	ARG_WIDTH_UNITS,
	ARG_CAP_STYLE,
	ARG_JOIN_STYLE,
	ARG_LINE_STYLE,
	ARG_FIRST_ARROWHEAD,
	ARG_LAST_ARROWHEAD,
	ARG_SMOOTH,
	ARG_SPLINE_STEPS,
	ARG_ARROW_SHAPE_A,
	ARG_ARROW_SHAPE_B,
	ARG_ARROW_SHAPE_C
};


static void gtk_canvas_line_class_init (GtkCanvasLineClass *class);
static void gtk_canvas_line_init       (GtkCanvasLine      *line);
static void gtk_canvas_line_destroy    (GtkObject            *object);
static void gtk_canvas_line_set_arg    (GtkObject            *object,
					  GtkArg               *arg,
					  guint                 arg_id);
static void gtk_canvas_line_get_arg    (GtkObject            *object,
					  GtkArg               *arg,
					  guint                 arg_id);

static void   gtk_canvas_line_update      (GtkCanvasItem *item, double *affine, ArtSVP *clip_path, int flags);
static void   gtk_canvas_line_realize     (GtkCanvasItem *item);
static void   gtk_canvas_line_unrealize   (GtkCanvasItem *item);
static void   gtk_canvas_line_draw        (GtkCanvasItem *item, GdkDrawable *drawable,
					     int x, int y, int width, int height);
static double gtk_canvas_line_point       (GtkCanvasItem *item, double x, double y,
					     int cx, int cy, GtkCanvasItem **actual_item);
static void   gtk_canvas_line_translate   (GtkCanvasItem *item, double dx, double dy);
static void   gtk_canvas_line_bounds      (GtkCanvasItem *item, double *x1, double *y1, double *x2, double *y2);
static void   gtk_canvas_line_render      (GtkCanvasItem *item, GtkCanvasBuf *buf);


static GtkCanvasItemClass *parent_class;


GtkType
gtk_canvas_line_get_type (void)
{
	static GtkType line_type = 0;

	if (!line_type) {
		GtkTypeInfo line_info = {
			"GtkCanvasLine",
			sizeof (GtkCanvasLine),
			sizeof (GtkCanvasLineClass),
			(GtkClassInitFunc) gtk_canvas_line_class_init,
			(GtkObjectInitFunc) gtk_canvas_line_init,
			NULL, /* reserved_1 */
			NULL, /* reserved_2 */
			(GtkClassInitFunc) NULL
		};

		line_type = gtk_type_unique (gtk_canvas_item_get_type (), &line_info);
	}

	return line_type;
}

static void
gtk_canvas_line_class_init (GtkCanvasLineClass *class)
{
	GtkObjectClass *object_class;
	GtkCanvasItemClass *item_class;

	object_class = (GtkObjectClass *) class;
	item_class = (GtkCanvasItemClass *) class;

	parent_class = gtk_type_class (gtk_canvas_item_get_type ());

	gtk_object_add_arg_type ("GtkCanvasLine::points", GTK_TYPE_CANVAS_POINTS, GTK_ARG_READWRITE, ARG_POINTS);
	gtk_object_add_arg_type ("GtkCanvasLine::fill_color", GTK_TYPE_STRING, GTK_ARG_WRITABLE, ARG_FILL_COLOR);
	gtk_object_add_arg_type ("GtkCanvasLine::fill_color_gdk", GTK_TYPE_GDK_COLOR, GTK_ARG_READWRITE, ARG_FILL_COLOR_GDK);
	gtk_object_add_arg_type ("GtkCanvasLine::fill_color_rgba", GTK_TYPE_UINT, GTK_ARG_READWRITE, ARG_FILL_COLOR_RGBA);
	gtk_object_add_arg_type ("GtkCanvasLine::fill_stipple", GTK_TYPE_GDK_WINDOW, GTK_ARG_READWRITE, ARG_FILL_STIPPLE);
	gtk_object_add_arg_type ("GtkCanvasLine::width_pixels", GTK_TYPE_UINT, GTK_ARG_WRITABLE, ARG_WIDTH_PIXELS);
	gtk_object_add_arg_type ("GtkCanvasLine::width_units", GTK_TYPE_DOUBLE, GTK_ARG_WRITABLE, ARG_WIDTH_UNITS);
	gtk_object_add_arg_type ("GtkCanvasLine::cap_style", GTK_TYPE_GDK_CAP_STYLE, GTK_ARG_READWRITE, ARG_CAP_STYLE);
	gtk_object_add_arg_type ("GtkCanvasLine::join_style", GTK_TYPE_GDK_JOIN_STYLE, GTK_ARG_READWRITE, ARG_JOIN_STYLE);
	gtk_object_add_arg_type ("GtkCanvasLine::line_style", GTK_TYPE_GDK_LINE_STYLE, GTK_ARG_READWRITE, ARG_LINE_STYLE);
	gtk_object_add_arg_type ("GtkCanvasLine::first_arrowhead", GTK_TYPE_BOOL, GTK_ARG_READWRITE, ARG_FIRST_ARROWHEAD);
	gtk_object_add_arg_type ("GtkCanvasLine::last_arrowhead", GTK_TYPE_BOOL, GTK_ARG_READWRITE, ARG_LAST_ARROWHEAD);
	gtk_object_add_arg_type ("GtkCanvasLine::smooth", GTK_TYPE_BOOL, GTK_ARG_READWRITE, ARG_SMOOTH);
	gtk_object_add_arg_type ("GtkCanvasLine::spline_steps", GTK_TYPE_UINT, GTK_ARG_READWRITE, ARG_SPLINE_STEPS);
	gtk_object_add_arg_type ("GtkCanvasLine::arrow_shape_a", GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_ARROW_SHAPE_A);
	gtk_object_add_arg_type ("GtkCanvasLine::arrow_shape_b", GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_ARROW_SHAPE_B);
	gtk_object_add_arg_type ("GtkCanvasLine::arrow_shape_c", GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_ARROW_SHAPE_C);

	object_class->destroy = gtk_canvas_line_destroy;
	object_class->set_arg = gtk_canvas_line_set_arg;
	object_class->get_arg = gtk_canvas_line_get_arg;

	item_class->update = gtk_canvas_line_update;
	item_class->realize = gtk_canvas_line_realize;
	item_class->unrealize = gtk_canvas_line_unrealize;
	item_class->draw = gtk_canvas_line_draw;
	item_class->point = gtk_canvas_line_point;
	item_class->translate = gtk_canvas_line_translate;
	item_class->bounds = gtk_canvas_line_bounds;

	item_class->render = gtk_canvas_line_render;
}

static void
gtk_canvas_line_init (GtkCanvasLine *line)
{
	line->width = 0.0;
	line->cap = GDK_CAP_BUTT;
	line->join = GDK_JOIN_MITER;
	line->line_style = GDK_LINE_SOLID;
	line->shape_a = 0.0;
	line->shape_b = 0.0;
	line->shape_c = 0.0;
	line->spline_steps = DEFAULT_SPLINE_STEPS;
}

static void
gtk_canvas_line_destroy (GtkObject *object)
{
	GtkCanvasLine *line;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_LINE (object));

	line = GTK_CANVAS_LINE (object);

	if (line->coords)
		g_free (line->coords);

	if (line->first_coords)
		g_free (line->first_coords);

	if (line->last_coords)
		g_free (line->last_coords);

	if (line->stipple)
		gdk_bitmap_unref (line->stipple);

	if (line->fill_svp)
		art_svp_free (line->fill_svp);

	if (line->first_svp)
		art_svp_free (line->first_svp);

	if (line->last_svp)
		art_svp_free (line->last_svp);

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

/* Computes the bounding box of the line, including its arrow points.  Assumes that the number of
 * points in the line is not zero.
 */
static void
get_bounds (GtkCanvasLine *line, double *bx1, double *by1, double *bx2, double *by2)
{
	double *coords;
	double x1, y1, x2, y2;
	double width;
	int i;

	/* Find bounding box of line's points */

	x1 = x2 = line->coords[0];
	y1 = y2 = line->coords[1];

	for (i = 1, coords = line->coords + 2; i < line->num_points; i++, coords += 2)
		GROW_BOUNDS (x1, y1, x2, y2, coords[0], coords[1]);

	/* Add possible over-estimate for wide lines */

	if (line->width_pixels)
		width = line->width / line->item.canvas->pixels_per_unit;
	else
		width = line->width;

	x1 -= width;
	y1 -= width;
	x2 += width;
	y2 += width;

	/* For mitered lines, make a second pass through all the points.  Compute the location of
	 * the two miter vertex points and add them to the bounding box.
	 */

	if (line->join == GDK_JOIN_MITER)
		for (i = line->num_points, coords = line->coords; i >= 3; i--, coords += 2) {
			double mx1, my1, mx2, my2;

			if (gtk_canvas_get_miter_points (coords[0], coords[1],
							   coords[2], coords[3],
							   coords[4], coords[5],
							   width,
							   &mx1, &my1, &mx2, &my2)) {
				GROW_BOUNDS (x1, y1, x2, y2, mx1, my1);
				GROW_BOUNDS (x1, y1, x2, y2, mx2, my2);
			}
		}

	/* Add the arrow points, if any */

	if (line->first_arrow && line->first_coords)
		for (i = 0, coords = line->first_coords; i < NUM_ARROW_POINTS; i++, coords += 2)
			GROW_BOUNDS (x1, y1, x2, y2, coords[0], coords[1]);

	if (line->last_arrow && line->last_coords)
		for (i = 0, coords = line->last_coords; i < NUM_ARROW_POINTS; i++, coords += 2)
			GROW_BOUNDS (x1, y1, x2, y2, coords[0], coords[1]);

	/* Done */

	*bx1 = x1;
	*by1 = y1;
	*bx2 = x2;
	*by2 = y2;
}

/* Computes the bounding box of the line, in canvas coordinates.  Assumes that the number of points in the polygon is
 * not zero. Affine is the i2c transformation.
 */
static void
get_bounds_canvas (GtkCanvasLine *line, double *bx1, double *by1, double *bx2, double *by2, double affine[6])
{
	GtkCanvasItem *item;

	/* It would be possible to tighten the bounds somewhat by transforming the individual points before
	   aggregating them into the bbox. But it hardly seems worth it. */
	ArtDRect bbox_world;
	ArtDRect bbox_canvas;

	item = GTK_CANVAS_ITEM (line);

	get_bounds (line, &bbox_world.x0, &bbox_world.y0, &bbox_world.x1, &bbox_world.y1);

	art_drect_affine_transform (&bbox_canvas, &bbox_world, affine);
	/* include 1 pixel of fudge */
	*bx1 = bbox_canvas.x0 - 1;
	*by1 = bbox_canvas.y0 - 1;
	*bx2 = bbox_canvas.x1 + 1;
	*by2 = bbox_canvas.y1 + 1;
}

static void
recalc_bounds (GtkCanvasLine *line)
{
	GtkCanvasItem *item;
	double x1, y1, x2, y2;
	int cx1, cx2, cy1, cy2;
	double dx, dy;

	item = GTK_CANVAS_ITEM (line);

	if (line->num_points == 0) {
		item->x1 = item->y1 = item->x2 = item->y2 = 0;
		return;
	}

	/* Get bounds in world coordinates */

	get_bounds (line, &x1, &y1, &x2, &y2);

	/* Convert to canvas pixel coords */

	dx = dy = 0.0;
	gtk_canvas_item_i2w (item, &dx, &dy);

	gtk_canvas_w2c (item->canvas, x1 + dx, y1 + dy, &cx1, &cy1);
	gtk_canvas_w2c (item->canvas, x2 + dx, y2 + dy, &cx2, &cy2);
	item->x1 = cx1;
	item->y1 = cy1;
	item->x2 = cx2;
	item->y2 = cy2;

	/* Some safety fudging */

	item->x1--;
	item->y1--;
	item->x2++;
	item->y2++;

	gtk_canvas_group_child_bounds (GTK_CANVAS_GROUP (item->parent), item);
}

/* Recalculates the arrow polygons for the line */
static void
reconfigure_arrows (GtkCanvasLine *line)
{
	double *poly, *coords;
	double dx, dy, length;
	double sin_theta, cos_theta, tmp;
	double frac_height;	/* Line width as fraction of arrowhead width */
	double backup;		/* Distance to backup end points so the line ends in the middle of the arrowhead */
	double vx, vy;		/* Position of arrowhead vertex */
	double shape_a, shape_b, shape_c;
	double width;
	int i;

	if (line->num_points == 0)
		return;

	/* Set up things */

	if (line->first_arrow) {
		if (line->first_coords) {
			line->coords[0] = line->first_coords[0];
			line->coords[1] = line->first_coords[1];
		} else
			line->first_coords = g_new (double, 2 * NUM_ARROW_POINTS);
	} else if (line->first_coords) {
		line->coords[0] = line->first_coords[0];
		line->coords[1] = line->first_coords[1];

		g_free (line->first_coords);
		line->first_coords = NULL;
	}

	i = 2 * (line->num_points - 1);

	if (line->last_arrow) {
		if (line->last_coords) {
			line->coords[i] = line->last_coords[0];
			line->coords[i + 1] = line->last_coords[1];
		} else
			line->last_coords = g_new (double, 2 * NUM_ARROW_POINTS);
	} else if (line->last_coords) {
		line->coords[i] = line->last_coords[0];
		line->coords[i + 1] = line->last_coords[1];

		g_free (line->last_coords);
		line->last_coords = NULL;
	}

	if (!line->first_arrow && !line->last_arrow)
		return;

	if (line->width_pixels)
		width = line->width / line->item.canvas->pixels_per_unit;
	else
		width = line->width;

	/* Add fudge value for better-looking results */

	shape_a = line->shape_a;
	shape_b = line->shape_b;
	shape_c = line->shape_c + width / 2.0;

	if (line->width_pixels) {
		shape_a /= line->item.canvas->pixels_per_unit;
		shape_b /= line->item.canvas->pixels_per_unit;
		shape_c /= line->item.canvas->pixels_per_unit;
	}

	shape_a += 0.001;
	shape_b += 0.001;
	shape_c += 0.001;

	/* Compute the polygon for the first arrowhead and adjust the first point in the line so
	 * that the line does not stick out past the leading edge of the arrowhead.
	 */

	frac_height = (line->width / 2.0) / shape_c;
	backup = frac_height * shape_b + shape_a * (1.0 - frac_height) / 2.0;

	if (line->first_arrow) {
		poly = line->first_coords;
		poly[0] = poly[10] = line->coords[0];
		poly[1] = poly[11] = line->coords[1];

		dx = poly[0] - line->coords[2];
		dy = poly[1] - line->coords[3];
		length = sqrt (dx * dx + dy * dy);
		if (length < GTK_CANVAS_EPSILON)
			sin_theta = cos_theta = 0.0;
		else {
			sin_theta = dy / length;
			cos_theta = dx / length;
		}

		vx = poly[0] - shape_a * cos_theta;
		vy = poly[1] - shape_a * sin_theta;

		tmp = shape_c * sin_theta;

		poly[2] = poly[0] - shape_b * cos_theta + tmp;
		poly[8] = poly[2] - 2.0 * tmp;

		tmp = shape_c * cos_theta;

		poly[3] = poly[1] - shape_b * sin_theta - tmp;
		poly[9] = poly[3] + 2.0 * tmp;

		poly[4] = poly[2] * frac_height + vx * (1.0 - frac_height);
		poly[5] = poly[3] * frac_height + vy * (1.0 - frac_height);
		poly[6] = poly[8] * frac_height + vx * (1.0 - frac_height);
		poly[7] = poly[9] * frac_height + vy * (1.0 - frac_height);

		/* Move the first point towards the second so that the corners at the end of the
		 * line are inside the arrowhead.
		 */

		line->coords[0] = poly[0] - backup * cos_theta;
		line->coords[1] = poly[1] - backup * sin_theta;
	}

	/* Same process for last arrowhead */

	if (line->last_arrow) {
		coords = line->coords + 2 * (line->num_points - 2);
		poly = line->last_coords;
		poly[0] = poly[10] = coords[2];
		poly[1] = poly[11] = coords[3];

		dx = poly[0] - coords[0];
		dy = poly[1] - coords[1];
		length = sqrt (dx * dx + dy * dy);
		if (length < GTK_CANVAS_EPSILON)
			sin_theta = cos_theta = 0.0;
		else {
			sin_theta = dy / length;
			cos_theta = dx / length;
		}

		vx = poly[0] - shape_a * cos_theta;
		vy = poly[1] - shape_a * sin_theta;

		tmp = shape_c * sin_theta;

		poly[2] = poly[0] - shape_b * cos_theta + tmp;
		poly[8] = poly[2] - 2.0 * tmp;

		tmp = shape_c * cos_theta;

		poly[3] = poly[1] - shape_b * sin_theta - tmp;
		poly[9] = poly[3] + 2.0 * tmp;

		poly[4] = poly[2] * frac_height + vx * (1.0 - frac_height);
		poly[5] = poly[3] * frac_height + vy * (1.0 - frac_height);
		poly[6] = poly[8] * frac_height + vx * (1.0 - frac_height);
		poly[7] = poly[9] * frac_height + vy * (1.0 - frac_height);

		coords[2] = poly[0] - backup * cos_theta;
		coords[3] = poly[1] - backup * sin_theta;
	}
}

/* Convenience function to set the line's GC's foreground color */
static void
set_line_gc_foreground (GtkCanvasLine *line)
{
	GdkColor c;

	if (!line->gc)
		return;

	c.pixel = line->fill_pixel;
	gdk_gc_set_foreground (line->gc, &c);
}

/* Recalculate the line's width and set it in its GC */
static void
set_line_gc_width (GtkCanvasLine *line)
{
	int width;

	if (!line->gc)
		return;

	if (line->width_pixels)
		width = (int) line->width;
	else
		width = (int) (line->width * line->item.canvas->pixels_per_unit + 0.5);

	gdk_gc_set_line_attributes (line->gc,
				    width,
				    line->line_style,
				    (line->first_arrow || line->last_arrow) ? GDK_CAP_BUTT : line->cap,
				    line->join);
}

/* Sets the stipple pattern for the line */
static void
set_stipple (GtkCanvasLine *line, GdkBitmap *stipple, int reconfigure)
{
	if (line->stipple && !reconfigure)
		gdk_bitmap_unref (line->stipple);

	line->stipple = stipple;
	if (stipple && !reconfigure)
		gdk_bitmap_ref (stipple);

	if (line->gc) {
		if (stipple) {
			gdk_gc_set_stipple (line->gc, stipple);
			gdk_gc_set_fill (line->gc, GDK_STIPPLED);
		} else
			gdk_gc_set_fill (line->gc, GDK_SOLID);
	}
}

#ifdef OLD_XFORM
/* Convenience functions to recalculate the arrows and bounds of the line */
static void
reconfigure_arrows_and_bounds (GtkCanvasLine *line)
{
	reconfigure_arrows (line);
	recalc_bounds (line);
}
#endif

static void
gtk_canvas_line_set_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	GtkCanvasItem *item;
	GtkCanvasLine *line;
	GtkCanvasPoints *points;
	GdkColor color = { 0, 0, 0, 0, };
	GdkColor *pcolor;
	gboolean color_changed;
	int have_pixel;

	item = GTK_CANVAS_ITEM (object);
	line = GTK_CANVAS_LINE (object);

	color_changed = FALSE;
	have_pixel = FALSE;

	switch (arg_id) {
	case ARG_POINTS:
		points = GTK_VALUE_POINTER (*arg);

		if (line->coords) {
			g_free (line->coords);
			line->coords = NULL;
		}

		if (!points)
			line->num_points = 0;
		else {
			line->num_points = points->num_points;
			line->coords = g_new (double, 2 * line->num_points);
			memcpy (line->coords, points->coords, 2 * line->num_points * sizeof (double));
		}

		/* Drop the arrowhead polygons if they exist -- they will be regenerated */

		if (line->first_coords) {
			g_free (line->first_coords);
			line->first_coords = NULL;
		}

		if (line->last_coords) {
			g_free (line->last_coords);
			line->last_coords = NULL;
		}

		/* Since the line's points have changed, we need to re-generate arrowheads in
		 * addition to recalculating the bounds.
		 */

#ifdef OLD_XFORM
		reconfigure_arrows_and_bounds (line);
#else
		gtk_canvas_item_request_update (item);
#endif
		break;

	case ARG_FILL_COLOR:
		if (GTK_VALUE_STRING (*arg))
			gdk_color_parse (GTK_VALUE_STRING (*arg), &color);
		line->fill_rgba = ((color.red & 0xff00) << 16 |
				   (color.green & 0xff00) << 8 |
				   (color.blue & 0xff00) |
				   0xff);
		color_changed = TRUE;
		break;

	case ARG_FILL_COLOR_GDK:
		pcolor = GTK_VALUE_BOXED (*arg);
		if (pcolor) {
			color = *pcolor;
			gdk_color_context_query_color (item->canvas->cc, &color);
			have_pixel = TRUE;
		}

		line->fill_rgba = ((color.red & 0xff00) << 16 |
				   (color.green & 0xff00) << 8 |
				   (color.blue & 0xff00) |
				   0xff);
		color_changed = TRUE;
		break;

	case ARG_FILL_COLOR_RGBA:
		line->fill_rgba = GTK_VALUE_UINT (*arg);
		color_changed = TRUE;
		break;

	case ARG_FILL_STIPPLE:
		set_stipple (line, GTK_VALUE_BOXED (*arg), FALSE);
		gtk_canvas_item_request_redraw_svp (item, line->fill_svp);
		break;

	case ARG_WIDTH_PIXELS:
		line->width = GTK_VALUE_UINT (*arg);
		line->width_pixels = TRUE;
		set_line_gc_width (line);
#ifdef OLD_XFORM
		reconfigure_arrows_and_bounds (line);
#else
		gtk_canvas_item_request_update (item);
#endif
		break;

	case ARG_WIDTH_UNITS:
		line->width = fabs (GTK_VALUE_DOUBLE (*arg));
		line->width_pixels = FALSE;
		set_line_gc_width (line);
#ifdef OLD_XFORM
		reconfigure_arrows_and_bounds (line);
#else
		gtk_canvas_item_request_update (item);
#endif
		break;

	case ARG_CAP_STYLE:
		line->cap = GTK_VALUE_ENUM (*arg);
#ifdef OLD_XFORM
		reconfigure_arrows_and_bounds (line);
#else
		gtk_canvas_item_request_update (item);
#endif
		break;

	case ARG_JOIN_STYLE:
		line->join = GTK_VALUE_ENUM (*arg);
#ifdef OLD_XFORM
		reconfigure_arrows_and_bounds (line);
#else
		gtk_canvas_item_request_update (item);
#endif
		break;

	case ARG_LINE_STYLE:
		line->line_style = GTK_VALUE_ENUM (*arg);
		set_line_gc_width (line);
#ifdef OLD_XFORM
		reconfigure_arrows_and_bounds (line);
#else
		gtk_canvas_item_request_update (item);
#endif
		break;

	case ARG_FIRST_ARROWHEAD:
		line->first_arrow = GTK_VALUE_BOOL (*arg);
#ifdef OLD_XFORM
		reconfigure_arrows_and_bounds (line);
#else
		gtk_canvas_item_request_update (item);
#endif
		break;

	case ARG_LAST_ARROWHEAD:
		line->last_arrow = GTK_VALUE_BOOL (*arg);
#ifdef OLD_XFORM
		reconfigure_arrows_and_bounds (line);
#else
		gtk_canvas_item_request_update (item);
#endif
		break;

	case ARG_SMOOTH:
		/* FIXME */
		break;

	case ARG_SPLINE_STEPS:
		/* FIXME */
		break;

	case ARG_ARROW_SHAPE_A:
		line->shape_a = fabs (GTK_VALUE_DOUBLE (*arg));
#ifdef OLD_XFORM
		reconfigure_arrows_and_bounds (line);
#else
		gtk_canvas_item_request_update (item);
#endif
		break;

	case ARG_ARROW_SHAPE_B:
		line->shape_b = fabs (GTK_VALUE_DOUBLE (*arg));
#ifdef OLD_XFORM
		reconfigure_arrows_and_bounds (line);
#else
		gtk_canvas_item_request_update (item);
#endif
		break;

	case ARG_ARROW_SHAPE_C:
		line->shape_c = fabs (GTK_VALUE_DOUBLE (*arg));
#ifdef OLD_XFORM
		reconfigure_arrows_and_bounds (line);
#else
		gtk_canvas_item_request_update (item);
#endif
		break;

	default:
		break;
	}

	if (color_changed) {
		if (have_pixel)
			line->fill_pixel = color.pixel;
		else
			line->fill_pixel = gtk_canvas_get_color_pixel (item->canvas,
									 line->fill_rgba);

		if (!item->canvas->aa)
			set_line_gc_foreground (line);

		gtk_canvas_item_request_redraw_svp (item, line->fill_svp);
	}
}

/* Returns a copy of the line's points without the endpoint adjustments for
 * arrowheads.
 */
static GtkCanvasPoints *
get_points (GtkCanvasLine *line)
{
	GtkCanvasPoints *points;
	int start_ofs, end_ofs;

	if (line->num_points == 0)
		return NULL;

	start_ofs = end_ofs = 0;

	points = gtk_canvas_points_new (line->num_points);

	/* Invariant:  if first_coords or last_coords exist, then the line's
	 * endpoints have been adjusted.
	 */

	if (line->first_coords) {
		start_ofs = 1;

		points->coords[0] = line->first_coords[0];
		points->coords[1] = line->first_coords[1];
	}

	if (line->last_coords) {
		end_ofs = 1;

		points->coords[2 * (line->num_points - 1)] = line->last_coords[0];
		points->coords[2 * (line->num_points - 1) + 1] = line->last_coords[1];
	}

	memcpy (points->coords + 2 * start_ofs,
		line->coords + 2 * start_ofs,
		2 * (line->num_points - (start_ofs + end_ofs)) * sizeof (double));

	return points;
}

static void
gtk_canvas_line_get_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	GtkCanvasLine *line;
	GdkColor *color;

	line = GTK_CANVAS_LINE (object);

	switch (arg_id) {
	case ARG_POINTS:
		GTK_VALUE_POINTER (*arg) = get_points (line);
		break;

	case ARG_FILL_COLOR_GDK:
		color = g_new (GdkColor, 1);
		color->pixel = line->fill_pixel;
		gdk_color_context_query_color (line->item.canvas->cc, color);
		GTK_VALUE_BOXED (*arg) = color;
		break;

	case ARG_FILL_COLOR_RGBA:
		GTK_VALUE_UINT (*arg) = line->fill_rgba;
		break;

	case ARG_FILL_STIPPLE:
		GTK_VALUE_BOXED (*arg) = line->stipple;
		break;

	case ARG_CAP_STYLE:
		GTK_VALUE_ENUM (*arg) = line->cap;
		break;

	case ARG_JOIN_STYLE:
		GTK_VALUE_ENUM (*arg) = line->join;
		break;

	case ARG_LINE_STYLE:
		GTK_VALUE_ENUM (*arg) = line->line_style;
		break;

	case ARG_FIRST_ARROWHEAD:
		GTK_VALUE_BOOL (*arg) = line->first_arrow;
		break;

	case ARG_LAST_ARROWHEAD:
		GTK_VALUE_BOOL (*arg) = line->last_arrow;
		break;

	case ARG_SMOOTH:
		GTK_VALUE_BOOL (*arg) = line->smooth;
		break;

	case ARG_SPLINE_STEPS:
		GTK_VALUE_UINT (*arg) = line->spline_steps;
		break;

	case ARG_ARROW_SHAPE_A:
		GTK_VALUE_DOUBLE (*arg) = line->shape_a;
		break;

	case ARG_ARROW_SHAPE_B:
		GTK_VALUE_DOUBLE (*arg) = line->shape_b;
		break;

	case ARG_ARROW_SHAPE_C:
		GTK_VALUE_DOUBLE (*arg) = line->shape_c;
		break;

	default:
		arg->type = GTK_TYPE_INVALID;
		break;
	}
}

static void
gtk_canvas_line_render (GtkCanvasItem *item,
			     GtkCanvasBuf *buf)
{
	GtkCanvasLine *line;

	line = GTK_CANVAS_LINE (item);

	if (line->fill_svp != NULL)
		gtk_canvas_render_svp (buf, line->fill_svp, line->fill_rgba);

	if (line->first_svp != NULL)
		gtk_canvas_render_svp (buf, line->first_svp, line->fill_rgba);

	if (line->last_svp != NULL)
		gtk_canvas_render_svp (buf, line->last_svp, line->fill_rgba);
}


static ArtSVP *
svp_from_points (const double *item_coords, int num_points, const double affine[6])
{
	ArtVpath *vpath;
	ArtSVP *svp;
	double x, y;
	int i;

	vpath = art_new (ArtVpath, num_points + 2);

	for (i = 0; i < num_points; i++) {
		vpath[i].code = i == 0 ? ART_MOVETO : ART_LINETO;
		x = item_coords[i * 2];
		y = item_coords[i * 2 + 1];
		vpath[i].x = x * affine[0] + y * affine[2] + affine[4];
		vpath[i].y = x * affine[1] + y * affine[3] + affine[5];
	}
#if 0
	vpath[i].code = ART_LINETO;
	vpath[i].x = vpath[0].x;
	vpath[i].y = vpath[0].y;
	i++;
#endif
	vpath[i].code = ART_END;
	vpath[i].x = 0;
	vpath[i].y = 0;

	svp = art_svp_from_vpath (vpath);

	art_free (vpath);

	return svp;
}

static void
gtk_canvas_line_update (GtkCanvasItem *item, double *affine, ArtSVP *clip_path, int flags)
{
	GtkCanvasLine *line;
	int i;
	ArtVpath *vpath;
	ArtPoint pi, pc;
	double width;
	ArtSVP *svp;
	double x1, y1, x2, y2;

	line = GTK_CANVAS_LINE (item);

	if (parent_class->update)
		(* parent_class->update) (item, affine, clip_path, flags);

	reconfigure_arrows (line);

	if (item->canvas->aa) {
		gtk_canvas_item_reset_bounds (item);

		vpath = art_new (ArtVpath, line->num_points + 2);

		for (i = 0; i < line->num_points; i++) {
			pi.x = line->coords[i * 2];
			pi.y = line->coords[i * 2 + 1];
			art_affine_point (&pc, &pi, affine);
			vpath[i].code = i == 0 ? ART_MOVETO : ART_LINETO;
			vpath[i].x = pc.x;
			vpath[i].y = pc.y;
		}
		vpath[i].code = ART_END;
		vpath[i].x = 0;
		vpath[i].y = 0;

		if (line->width_pixels)
			width = line->width;
		else
			width = line->width * art_affine_expansion (affine);

		if (width < 0.5)
			width = 0.5;

		svp = art_svp_vpath_stroke (vpath,
					    gtk_canvas_join_gdk_to_art (line->join),
					    gtk_canvas_cap_gdk_to_art (line->cap),
					    width,
					    4,
					    0.25);
		art_free (vpath);

		gtk_canvas_item_update_svp_clip (item, &line->fill_svp, svp, clip_path);

		if (line->first_arrow)
			svp = svp_from_points (line->first_coords, NUM_ARROW_POINTS, affine);
		else
			svp = NULL;

		gtk_canvas_item_update_svp_clip (item, &line->first_svp, svp, clip_path);

		if (line->last_arrow)
			svp = svp_from_points (line->last_coords, NUM_ARROW_POINTS, affine);
		else
			svp = NULL;

		gtk_canvas_item_update_svp_clip (item, &line->last_svp, svp, clip_path);

	} else {
		set_line_gc_foreground (line);
		set_line_gc_width (line);
		set_stipple (line, line->stipple, TRUE);

		get_bounds_canvas (line, &x1, &y1, &x2, &y2, affine);
		gtk_canvas_update_bbox (item, x1, y1, x2, y2);
	}

#ifdef OLD_XFORM
	reconfigure_arrows_and_bounds (line);
#endif
}

static void
gtk_canvas_line_realize (GtkCanvasItem *item)
{
	GtkCanvasLine *line;

	line = GTK_CANVAS_LINE (item);

	if (parent_class->realize)
		(* parent_class->realize) (item);

	line->gc = gdk_gc_new (item->canvas->layout.bin_window);

#if 0
	(* GTK_CANVAS_ITEM_CLASS (item->object.klass)->update) (item, NULL, NULL, 0);
#endif
}

static void
gtk_canvas_line_unrealize (GtkCanvasItem *item)
{
	GtkCanvasLine *line;

	line = GTK_CANVAS_LINE (item);

	gdk_gc_unref (line->gc);
	line->gc = NULL;

	if (parent_class->unrealize)
		(* parent_class->unrealize) (item);
}

static void
item_to_canvas (GtkCanvas *canvas, double *item_coords, GdkPoint *canvas_coords, int num_points,
		int *num_drawn_points, double i2c[6])
{
	int i;
	int old_cx, old_cy;
	int cx, cy;
	ArtPoint pi, pc;

#ifdef VERBOSE
	{
		char str[128];
		art_affine_to_string (str, i2c);
		g_print ("line item_to_canvas %s\n", str);
	}
#endif

	/* the first point is always drawn */

	pi.x = item_coords[0];
	pi.y = item_coords[1];
	art_affine_point (&pc, &pi, i2c);
	cx = floor (pc.x + 0.5);
	cy = floor (pc.y + 0.5);
	canvas_coords->x = cx;
	canvas_coords->y = cy;
	canvas_coords++;
	old_cx = cx;
	old_cy = cy;
	*num_drawn_points = 1;

	for (i = 1; i < num_points; i++) {
		pi.x = item_coords[i * 2];
		pi.y = item_coords[i * 2 + 1];
		art_affine_point (&pc, &pi, i2c);
		cx = floor (pc.x + 0.5);
		cy = floor (pc.y + 0.5);
		if (old_cx != cx || old_cy != cy) {
			canvas_coords->x = cx;
			canvas_coords->y = cy;
			old_cx = cx;
			old_cy = cy;
			canvas_coords++;
			(*num_drawn_points)++;
		}
	}
}

static void
gtk_canvas_line_draw (GtkCanvasItem *item, GdkDrawable *drawable,
			int x, int y, int width, int height)
{
	GtkCanvasLine *line;
	GdkPoint static_points[NUM_STATIC_POINTS];
	GdkPoint *points;
	int actual_num_points_drawn;
	double i2c[6];

	line = GTK_CANVAS_LINE (item);

	if (line->num_points == 0)
		return;

	/* Build array of canvas pixel coordinates */

	if (line->num_points <= NUM_STATIC_POINTS)
		points = static_points;
	else
		points = g_new (GdkPoint, line->num_points);

	gtk_canvas_item_i2c_affine (item, i2c);

	i2c[4] -= x;
	i2c[5] -= y;

	item_to_canvas (item->canvas, line->coords, points, line->num_points,
			&actual_num_points_drawn, i2c);

	if (line->stipple)
		gtk_canvas_set_stipple_origin (item->canvas, line->gc);

	gdk_draw_lines (drawable, line->gc, points, actual_num_points_drawn);

	if (points != static_points)
		g_free (points);

	/* Draw arrowheads */

	points = static_points;

	if (line->first_arrow) {
		item_to_canvas (item->canvas, line->first_coords, points, NUM_ARROW_POINTS,
				&actual_num_points_drawn, i2c);
		gdk_draw_polygon (drawable, line->gc, TRUE, points, actual_num_points_drawn );
	}

	if (line->last_arrow) {
		item_to_canvas (item->canvas, line->last_coords, points, NUM_ARROW_POINTS,
				&actual_num_points_drawn, i2c);
		gdk_draw_polygon (drawable, line->gc, TRUE, points, actual_num_points_drawn );
	}
}

static double
gtk_canvas_line_point (GtkCanvasItem *item, double x, double y,
			 int cx, int cy, GtkCanvasItem **actual_item)
{
	GtkCanvasLine *line;
	double *line_points = NULL, *coords;
	double static_points[2 * NUM_STATIC_POINTS];
	double poly[10];
	double best, dist;
	double dx, dy;
	double width;
	int num_points = 0, i;
	int changed_miter_to_bevel;

#ifdef VERBOSE
	g_print ("gtk_canvas_line_point x, y = (%g, %g); cx, cy = (%d, %d)\n", x, y, cx, cy);
#endif

	line = GTK_CANVAS_LINE (item);

	*actual_item = item;

	best = 1.0e36;

	/* Handle smoothed lines by generating an expanded set ot points */

	if (line->smooth && (line->num_points > 2)) {
		/* FIXME */
	} else {
		num_points = line->num_points;
		line_points = line->coords;
	}

	/* Compute a polygon for each edge of the line and test the point against it.  The effective
	 * width of the line is adjusted so that it will be at least one pixel thick (so that zero
	 * pixel-wide lines can be pickedup as well).
	 */

	if (line->width_pixels)
		width = line->width / item->canvas->pixels_per_unit;
	else
		width = line->width;

	if (width < (1.0 / item->canvas->pixels_per_unit))
		width = 1.0 / item->canvas->pixels_per_unit;

	changed_miter_to_bevel = 0;

	for (i = num_points, coords = line_points; i >= 2; i--, coords += 2) {
		/* If rounding is done around the first point, then compute distance between the
		 * point and the first point.
		 */

		if (((line->cap == GDK_CAP_ROUND) && (i == num_points))
		    || ((line->join == GDK_JOIN_ROUND) && (i != num_points))) {
			dx = coords[0] - x;
			dy = coords[1] - y;
			dist = sqrt (dx * dx + dy * dy) - width / 2.0;
			if (dist < GTK_CANVAS_EPSILON) {
				best = 0.0;
				goto done;
			} else if (dist < best)
				best = dist;
		}

		/* Compute the polygonal shape corresponding to this edge, with two points for the
		 * first point of the edge and two points for the last point of the edge.
		 */

		if (i == num_points)
			gtk_canvas_get_butt_points (coords[2], coords[3], coords[0], coords[1],
						      width, (line->cap == GDK_CAP_PROJECTING),
						      poly, poly + 1, poly + 2, poly + 3);
		else if ((line->join == GDK_JOIN_MITER) && !changed_miter_to_bevel) {
			poly[0] = poly[6];
			poly[1] = poly[7];
			poly[2] = poly[4];
			poly[3] = poly[5];
		} else {
			gtk_canvas_get_butt_points (coords[2], coords[3], coords[0], coords[1],
						      width, FALSE,
						      poly, poly + 1, poly + 2, poly + 3);

			/* If this line uses beveled joints, then check the distance to a polygon
			 * comprising the last two points of the previous polygon and the first two
			 * from this polygon; this checks the wedges that fill the mitered point.
			 */

			if ((line->join == GDK_JOIN_BEVEL) || changed_miter_to_bevel) {
				poly[8] = poly[0];
				poly[9] = poly[1];

				dist = gtk_canvas_polygon_to_point (poly, 5, x, y);
				if (dist < GTK_CANVAS_EPSILON) {
					best = 0.0;
					goto done;
				} else if (dist < best)
					best = dist;

				changed_miter_to_bevel = FALSE;
			}
		}

		if (i == 2)
			gtk_canvas_get_butt_points (coords[0], coords[1], coords[2], coords[3],
						      width, (line->cap == GDK_CAP_PROJECTING),
						      poly + 4, poly + 5, poly + 6, poly + 7);
		else if (line->join == GDK_JOIN_MITER) {
			if (!gtk_canvas_get_miter_points (coords[0], coords[1],
							    coords[2], coords[3],
							    coords[4], coords[5],
							    width,
							    poly + 4, poly + 5, poly + 6, poly + 7)) {
				changed_miter_to_bevel = TRUE;
				gtk_canvas_get_butt_points (coords[0], coords[1], coords[2], coords[3],
							      width, FALSE,
							      poly + 4, poly + 5, poly + 6, poly + 7);
			}
		} else
			gtk_canvas_get_butt_points (coords[0], coords[1], coords[2], coords[3],
						      width, FALSE,
						      poly + 4, poly + 5, poly + 6, poly + 7);

		poly[8] = poly[0];
		poly[9] = poly[1];

		dist = gtk_canvas_polygon_to_point (poly, 5, x, y);
		if (dist < GTK_CANVAS_EPSILON) {
			best = 0.0;
			goto done;
		} else if (dist < best)
			best = dist;
	}

	/* If caps are rounded, check the distance to the cap around the final end point of the line */

	if (line->cap == GDK_CAP_ROUND) {
		dx = coords[0] - x;
		dy = coords[1] - y;
		dist = sqrt (dx * dx + dy * dy) - width / 2.0;
		if (dist < GTK_CANVAS_EPSILON) {
			best = 0.0;
			goto done;
		} else
			best = dist;
	}

	/* sometimes the GtkCanvasItem::update signal will not have
           been processed between deleting the arrow points and a call
           to this routine -- this can cause a segfault here */
	if ((line->first_arrow && !line->first_coords) ||
	    (line->last_arrow && !line->last_coords))
		reconfigure_arrows(line);

	/* If there are arrowheads, check the distance to them */

	if (line->first_arrow) {
		dist = gtk_canvas_polygon_to_point (line->first_coords, NUM_ARROW_POINTS, x, y);
		if (dist < GTK_CANVAS_EPSILON) {
			best = 0.0;
			goto done;
		} else
			best = dist;
	}

	if (line->last_arrow) {
		dist = gtk_canvas_polygon_to_point (line->last_coords, NUM_ARROW_POINTS, x, y);
		if (dist < GTK_CANVAS_EPSILON) {
			best = 0.0;
			goto done;
		} else
			best = dist;
	}

done:

	if ((line_points != static_points) && (line_points != line->coords))
		g_free (line_points);

	return best;
}

static void
gtk_canvas_line_translate (GtkCanvasItem *item, double dx, double dy)
{
	GtkCanvasLine *line;
	int i;
	double *coords;

	line = GTK_CANVAS_LINE (item);

	for (i = 0, coords = line->coords; i < line->num_points; i++, coords += 2) {
		coords[0] += dx;
		coords[1] += dy;
	}

	if (line->first_arrow)
		for (i = 0, coords = line->first_coords; i < NUM_ARROW_POINTS; i++, coords += 2) {
			coords[0] += dx;
			coords[1] += dy;
		}

	if (line->last_arrow)
		for (i = 0, coords = line->last_coords; i < NUM_ARROW_POINTS; i++, coords += 2) {
			coords[0] += dx;
			coords[1] += dy;
		}

	recalc_bounds (line);
}

static void
gtk_canvas_line_bounds (GtkCanvasItem *item, double *x1, double *y1, double *x2, double *y2)
{
	GtkCanvasLine *line;

	line = GTK_CANVAS_LINE (item);

	if (line->num_points == 0) {
		*x1 = *y1 = *x2 = *y2 = 0.0;
		return;
	}

	get_bounds (line, x1, y1, x2, y2);
}
