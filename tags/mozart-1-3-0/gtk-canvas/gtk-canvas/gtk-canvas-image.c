/* Image item type for GtkCanvas widget
 *
 * GtkCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 * Copyright (C) 1998 The Free Software Foundation
 *
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 */

#include <config.h>
#include <string.h> /* for memcpy() */
#include <math.h>
#include "libart_lgpl/art_misc.h"
#include "libart_lgpl/art_affine.h"
#include "libart_lgpl/art_pixbuf.h"
#include "libart_lgpl/art_rgb_pixbuf_affine.h"
#include "gtk-canvas-image.h"
#include "gtk-canvas-util.h"
#include "gtk-canvastypebuiltins.h"


enum {
	ARG_0,
	ARG_IMAGE,
	ARG_PIXBUF,
	ARG_X,
	ARG_Y,
	ARG_WIDTH,
	ARG_HEIGHT,
	ARG_ANCHOR
};


static void gtk_canvas_image_class_init (GtkCanvasImageClass *class);
static void gtk_canvas_image_init       (GtkCanvasImage      *image);
static void gtk_canvas_image_destroy    (GtkObject             *object);
static void gtk_canvas_image_set_arg    (GtkObject             *object,
					   GtkArg                *arg,
					   guint                  arg_id);
static void gtk_canvas_image_get_arg    (GtkObject             *object,
					   GtkArg                *arg,
					   guint                  arg_id);

static void   gtk_canvas_image_update      (GtkCanvasItem *item, double *affine, ArtSVP *clip_path, int flags);
static void   gtk_canvas_image_realize     (GtkCanvasItem *item);
static void   gtk_canvas_image_unrealize   (GtkCanvasItem *item);
static void   gtk_canvas_image_draw        (GtkCanvasItem *item, GdkDrawable *drawable,
					      int x, int y, int width, int height);
static double gtk_canvas_image_point       (GtkCanvasItem *item, double x, double y,
					      int cx, int cy, GtkCanvasItem **actual_item);
static void   gtk_canvas_image_translate   (GtkCanvasItem *item, double dx, double dy);
static void   gtk_canvas_image_bounds      (GtkCanvasItem *item, double *x1, double *y1, double *x2, double *y2);
static void   gtk_canvas_image_render      (GtkCanvasItem *item, GtkCanvasBuf *buf);

static ArtPixBuf * pixbuf_from_imlib_image (GdkImlibImage *im);

static GtkCanvasItemClass *parent_class;


GtkType
gtk_canvas_image_get_type (void)
{
	static GtkType image_type = 0;

	if (!image_type) {
		GtkTypeInfo image_info = {
			"GtkCanvasImage",
			sizeof (GtkCanvasImage),
			sizeof (GtkCanvasImageClass),
			(GtkClassInitFunc) gtk_canvas_image_class_init,
			(GtkObjectInitFunc) gtk_canvas_image_init,
			NULL, /* reserved_1 */
			NULL, /* reserved_2 */
			(GtkClassInitFunc) NULL
		};

		image_type = gtk_type_unique (gtk_canvas_item_get_type (), &image_info);
	}

	return image_type;
}

