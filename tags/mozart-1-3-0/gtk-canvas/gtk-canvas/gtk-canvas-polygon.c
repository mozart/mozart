/* Polygon item type for GtkCanvas widget
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
#include "gtk-canvas-polygon.h"
#include "gtk-canvas-util.h"
#include "gtk-canvastypebuiltins.h"


#define NUM_STATIC_POINTS 256	/* Number of static points to use to avoid allocating arrays */


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
	ARG_OUTLINE_COLOR,
	ARG_OUTLINE_COLOR_GDK,
	ARG_OUTLINE_COLOR_RGBA,
	ARG_FILL_STIPPLE,
	ARG_OUTLINE_STIPPLE,
	ARG_WIDTH_PIXELS,
	ARG_WIDTH_UNITS
};


static void gtk_canvas_polygon_class_init (GtkCanvasPolygonClass *class);
static void gtk_canvas_polygon_init       (GtkCanvasPolygon      *poly);
static void gtk_canvas_polygon_destroy    (GtkObject               *object);
static void gtk_canvas_polygon_set_arg    (GtkObject               *object,
					     GtkArg                  *arg,
					     guint                    arg_id);
static void gtk_canvas_polygon_get_arg    (GtkObject               *object,
					     GtkArg                  *arg,
					     guint                    arg_id);

static void   gtk_canvas_polygon_update      (GtkCanvasItem *item, double *affine, ArtSVP *clip_path, int flags);
static void   gtk_canvas_polygon_realize     (GtkCanvasItem *item);
static void   gtk_canvas_polygon_unrealize   (GtkCanvasItem *item);
static void   gtk_canvas_polygon_draw        (GtkCanvasItem *item, GdkDrawable *drawable,
						int x, int y, int width, int height);
static double gtk_canvas_polygon_point       (GtkCanvasItem *item, double x, double y,
						int cx, int cy, GtkCanvasItem **actual_item);
static void   gtk_canvas_polygon_translate   (GtkCanvasItem *item, double dx, double dy);
static void   gtk_canvas_polygon_bounds      (GtkCanvasItem *item, double *x1, double *y1, double *x2, double *y2);
static void   gtk_canvas_polygon_render      (GtkCanvasItem *item, GtkCanvasBuf *buf);


static GtkCanvasItemClass *parent_class;


GtkType
gtk_canvas_polygon_get_type (void)
{
	static GtkType polygon_type = 0;

	if (!polygon_type) {
		GtkTypeInfo polygon_info = {
			"GtkCanvasPolygon",
			sizeof (GtkCanvasPolygon),
			sizeof (GtkCanvasPolygonClass),
			(GtkClassInitFunc) gtk_canvas_polygon_class_init,
			(GtkObjectInitFunc) gtk_canvas_polygon_init,
			NULL, /* reserved_1 */
			NULL, /* reserved_2 */
			(GtkClassInitFunc) NULL
		};

		polygon_type = gtk_type_unique (gtk_canvas_item_get_type (), &polygon_info);
	}

	return polygon_type;
}