static void
gtk_canvas_image_class_init (GtkCanvasImageClass *class)
{
	GtkObjectClass *object_class;
	GtkCanvasItemClass *item_class;

	object_class = (GtkObjectClass *) class;
	item_class = (GtkCanvasItemClass *) class;

	parent_class = gtk_type_class (gtk_canvas_item_get_type ());

	gtk_object_add_arg_type ("GtkCanvasImage::image", GTK_TYPE_GDK_IMLIB_IMAGE, GTK_ARG_READWRITE, ARG_IMAGE);
	gtk_object_add_arg_type ("GtkCanvasImage::pixbuf", GTK_TYPE_BOXED, GTK_ARG_WRITABLE, ARG_PIXBUF);
	gtk_object_add_arg_type ("GtkCanvasImage::x", GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_X);
	gtk_object_add_arg_type ("GtkCanvasImage::y", GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_Y);
	gtk_object_add_arg_type ("GtkCanvasImage::width", GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_WIDTH);
	gtk_object_add_arg_type ("GtkCanvasImage::height", GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_HEIGHT);
	gtk_object_add_arg_type ("GtkCanvasImage::anchor", GTK_TYPE_ANCHOR_TYPE, GTK_ARG_READWRITE, ARG_ANCHOR);

	object_class->destroy = gtk_canvas_image_destroy;
	object_class->set_arg = gtk_canvas_image_set_arg;
	object_class->get_arg = gtk_canvas_image_get_arg;

	item_class->update = gtk_canvas_image_update;
	item_class->realize = gtk_canvas_image_realize;
	item_class->unrealize = gtk_canvas_image_unrealize;
	item_class->draw = gtk_canvas_image_draw;
	item_class->point = gtk_canvas_image_point;
	item_class->translate = gtk_canvas_image_translate;
	item_class->bounds = gtk_canvas_image_bounds;
	item_class->render = gtk_canvas_image_render;
}

static void
gtk_canvas_image_init (GtkCanvasImage *image)
{
	image->x = 0.0;
	image->y = 0.0;
	image->width = 0.0;
	image->height = 0.0;
	image->anchor = GTK_ANCHOR_CENTER;
}

static void
free_pixmap_and_mask (GtkCanvasImage *image)
{
	if (image->pixmap)
		gdk_imlib_free_pixmap (image->pixmap);
#if 0
	/* When you tell imlib to free a pixmap, it will also free its
	 * associated mask.  Now is that broken, or what?
	 */
	if (image->mask)
		gdk_imlib_free_bitmap (image->mask);
#endif

	image->pixmap = NULL;
	image->mask = NULL;
	image->cwidth = 0;
	image->cheight = 0;
}

static void
gtk_canvas_image_destroy (GtkObject *object)
{
	GtkCanvasImage *image;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_IMAGE (object));

	image = GTK_CANVAS_IMAGE (object);

	free_pixmap_and_mask (image);

	if (image->pixbuf) {
		art_pixbuf_free (image->pixbuf);
		image->pixbuf = NULL;
	}

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

/* Get's the image bounds expressed as item-relative coordinates. */
static void
get_bounds_item_relative (GtkCanvasImage *image, double *px1, double *py1, double *px2, double *py2)
{
	GtkCanvasItem *item;
	double x, y;

	item = GTK_CANVAS_ITEM (image);

	/* Get item coordinates */

	x = image->x;
	y = image->y;

	/* Anchor image */

	switch (image->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_W:
	case GTK_ANCHOR_SW:
		break;

	case GTK_ANCHOR_N:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_S:
		x -= image->width / 2;
		break;

	case GTK_ANCHOR_NE:
	case GTK_ANCHOR_E:
	case GTK_ANCHOR_SE:
		x -= image->width;
		break;
	}

	switch (image->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_N:
	case GTK_ANCHOR_NE:
		break;

	case GTK_ANCHOR_W:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_E:
		y -= image->height / 2;
		break;

	case GTK_ANCHOR_SW:
	case GTK_ANCHOR_S:
	case GTK_ANCHOR_SE:
		y -= image->height;
		break;
	}

	/* Bounds */

	*px1 = x;
	*py1 = y;
	*px2 = x + image->width;
	*py2 = y + image->height;
}

static void
get_bounds (GtkCanvasImage *image, double *px1, double *py1, double *px2, double *py2)
{
	GtkCanvasItem *item;
	double wx, wy;
	double i2c[6];
	ArtDRect i_bbox, c_bbox;

	item = GTK_CANVAS_ITEM (image);

	gtk_canvas_item_i2c_affine (item, i2c);

	get_bounds_item_relative (image, &i_bbox.x0, &i_bbox.y0, &i_bbox.x1, &i_bbox.y1);
	art_drect_affine_transform (&c_bbox, &i_bbox, i2c);

	/* add a fudge factor */
	*px1 = c_bbox.x0 - 1;
	*py1 = c_bbox.y0 - 1;
	*px2 = c_bbox.x1 + 1;
	*py2 = c_bbox.y1 + 1;
}