static void
gtk_canvas_polygon_class_init (GtkCanvasPolygonClass *class)
{
	GtkObjectClass *object_class;
	GtkCanvasItemClass *item_class;

	object_class = (GtkObjectClass *) class;
	item_class = (GtkCanvasItemClass *) class;

	parent_class = gtk_type_class (gtk_canvas_item_get_type ());

	gtk_object_add_arg_type ("GtkCanvasPolygon::points", GTK_TYPE_CANVAS_POINTS, GTK_ARG_READWRITE, ARG_POINTS);
	gtk_object_add_arg_type ("GtkCanvasPolygon::fill_color", GTK_TYPE_STRING, GTK_ARG_WRITABLE, ARG_FILL_COLOR);
	gtk_object_add_arg_type ("GtkCanvasPolygon::fill_color_gdk", GTK_TYPE_GDK_COLOR, GTK_ARG_READWRITE, ARG_FILL_COLOR_GDK);
	gtk_object_add_arg_type ("GtkCanvasPolygon::fill_color_rgba", GTK_TYPE_UINT, GTK_ARG_READWRITE, ARG_FILL_COLOR_RGBA);
	gtk_object_add_arg_type ("GtkCanvasPolygon::outline_color", GTK_TYPE_STRING, GTK_ARG_WRITABLE, ARG_OUTLINE_COLOR);
	gtk_object_add_arg_type ("GtkCanvasPolygon::outline_color_gdk", GTK_TYPE_GDK_COLOR, GTK_ARG_READWRITE, ARG_OUTLINE_COLOR_GDK);
	gtk_object_add_arg_type ("GtkCanvasPolygon::outline_color_rgba", GTK_TYPE_UINT, GTK_ARG_READWRITE, ARG_OUTLINE_COLOR_RGBA);
	gtk_object_add_arg_type ("GtkCanvasPolygon::fill_stipple", GTK_TYPE_GDK_WINDOW, GTK_ARG_READWRITE, ARG_FILL_STIPPLE);
	gtk_object_add_arg_type ("GtkCanvasPolygon::outline_stipple", GTK_TYPE_GDK_WINDOW, GTK_ARG_READWRITE, ARG_OUTLINE_STIPPLE);
	gtk_object_add_arg_type ("GtkCanvasPolygon::width_pixels", GTK_TYPE_UINT, GTK_ARG_WRITABLE, ARG_WIDTH_PIXELS);
	gtk_object_add_arg_type ("GtkCanvasPolygon::width_units", GTK_TYPE_DOUBLE, GTK_ARG_WRITABLE, ARG_WIDTH_UNITS);

	object_class->destroy = gtk_canvas_polygon_destroy;
	object_class->set_arg = gtk_canvas_polygon_set_arg;
	object_class->get_arg = gtk_canvas_polygon_get_arg;

	item_class->update = gtk_canvas_polygon_update;
	item_class->realize = gtk_canvas_polygon_realize;
	item_class->unrealize = gtk_canvas_polygon_unrealize;
	item_class->draw = gtk_canvas_polygon_draw;
	item_class->point = gtk_canvas_polygon_point;
	item_class->translate = gtk_canvas_polygon_translate;
	item_class->bounds = gtk_canvas_polygon_bounds;
	item_class->render = gtk_canvas_polygon_render;
}

static void
gtk_canvas_polygon_init (GtkCanvasPolygon *poly)
{
	poly->width = 0.0;
}

static void
gtk_canvas_polygon_destroy (GtkObject *object)
{
	GtkCanvasPolygon *poly;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_POLYGON (object));

	poly = GTK_CANVAS_POLYGON (object);

	if (poly->coords)
		g_free (poly->coords);

	if (poly->fill_stipple)
		gdk_bitmap_unref (poly->fill_stipple);

	if (poly->outline_stipple)
		gdk_bitmap_unref (poly->outline_stipple);

	if (poly->fill_svp)
		art_svp_free (poly->fill_svp);

	if (poly->outline_svp)
		art_svp_free (poly->outline_svp);

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

/* Computes the bounding box of the polygon.  Assumes that the number of points in the polygon is
 * not zero.
 */
static void
get_bounds (GtkCanvasPolygon *poly, double *bx1, double *by1, double *bx2, double *by2)
{
	double *coords;
	double x1, y1, x2, y2;
	double width;
	int i;

	/* Compute bounds of vertices */

	x1 = x2 = poly->coords[0];
	y1 = y2 = poly->coords[1];

	for (i = 1, coords = poly->coords + 2; i < poly->num_points; i++, coords += 2) {
		GROW_BOUNDS (x1, y1, x2, y2, coords[0], coords[1]);
	}

	/* Add outline width */

	if (poly->width_pixels)
		width = poly->width / poly->item.canvas->pixels_per_unit;
	else
		width = poly->width;

	width /= 2.0;

	x1 -= width;
	y1 -= width;
	x2 += width;
	y2 += width;

	/* Done */

	*bx1 = x1;
	*by1 = y1;
	*bx2 = x2;
	*by2 = y2;
}