/* deprecated */
static void
recalc_bounds (GtkCanvasImage *image)
{
	GtkCanvasItem *item;
	double wx, wy;

	item = GTK_CANVAS_ITEM (image);

	get_bounds (image, &item->x1, &item->y1, &item->x2, &item->y2);

	item->x1 = image->cx;
	item->y1 = image->cy;
	item->x2 = image->cx + image->cwidth;
	item->y2 = image->cy + image->cheight;

	gtk_canvas_group_child_bounds (GTK_CANVAS_GROUP (item->parent), item);
}

static void
gtk_canvas_image_set_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	GtkCanvasItem *item;
	GtkCanvasImage *image;
	int update;
	int calc_bounds;

	item = GTK_CANVAS_ITEM (object);
	image = GTK_CANVAS_IMAGE (object);

	update = FALSE;
	calc_bounds = FALSE;

	switch (arg_id) {
	case ARG_IMAGE:
		/* The pixmap and mask will be freed when the item is reconfigured */
		image->im = GTK_VALUE_POINTER (*arg);	
		if (item->canvas->aa) {
			if (image->pixbuf != NULL)
				art_pixbuf_free (image->pixbuf);
			image->pixbuf = pixbuf_from_imlib_image (image->im);
		}
		update = TRUE;
		break;

	case ARG_PIXBUF:
		/* The pixmap and mask will be freed when the item is reconfigured */
		if (item->canvas->aa && GTK_VALUE_BOXED (*arg)) {
			image->im = NULL;
			if (image->pixbuf != NULL)
				art_pixbuf_free (image->pixbuf);
			image->pixbuf = GTK_VALUE_BOXED (*arg);
		}
		update = TRUE;
		break;

	case ARG_X:
		image->x = GTK_VALUE_DOUBLE (*arg);
		update = TRUE;
		break;

	case ARG_Y:
		image->y = GTK_VALUE_DOUBLE (*arg);
		update = TRUE;
		break;

	case ARG_WIDTH:
		image->width = fabs (GTK_VALUE_DOUBLE (*arg));
		update = TRUE;
		break;

	case ARG_HEIGHT:
		image->height = fabs (GTK_VALUE_DOUBLE (*arg));
		update = TRUE;
		break;

	case ARG_ANCHOR:
		image->anchor = GTK_VALUE_ENUM (*arg);
		update = TRUE;
		break;

	default:
		break;
	}

#ifdef OLD_XFORM
	if (update)
		(* GTK_CANVAS_ITEM_CLASS (item->object.klass)->update) (item, NULL, NULL, 0);

	if (calc_bounds)
		recalc_bounds (image);
#else
	if (update)
		gtk_canvas_item_request_update (item);
#endif
}

static void
gtk_canvas_image_get_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	GtkCanvasImage *image;

	image = GTK_CANVAS_IMAGE (object);

	switch (arg_id) {
	case ARG_IMAGE:
		GTK_VALUE_POINTER (*arg) = image->im;
		break;

	case ARG_X:
		GTK_VALUE_DOUBLE (*arg) = image->x;
		break;

	case ARG_Y:
		GTK_VALUE_DOUBLE (*arg) = image->y;
		break;

	case ARG_WIDTH:
		GTK_VALUE_DOUBLE (*arg) = image->width;
		break;

	case ARG_HEIGHT:
		GTK_VALUE_DOUBLE (*arg) = image->height;
		break;

	case ARG_ANCHOR:
		GTK_VALUE_ENUM (*arg) = image->anchor;
		break;

	default:
		arg->type = GTK_TYPE_INVALID;
		break;
	}
}