/* Computes the bounding box of the polygon, in canvas coordinates.  Assumes that the number of points in the polygon is
 * not zero.
 */
static void
get_bounds_canvas (GtkCanvasPolygon *poly, double *bx1, double *by1, double *bx2, double *by2, double affine[6])
{
	GtkCanvasItem *item;
	ArtDRect bbox_world;
	ArtDRect bbox_canvas;
	double i2w[6], w2c[6], i2c[6];

	item = GTK_CANVAS_ITEM (poly);

	get_bounds (poly, &bbox_world.x0, &bbox_world.y0, &bbox_world.x1, &bbox_world.y1);

	art_drect_affine_transform (&bbox_canvas, &bbox_world, affine);
	/* include 1 pixel of fudge */
	*bx1 = bbox_canvas.x0 - 1;
	*by1 = bbox_canvas.y0 - 1;
	*bx2 = bbox_canvas.x1 + 1;
	*by2 = bbox_canvas.y1 + 1;
}

/* Recalculates the canvas bounds for the polygon */
static void
recalc_bounds (GtkCanvasPolygon *poly)
{
	GtkCanvasItem *item;
	double x1, y1, x2, y2;
	int cx1, cy1, cx2, cy2;
	double dx, dy;

	item = GTK_CANVAS_ITEM (poly);

	if (poly->num_points == 0) {
		item->x1 = item->y1 = item->x2 = item->y2 = 0;
		return;
	}

	/* Get bounds in world coordinates */

	get_bounds (poly, &x1, &y1, &x2, &y2);

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

/* Sets the points of the polygon item to the specified ones.  If needed, it will add a point to
 * close the polygon.
 */
static void
set_points (GtkCanvasPolygon *poly, GtkCanvasPoints *points)
{
	int duplicate;

	/* See if we need to duplicate the first point */

	duplicate = ((points->coords[0] != points->coords[2 * points->num_points - 2])
		     || (points->coords[1] != points->coords[2 * points->num_points - 1]));

	if (duplicate)
		poly->num_points = points->num_points + 1;
	else
		poly->num_points = points->num_points;

	poly->coords = g_new (double, 2 * poly->num_points);
	memcpy (poly->coords, points->coords, 2 * points->num_points * sizeof (double));

	if (duplicate) {
		poly->coords[2 * poly->num_points - 2] = poly->coords[0];
		poly->coords[2 * poly->num_points - 1] = poly->coords[1];
	}
}

/* Convenience function to set a GC's foreground color to the specified pixel value */
static void
set_gc_foreground (GdkGC *gc, gulong pixel)
{
	GdkColor c;

	if (!gc)
		return;

	c.pixel = pixel;
	gdk_gc_set_foreground (gc, &c);
}

/* Sets the stipple pattern for the specified gc */
static void
set_stipple (GdkGC *gc, GdkBitmap **internal_stipple, GdkBitmap *stipple, int reconfigure)
{
	if (*internal_stipple && !reconfigure)
		gdk_bitmap_unref (*internal_stipple);

	*internal_stipple = stipple;
	if (stipple && !reconfigure)
		gdk_bitmap_ref (stipple);

	if (gc) {
		if (stipple) {
			gdk_gc_set_stipple (gc, stipple);
			gdk_gc_set_fill (gc, GDK_STIPPLED);
		} else
			gdk_gc_set_fill (gc, GDK_SOLID);
	}
}

/* Recalculate the outline width of the polygon and set it in its GC */
static void
set_outline_gc_width (GtkCanvasPolygon *poly)
{
	int width;

	if (!poly->outline_gc)
		return;

	if (poly->width_pixels)
		width = (int) poly->width;
	else
		width = (int) (poly->width * poly->item.canvas->pixels_per_unit + 0.5);

	gdk_gc_set_line_attributes (poly->outline_gc, width,
				    GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
}

static void
gtk_canvas_polygon_set_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	GtkCanvasItem *item;
	GtkCanvasPolygon *poly;
	GtkCanvasPoints *points;
	GdkColor color = { 0, 0, 0, 0, };
	GdkColor *pcolor;
	int have_pixel;

	item = GTK_CANVAS_ITEM (object);
	poly = GTK_CANVAS_POLYGON (object);
	have_pixel = FALSE;

	switch (arg_id) {
	case ARG_POINTS:
		points = GTK_VALUE_POINTER (*arg);

		if (poly->coords) {
			g_free (poly->coords);
			poly->coords = NULL;
		}

		if (!points)
			poly->num_points = 0;
		else
			set_points (poly, points);

		gtk_canvas_item_request_update (item);
		break;

        case ARG_FILL_COLOR:
	case ARG_FILL_COLOR_GDK:
	case ARG_FILL_COLOR_RGBA:
		switch (arg_id) {
		case ARG_FILL_COLOR:
			if (GTK_VALUE_STRING (*arg) &&
			    gdk_color_parse (GTK_VALUE_STRING (*arg), &color))
				poly->fill_set = TRUE;
			else
				poly->fill_set = FALSE;

			poly->fill_color = ((color.red & 0xff00) << 16 |
					    (color.green & 0xff00) << 8 |
					    (color.blue & 0xff00) |
					    0xff);
			break;

		case ARG_FILL_COLOR_GDK:
			pcolor = GTK_VALUE_BOXED (*arg);
			poly->fill_set = pcolor != NULL;

			if (pcolor) {
				color = *pcolor;
				gdk_color_context_query_color (item->canvas->cc, &color);
				have_pixel = TRUE;
			}

			poly->fill_color = ((color.red & 0xff00) << 16 |
					    (color.green & 0xff00) << 8 |
					    (color.blue & 0xff00) |
					    0xff);
			break;

		case ARG_FILL_COLOR_RGBA:
			poly->fill_set = TRUE;
			poly->fill_color = GTK_VALUE_UINT (*arg);
			break;
		}
#ifdef VERBOSE
		g_print ("poly fill color = %08x\n", poly->fill_color);
#endif
		if (have_pixel)
			poly->fill_pixel = color.pixel;
		else
			poly->fill_pixel = gtk_canvas_get_color_pixel (item->canvas,
									 poly->fill_color);

		set_gc_foreground (poly->fill_gc, poly->fill_pixel);
                gtk_canvas_item_request_redraw_svp (item, poly->fill_svp);
		break;

        case ARG_OUTLINE_COLOR:
	case ARG_OUTLINE_COLOR_GDK:
	case ARG_OUTLINE_COLOR_RGBA:
		switch (arg_id) {
		case ARG_OUTLINE_COLOR:
			if (GTK_VALUE_STRING (*arg) &&
			    gdk_color_parse (GTK_VALUE_STRING (*arg), &color))
				poly->outline_set = TRUE;
			else
				poly->outline_set = FALSE;

			poly->outline_color = ((color.red & 0xff00) << 16 |
					       (color.green & 0xff00) << 8 |
					       (color.blue & 0xff00) |
					       0xff);
			break;

		case ARG_OUTLINE_COLOR_GDK:
			pcolor = GTK_VALUE_BOXED (*arg);
			poly->outline_set = pcolor != NULL;

			if (pcolor) {
				color = *pcolor;
				gdk_color_context_query_color (item->canvas->cc, &color);
				have_pixel = TRUE;
			}

			poly->outline_color = ((color.red & 0xff00) << 16 |
					       (color.green & 0xff00) << 8 |
					       (color.blue & 0xff00) |
					       0xff);
			break;

		case ARG_OUTLINE_COLOR_RGBA:
			poly->outline_set = TRUE;
			poly->outline_color = GTK_VALUE_UINT (*arg);
			break;
		}
#ifdef VERBOSE
		g_print ("poly outline color = %08x\n", poly->outline_color);
#endif
		if (have_pixel)
			poly->outline_pixel = color.pixel;
		else
			poly->outline_pixel = gtk_canvas_get_color_pixel (item->canvas,
									    poly->outline_color);

		set_gc_foreground (poly->outline_gc, poly->outline_pixel);
                gtk_canvas_item_request_redraw_svp (item, poly->outline_svp);
		break;

	case ARG_FILL_STIPPLE:
		set_stipple (poly->fill_gc, &poly->fill_stipple, GTK_VALUE_BOXED (*arg), FALSE);
		gtk_canvas_item_request_update (item);
		break;

	case ARG_OUTLINE_STIPPLE:
		set_stipple (poly->outline_gc, &poly->outline_stipple, GTK_VALUE_BOXED (*arg), FALSE);
		gtk_canvas_item_request_update (item);
		break;

	case ARG_WIDTH_PIXELS:
		poly->width = GTK_VALUE_UINT (*arg);
		poly->width_pixels = TRUE;
		set_outline_gc_width (poly);
#ifdef OLD_XFORM
		recalc_bounds (poly);
#else
		gtk_canvas_item_request_update (item);
#endif
		break;

	case ARG_WIDTH_UNITS:
		poly->width = fabs (GTK_VALUE_DOUBLE (*arg));
		poly->width_pixels = FALSE;
		set_outline_gc_width (poly);
#ifdef OLD_XFORM
		recalc_bounds (poly);
#else
		gtk_canvas_item_request_update (item);
#endif
		break;

	default:
		break;
	}
}

/* Allocates a GdkColor structure filled with the specified pixel, and puts it into the specified
 * arg for returning it in the get_arg method.
 */
static void
get_color_arg (GtkCanvasPolygon *poly, gulong pixel, GtkArg *arg)
{
	GdkColor *color;

	color = g_new (GdkColor, 1);
	color->pixel = pixel;
	gdk_color_context_query_color (poly->item.canvas->cc, color);
	GTK_VALUE_BOXED (*arg) = color;
}

static void
gtk_canvas_polygon_get_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	GtkCanvasPolygon *poly;
	GtkCanvasPoints *points;
	GdkColor *color;

	poly = GTK_CANVAS_POLYGON (object);

	switch (arg_id) {
	case ARG_POINTS:
		if (poly->num_points != 0) {
			points = gtk_canvas_points_new (poly->num_points);
			memcpy (points->coords, poly->coords, 2 * poly->num_points * sizeof (double));
			GTK_VALUE_POINTER (*arg) = points;
		} else
			GTK_VALUE_POINTER (*arg) = NULL;
		break;

	case ARG_FILL_COLOR_GDK:
		get_color_arg (poly, poly->fill_pixel, arg);
		break;

	case ARG_OUTLINE_COLOR_GDK:
		get_color_arg (poly, poly->outline_pixel, arg);
		break;

	case ARG_FILL_COLOR_RGBA:
		GTK_VALUE_UINT (*arg) = poly->fill_color;
		break;

	case ARG_OUTLINE_COLOR_RGBA:
		GTK_VALUE_UINT (*arg) = poly->outline_color;
		break;

	case ARG_FILL_STIPPLE:
		GTK_VALUE_BOXED (*arg) = poly->fill_stipple;
		break;

	case ARG_OUTLINE_STIPPLE:
		GTK_VALUE_BOXED (*arg) = poly->outline_stipple;
		break;

	default:
		arg->type = GTK_TYPE_INVALID;
		break;
	}
}