static void
gtk_canvas_image_update (GtkCanvasItem *item, double *affine, ArtSVP *clip_path, int flags)
{
	GtkCanvasImage *image;
	double x1, y1, x2, y2;
	ArtDRect i_bbox, c_bbox;
	int w, h;

	image = GTK_CANVAS_IMAGE (item);

	if (parent_class->update)
		(* parent_class->update) (item, affine, clip_path, flags);

	free_pixmap_and_mask (image);

	/* only works for non-rotated, non-skewed transforms */
	image->cwidth = (int) (image->width * affine[0] + 0.5);
	image->cheight = (int) (image->height * affine[3] + 0.5);

	if (image->im || image->pixbuf)
		image->need_recalc = TRUE;

#ifdef OLD_XFORM
	recalc_bounds (image);
#else
	get_bounds_item_relative (image, &i_bbox.x0, &i_bbox.y0, &i_bbox.x1, &i_bbox.y1);
	art_drect_affine_transform (&c_bbox, &i_bbox, affine);

	/* these values only make sense in the non-rotated, non-skewed case */
	image->cx = c_bbox.x0;
	image->cy = c_bbox.y0;

	/* add a fudge factor */
	c_bbox.x0--;
	c_bbox.y0--;
	c_bbox.x1++;
	c_bbox.y1++;

	gtk_canvas_update_bbox (item, c_bbox.x0, c_bbox.y0, c_bbox.x1, c_bbox.y1);

	if (image->im) {
		w = image->im->rgb_width;
		h = image->im->rgb_height;
	} else if (image->pixbuf) {
		w = image->pixbuf->width;
		h = image->pixbuf->height;
	} else
		w = h = 1;

	image->affine[0] = (affine[0] * image->width) / w;
	image->affine[1] = (affine[1] * image->height) / h;
	image->affine[2] = (affine[2] * image->width) / w;
	image->affine[3] = (affine[3] * image->height) / h;
	image->affine[4] = i_bbox.x0 * affine[0] + i_bbox.y0 * affine[2] + affine[4];
	image->affine[5] = i_bbox.x0 * affine[1] + i_bbox.y0 * affine[3] + affine[5];
#endif
}

static void
gtk_canvas_image_realize (GtkCanvasItem *item)
{
	GtkCanvasImage *image;

	image = GTK_CANVAS_IMAGE (item);

	if (parent_class->realize)
		(* parent_class->realize) (item);

	if (!item->canvas->aa)
		image->gc = gdk_gc_new (item->canvas->layout.bin_window);
}

static void
gtk_canvas_image_unrealize (GtkCanvasItem *item)
{
	GtkCanvasImage *image;

	image = GTK_CANVAS_IMAGE (item);

	if (!item->canvas->aa) {
		gdk_gc_unref (image->gc);
		image->gc = NULL;
	}

	if (parent_class->unrealize)
		(* parent_class->unrealize) (item);
}

static void
recalc_if_needed (GtkCanvasImage *image)
{
	if (!image->need_recalc)
		return;

	get_bounds (image, &image->item.x1, &image->item.y1, &image->item.x2, &image->item.y2);

	if (image->im && image->cwidth != 0 && image->cheight != 0) {
		gdk_imlib_render (image->im, image->cwidth, image->cheight);
		
		image->pixmap = gdk_imlib_move_image (image->im);
		g_assert (image->pixmap != NULL);
		image->mask = gdk_imlib_move_mask (image->im);
		
		if (image->gc)
			gdk_gc_set_clip_mask (image->gc, image->mask);
	}

	image->need_recalc = FALSE;
}

static void
gtk_canvas_image_draw (GtkCanvasItem *item, GdkDrawable *drawable,
			 int x, int y, int width, int height)
{
	GtkCanvasImage *image;

	image = GTK_CANVAS_IMAGE (item);

	if (!image->im)
		return;

	recalc_if_needed (image);

	if (image->mask)
		gdk_gc_set_clip_origin (image->gc, image->cx - x, image->cy - y);

	if (image->pixmap)
		gdk_draw_pixmap (drawable,
				 image->gc,
				 image->pixmap,
				 0, 0,
				 image->cx - x,
				 image->cy - y,
				 image->cwidth,
				 image->cheight);
}