static void
gtk_canvas_polygon_render (GtkCanvasItem *item,
			     GtkCanvasBuf *buf)
{
	GtkCanvasPolygon *poly;
	guint32 fg_color, bg_color;

	poly = GTK_CANVAS_POLYGON (item);

	if (poly->fill_svp != NULL)
		gtk_canvas_render_svp (buf, poly->fill_svp, poly->fill_color);

	if (poly->outline_svp != NULL)
		gtk_canvas_render_svp (buf, poly->outline_svp, poly->outline_color);
}

static void
gtk_canvas_polygon_update (GtkCanvasItem *item, double *affine, ArtSVP *clip_path, int flags)
{
	GtkCanvasPolygon *poly;
	ArtVpath *vpath;
	int i;
	ArtPoint pi, pc;
	ArtSVP *svp;
	double width;
	double x1, y1, x2, y2;

	poly = GTK_CANVAS_POLYGON (item);

	if (parent_class->update)
		(* parent_class->update) (item, affine, clip_path, flags);

	if (item->canvas->aa) {
		gtk_canvas_item_reset_bounds (item);

		vpath = art_new (ArtVpath, poly->num_points + 2);

		for (i = 0; i < poly->num_points; i++) {
			pi.x = poly->coords[i * 2];
			pi.y = poly->coords[i * 2 + 1];
			art_affine_point (&pc, &pi, affine);
			vpath[i].code = i == 0 ? ART_MOVETO : ART_LINETO;
			vpath[i].x = pc.x;
			vpath[i].y = pc.y;
		}
		vpath[i].code = ART_END;
		vpath[i].x = 0;
		vpath[i].y = 0;

		if (poly->fill_set) {
			svp = art_svp_from_vpath (vpath);
			gtk_canvas_item_update_svp_clip (item, &poly->fill_svp, svp, clip_path);
		}

		if (poly->outline_set) {
			if (poly->width_pixels)
				width = poly->width;
			else
				width = poly->width * item->canvas->pixels_per_unit;

			if (width < 0.5)
				width = 0.5;

			svp = art_svp_vpath_stroke (vpath,
						    ART_PATH_STROKE_JOIN_ROUND,
						    ART_PATH_STROKE_CAP_ROUND,
						    width,
						    4,
						    0.5);

			gtk_canvas_item_update_svp_clip (item, &poly->outline_svp, svp, clip_path);
		}
		art_free (vpath);

	} else {
		set_outline_gc_width (poly);
		set_gc_foreground (poly->fill_gc, poly->fill_pixel);
		set_gc_foreground (poly->outline_gc, poly->outline_pixel);
		set_stipple (poly->fill_gc, &poly->fill_stipple, poly->fill_stipple, TRUE);
		set_stipple (poly->outline_gc, &poly->outline_stipple, poly->outline_stipple, TRUE);

		get_bounds_canvas (poly, &x1, &y1, &x2, &y2, affine);
		gtk_canvas_update_bbox (item, x1, y1, x2, y2);
	}
}