static double
dist_to_mask (GtkCanvasImage *image, int cx, int cy)
{
	GtkCanvasItem *item;
	GdkImage *gimage;
	GdkRectangle a, b, dest;
	int x, y, tx, ty;
	double dist, best;

	item = GTK_CANVAS_ITEM (image);

	/* Trivial case:  if there is no mask, we are inside */

	if (!image->mask)
		return 0.0;

	/* Rectangle that we need */

	cx -= image->cx;
	cy -= image->cy;

	a.x = cx - item->canvas->close_enough;
	a.y = cy - item->canvas->close_enough;
	a.width = 2 * item->canvas->close_enough + 1;
	a.height = 2 * item->canvas->close_enough + 1;

	/* Image rectangle */

	b.x = 0;
	b.y = 0;
	b.width = image->cwidth;
	b.height = image->cheight;

	if (!gdk_rectangle_intersect (&a, &b, &dest))
		return a.width * a.height; /* "big" value */

	gimage = gdk_image_get (image->mask, dest.x, dest.y, dest.width, dest.height);

	/* Find the closest pixel */

	best = a.width * a.height; /* start with a "big" value */

	for (y = 0; y < dest.height; y++)
		for (x = 0; x < dest.width; x++)
			if (gdk_image_get_pixel (gimage, x, y)) {
				tx = x + dest.x - cx;
				ty = y + dest.y - cy;

				dist = sqrt (tx * tx + ty * ty);
				if (dist < best)
					best = dist;
			}

	gdk_image_destroy (gimage);
	return best;
}

static double
gtk_canvas_image_point (GtkCanvasItem *item, double x, double y,
			  int cx, int cy, GtkCanvasItem **actual_item)
{
	GtkCanvasImage *image;
	int x1, y1, x2, y2;
	int dx, dy;

	image = GTK_CANVAS_IMAGE (item);

	*actual_item = item;

	recalc_if_needed (image);

	x1 = image->cx - item->canvas->close_enough;
	y1 = image->cy - item->canvas->close_enough;
	x2 = image->cx + image->cwidth - 1 + item->canvas->close_enough;
	y2 = image->cy + image->cheight - 1 + item->canvas->close_enough;

	/* Hard case: is point inside image's gravity region? */

	if ((cx >= x1) && (cy >= y1) && (cx <= x2) && (cy <= y2))
		return dist_to_mask (image, cx, cy) / item->canvas->pixels_per_unit;

	/* Point is outside image */

	x1 += item->canvas->close_enough;
	y1 += item->canvas->close_enough;
	x2 -= item->canvas->close_enough;
	y2 -= item->canvas->close_enough;

	if (cx < x1)
		dx = x1 - cx;
	else if (cx > x2)
		dx = cx - x2;
	else
		dx = 0;

	if (cy < y1)
		dy = y1 - cy;
	else if (cy > y2)
		dy = cy - y2;
	else
		dy = 0;

	return sqrt (dx * dx + dy * dy) / item->canvas->pixels_per_unit;
}

static void
gtk_canvas_image_translate (GtkCanvasItem *item, double dx, double dy)
{
#ifdef OLD_XFORM
	GtkCanvasImage *image;

	image = GTK_CANVAS_IMAGE (item);

	image->x += dx;
	image->y += dy;

	recalc_bounds (image);
#endif
}

static void
gtk_canvas_image_bounds (GtkCanvasItem *item, double *x1, double *y1, double *x2, double *y2)
{
	GtkCanvasImage *image;

	image = GTK_CANVAS_IMAGE (item);

	*x1 = image->x;
	*y1 = image->y;

	switch (image->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_W:
	case GTK_ANCHOR_SW:
		break;

	case GTK_ANCHOR_N:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_S:
		*x1 -= image->width / 2.0;
		break;

	case GTK_ANCHOR_NE:
	case GTK_ANCHOR_E:
	case GTK_ANCHOR_SE:
		*x1 -= image->width;
		break;
	}

	switch (image->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_N:
	case GTK_ANCHOR_NE:
		break;

	case GTK_ANCHOR_W:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_E:
		*y1 -= image->height / 2.0;
		break;

	case GTK_ANCHOR_SW:
	case GTK_ANCHOR_S:
	case GTK_ANCHOR_SE:
		*y1 -= image->height;
		break;
	}

	*x2 = *x1 + image->width;
	*y2 = *y1 + image->height;
}

static void
gtk_canvas_image_render      (GtkCanvasItem *item, GtkCanvasBuf *buf)
{
	GtkCanvasImage *image;

	image = GTK_CANVAS_IMAGE (item);

        gtk_canvas_buf_ensure_buf (buf);

#ifdef VERBOSE
	{
		char str[128];
		art_affine_to_string (str, image->affine);
		g_print ("gtk_canvas_image_render %s\n", str);
	}
#endif

	art_rgb_pixbuf_affine (buf->buf,
			buf->rect.x0, buf->rect.y0, buf->rect.x1, buf->rect.y1,
			buf->buf_rowstride,
			image->pixbuf,
			image->affine,
			ART_FILTER_NEAREST, NULL);

	buf->is_bg = 0;
}

/* This creates a new pixbuf from the imlib image. */
static ArtPixBuf *
pixbuf_from_imlib_image (GdkImlibImage *im)
{
	art_u8 *pixels;
	int width, height, rowstride;
	int x, y;
	unsigned char *p_src, *p_alpha;
	art_u8 *p_dst;
	art_u8 r, g, b, alpha;
	art_u8 xr, xg, xb;

	if (im->alpha_data) {
		/*
		 * image has alpha data (not presently implemented in imlib as
		 * of 15 Dec 1998, but should happen soon.
		 *
		 * Unless you use the special gtk_canvas_load which is a wrapper
		 * to fix Imlib.
		 */
		width = im->rgb_width;
		height = im->rgb_height;
		rowstride = width * 4;
		pixels = art_alloc (rowstride * height);
		p_src = im->rgb_data;
		p_alpha = im->alpha_data;
		p_dst = pixels;
		for (y = 0; y < height; y++)
			for (x = 0; x < width; x++) {
				r = p_src[0];
				g = p_src[1];
				b = p_src[2];
				alpha = p_alpha[0];

					p_dst[0] = r;
					p_dst[1] = g;
					p_dst[2] = b;
					p_dst[3] = alpha;

				p_src += 3;
				p_alpha += 1;
				p_dst += 4;
			}
		return art_pixbuf_new_rgba (pixels, width, height, rowstride);
	} else if (im->shape_color.r >= 0 && im->shape_color.g >= 0 && im->shape_color.b >= 0) {
		/* image has one transparent color */
		width = im->rgb_width;
		height = im->rgb_height;
		rowstride = width * 4;
		pixels = art_alloc (rowstride * height);
		p_src = im->rgb_data;
		p_dst = pixels;
		xr = im->shape_color.r;
		xg = im->shape_color.g;
		xb = im->shape_color.b;
		for (y = 0; y < height; y++)
			for (x = 0; x < width; x++) {
				r = p_src[0];
				g = p_src[1];
				b = p_src[2];
				if (r == xr && g == xg && b == xb) {
					((art_u32 *)p_dst)[0] = 0;
				} else {
					p_dst[0] = r;
					p_dst[1] = g;
					p_dst[2] = b;
					p_dst[3] = 255;
				}
				p_src += 3;
				p_dst += 4;
			}
		return art_pixbuf_new_rgba (pixels, width, height, rowstride);
	} else {
		/* image is solid rgb */
		width = im->rgb_width;
		height = im->rgb_height;
		rowstride = (width * 3 + 3) & -4;
		pixels = art_alloc (rowstride * height);
		p_src = im->rgb_data;
		p_dst = pixels;
		for (y = 0; y < height; y++) {
			memcpy (p_dst, p_src, width * 3);
			p_src += width * 3;
			p_dst += rowstride;
		}
		return art_pixbuf_new_rgb (pixels, width, height, rowstride);
	}
}