static void
gtk_canvas_polygon_realize (GtkCanvasItem *item)
{
	GtkCanvasPolygon *poly;

	poly = GTK_CANVAS_POLYGON (item);

	if (parent_class->realize)
		(* parent_class->realize) (item);

	poly->fill_gc = gdk_gc_new (item->canvas->layout.bin_window);
	poly->outline_gc = gdk_gc_new (item->canvas->layout.bin_window);

#ifdef OLD_XFORM
	(* GTK_CANVAS_ITEM_CLASS (item->object.klass)->update) (item, NULL, NULL, 0);
#endif
}

static void
gtk_canvas_polygon_unrealize (GtkCanvasItem *item)
{
	GtkCanvasPolygon *poly;

	poly = GTK_CANVAS_POLYGON (item);

	gdk_gc_unref (poly->fill_gc);
	poly->fill_gc = NULL;
	gdk_gc_unref (poly->outline_gc);
	poly->outline_gc = NULL;

	if (parent_class->unrealize)
		(* parent_class->unrealize) (item);
}

/* Converts an array of world coordinates into an array of canvas pixel coordinates.  Takes in the
 * item->world deltas and the drawable deltas.
 */
static void
item_to_canvas (GtkCanvas *canvas, double *item_coords, GdkPoint *canvas_coords, int num_points,
		double i2c[6])
{
	int i;
	ArtPoint pi, pc;

#ifdef VERBOSE
	{
		char str[128];
		art_affine_to_string (str, i2c);
		g_print ("polygon item_to_canvas %s\n", str);
	}
#endif

	for (i = 0; i < num_points; i++) {
		pi.x = item_coords[i * 2];
		pi.y = item_coords[i * 2 + 1];
		art_affine_point (&pc, &pi, i2c);
		canvas_coords->x = floor (pc.x + 0.5);
		canvas_coords->y = floor (pc.y + 0.5);
		canvas_coords++;
	}
}

static void
gtk_canvas_polygon_draw (GtkCanvasItem *item, GdkDrawable *drawable,
			   int x, int y, int width, int height)
{
	GtkCanvasPolygon *poly;
	GdkPoint static_points[NUM_STATIC_POINTS];
	GdkPoint *points;
	double dx, dy;
	int cx, cy;
	int i;
	double i2c[6];

	poly = GTK_CANVAS_POLYGON (item);

	if (poly->num_points == 0)
		return;

	/* Build array of canvas pixel coordinates */

	if (poly->num_points <= NUM_STATIC_POINTS)
		points = static_points;
	else
		points = g_new (GdkPoint, poly->num_points);

	gtk_canvas_item_i2c_affine (item, i2c);

	i2c[4] -= x;
	i2c[5] -= y;

	item_to_canvas (item->canvas, poly->coords, points, poly->num_points, i2c);

	if (poly->fill_set) {
		if (poly->fill_stipple)
			gtk_canvas_set_stipple_origin (item->canvas, poly->fill_gc);

		gdk_draw_polygon (drawable, poly->fill_gc, TRUE, points, poly->num_points);
	}

	if (poly->outline_set) {
		if (poly->outline_stipple)
			gtk_canvas_set_stipple_origin (item->canvas, poly->outline_gc);

		gdk_draw_polygon (drawable, poly->outline_gc, FALSE, points, poly->num_points);
	}

	/* Done */

	if (points != static_points)
		g_free (points);
}

static double
gtk_canvas_polygon_point (GtkCanvasItem *item, double x, double y,
			    int cx, int cy, GtkCanvasItem **actual_item)
{
	GtkCanvasPolygon *poly;
	double dist;
	double width;

	poly = GTK_CANVAS_POLYGON (item);

	*actual_item = item;

	dist = gtk_canvas_polygon_to_point (poly->coords, poly->num_points, x, y);

	if (poly->outline_set) {
		if (poly->width_pixels)
			width = poly->width / item->canvas->pixels_per_unit;
		else
			width = poly->width;

		dist -= width / 2.0;

		if (dist < 0.0)
			dist = 0.0;
	}

	return dist;
}

static void
gtk_canvas_polygon_translate (GtkCanvasItem *item, double dx, double dy)
{
	GtkCanvasPolygon *poly;
	int i;
	double *coords;

	poly = GTK_CANVAS_POLYGON (item);

	for (i = 0, coords = poly->coords; i < poly->num_points; i++, coords += 2) {
		coords[0] += dx;
		coords[1] += dy;
	}

	recalc_bounds (poly);
}

static void
gtk_canvas_polygon_bounds (GtkCanvasItem *item, double *x1, double *y1, double *x2, double *y2)
{
	GtkCanvasPolygon *poly;

	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_POLYGON (item));

	poly = GTK_CANVAS_POLYGON (item);

	if (poly->num_points == 0) {
		*x1 = *y1 = *x2 = *y2 = 0.0;
		return;
	}

	get_bounds (poly, x1, y1, x2, y2);
}
