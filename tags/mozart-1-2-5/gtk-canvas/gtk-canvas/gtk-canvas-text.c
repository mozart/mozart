/* Text item type for GtkCanvas widget
 *
 * GtkCanvas is basically a port of the Tk toolkit's most excellent canvas
 * widget.  Tk is copyrighted by the Regents of the University of California,
 * Sun Microsystems, and other parties.
 *
 * Copyright (C) 1998 The Free Software Foundation
 *
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 */

#include <config.h>
#include <math.h>
#include "gtk-canvas-text.h"
#include <gdk/gdkx.h> /* for BlackPixel */

#include "libart_lgpl/art_affine.h"
#include "libart_lgpl/art_rgb.h"
#include "libart_lgpl/art_rgb_bitmap_affine.h"
#include "gtk-canvas-util.h"



/* This defines a line of text */	
struct line {
	char *text;	/* Line's text, it is a pointer into the text->text string */
	int length;	/* Line's length in characters */
	int width;	/* Line's width in pixels */
	int rbearing;   /* Line's rbearing */
	int lbearing;   /* Line's lbearing */
};

struct TextPrivate {
	int min_lbearing;
	int max_rbearing;
	struct line *lines;
};



/* Object argument IDs */
enum {
	ARG_0,
	ARG_TEXT,
	ARG_X,
	ARG_Y,
	ARG_FONT,
        ARG_FONTSET,
	ARG_FONT_GDK,
	ARG_ANCHOR,
	ARG_JUSTIFICATION,
	ARG_CLIP_WIDTH,
	ARG_CLIP_HEIGHT,
	ARG_CLIP,
	ARG_X_OFFSET,
	ARG_Y_OFFSET,
	ARG_FILL_COLOR,
	ARG_FILL_COLOR_GDK,
	ARG_FILL_COLOR_RGBA,
	ARG_FILL_STIPPLE,
	ARG_TEXT_WIDTH,
	ARG_TEXT_HEIGHT
};


static void gtk_canvas_text_class_init (GtkCanvasTextClass *class);
static void gtk_canvas_text_init (GtkCanvasText *text);
static void gtk_canvas_text_destroy (GtkObject *object);
static void gtk_canvas_text_set_arg (GtkObject *object, GtkArg *arg, guint arg_id);
static void gtk_canvas_text_get_arg (GtkObject *object, GtkArg *arg, guint arg_id);

static void gtk_canvas_text_update (GtkCanvasItem *item, double *affine,
				      ArtSVP *clip_path, int flags);
static void gtk_canvas_text_realize (GtkCanvasItem *item);
static void gtk_canvas_text_unrealize (GtkCanvasItem *item);
static void gtk_canvas_text_draw (GtkCanvasItem *item, GdkDrawable *drawable,
				    int x, int y, int width, int height);
static double gtk_canvas_text_point (GtkCanvasItem *item, double x, double y, int cx, int cy,
				       GtkCanvasItem **actual_item);
static void gtk_canvas_text_bounds (GtkCanvasItem *item,
				      double *x1, double *y1, double *x2, double *y2);
static void gtk_canvas_text_render (GtkCanvasItem *item, GtkCanvasBuf *buf);
static void split_into_lines (GtkCanvasText *text);

static GtkCanvasTextSuckFont *gtk_canvas_suck_font (GdkFont *font);
static void gtk_canvas_suck_font_free (GtkCanvasTextSuckFont *suckfont);


static GtkCanvasItemClass *parent_class;



/**
 * gtk_canvas_text_get_type:
 * @void: 
 * 
 * Registers the &GtkCanvasText class if necessary, and returns the type ID
 * associated to it.
 * 
 * Return value: The type ID of the &GtkCanvasText class.
 **/
GtkType
gtk_canvas_text_get_type (void)
{
	static GtkType text_type = 0;

	if (!text_type) {
		GtkTypeInfo text_info = {
			"GtkCanvasText",
			sizeof (GtkCanvasText),
			sizeof (GtkCanvasTextClass),
			(GtkClassInitFunc) gtk_canvas_text_class_init,
			(GtkObjectInitFunc) gtk_canvas_text_init,
			NULL, /* reserved_1 */
			NULL, /* reserved_2 */
			(GtkClassInitFunc) NULL
		};

		text_type = gtk_type_unique (gtk_canvas_item_get_type (), &text_info);
	}

	return text_type;
}

/* Class initialization function for the text item */
static void
gtk_canvas_text_class_init (GtkCanvasTextClass *class)
{
	GtkObjectClass *object_class;
	GtkCanvasItemClass *item_class;

	object_class = (GtkObjectClass *) class;
	item_class = (GtkCanvasItemClass *) class;

	parent_class = gtk_type_class (gtk_canvas_item_get_type ());

	gtk_object_add_arg_type ("GtkCanvasText::text",
				 GTK_TYPE_STRING, GTK_ARG_READWRITE, ARG_TEXT);
	gtk_object_add_arg_type ("GtkCanvasText::x",
				 GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_X);
	gtk_object_add_arg_type ("GtkCanvasText::y",
				 GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_Y);
	gtk_object_add_arg_type ("GtkCanvasText::font",
				 GTK_TYPE_STRING, GTK_ARG_WRITABLE, ARG_FONT);
	gtk_object_add_arg_type ("GtkCanvasText::fontset",
				 GTK_TYPE_STRING, GTK_ARG_WRITABLE, ARG_FONTSET);
	gtk_object_add_arg_type ("GtkCanvasText::font_gdk",
				 GTK_TYPE_GDK_FONT, GTK_ARG_READWRITE, ARG_FONT_GDK);
	gtk_object_add_arg_type ("GtkCanvasText::anchor",
				 GTK_TYPE_ANCHOR_TYPE, GTK_ARG_READWRITE, ARG_ANCHOR);
	gtk_object_add_arg_type ("GtkCanvasText::justification",
				 GTK_TYPE_JUSTIFICATION, GTK_ARG_READWRITE, ARG_JUSTIFICATION);
	gtk_object_add_arg_type ("GtkCanvasText::clip_width",
				 GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_CLIP_WIDTH);
	gtk_object_add_arg_type ("GtkCanvasText::clip_height",
				 GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_CLIP_HEIGHT);
	gtk_object_add_arg_type ("GtkCanvasText::clip",
				 GTK_TYPE_BOOL, GTK_ARG_READWRITE, ARG_CLIP);
	gtk_object_add_arg_type ("GtkCanvasText::x_offset",
				 GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_X_OFFSET);
	gtk_object_add_arg_type ("GtkCanvasText::y_offset",
				 GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_Y_OFFSET);
	gtk_object_add_arg_type ("GtkCanvasText::fill_color",
				 GTK_TYPE_STRING, GTK_ARG_WRITABLE, ARG_FILL_COLOR);
	gtk_object_add_arg_type ("GtkCanvasText::fill_color_gdk",
				 GTK_TYPE_GDK_COLOR, GTK_ARG_READWRITE, ARG_FILL_COLOR_GDK);
	gtk_object_add_arg_type ("GtkCanvasText::fill_color_rgba",
				 GTK_TYPE_UINT, GTK_ARG_READWRITE, ARG_FILL_COLOR_RGBA);
	gtk_object_add_arg_type ("GtkCanvasText::fill_stipple",
				 GTK_TYPE_GDK_WINDOW, GTK_ARG_READWRITE, ARG_FILL_STIPPLE);
	gtk_object_add_arg_type ("GtkCanvasText::text_width",
				 GTK_TYPE_DOUBLE, GTK_ARG_READABLE, ARG_TEXT_WIDTH);
	gtk_object_add_arg_type ("GtkCanvasText::text_height",
				 GTK_TYPE_DOUBLE, GTK_ARG_READABLE, ARG_TEXT_HEIGHT);

	object_class->destroy = gtk_canvas_text_destroy;
	object_class->set_arg = gtk_canvas_text_set_arg;
	object_class->get_arg = gtk_canvas_text_get_arg;

	item_class->update = gtk_canvas_text_update;
	item_class->realize = gtk_canvas_text_realize;
	item_class->unrealize = gtk_canvas_text_unrealize;
	item_class->draw = gtk_canvas_text_draw;
	item_class->point = gtk_canvas_text_point;
	item_class->bounds = gtk_canvas_text_bounds;
	item_class->render = gtk_canvas_text_render;
}

/* Object initialization function for the text item */
static void
gtk_canvas_text_init (GtkCanvasText *text)
{
	text->x = 0.0;
	text->y = 0.0;
	text->anchor = GTK_ANCHOR_CENTER;
	text->justification = GTK_JUSTIFY_LEFT;
	text->clip_width = 0.0;
	text->clip_height = 0.0;
	text->xofs = 0.0;
	text->yofs = 0.0;
}

/* Destroy handler for the text item */
static void
gtk_canvas_text_destroy (GtkObject *object)
{
	GtkCanvasText *text;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_TEXT (object));

	text = GTK_CANVAS_TEXT (object);

	if (text->text)
		g_free (text->text);

	if (text->lines) {
		if (((struct TextPrivate *)text->lines)->lines)
			g_free (((struct TextPrivate *)text->lines)->lines);
		g_free (text->lines);
	}

	if (text->font)
		gdk_font_unref (text->font);

	if (text->suckfont)
		gtk_canvas_suck_font_free (text->suckfont);

	if (text->stipple)
		gdk_bitmap_unref (text->stipple);

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
get_bounds_item_relative (GtkCanvasText *text, double *px1, double *py1, double *px2, double *py2)
{
	GtkCanvasItem *item;
	struct TextPrivate *private;
	double x, y;
	double clip_x, clip_y;

	private = (struct TextPrivate*) text->lines;
	item = GTK_CANVAS_ITEM (text);

	x = text->x;
	y = text->y;

	clip_x = x;
	clip_y = y;

	/* Calculate text dimensions */

	if (text->text && text->font)
		text->height = (text->font->ascent + text->font->descent) * text->num_lines;
	else
		text->height = 0;

	/* Anchor text */

	switch (text->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_W:
	case GTK_ANCHOR_SW:
		break;

	case GTK_ANCHOR_N:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_S:
		x -= text->max_width / 2;
		clip_x -= text->clip_width / 2;
		break;

	case GTK_ANCHOR_NE:
	case GTK_ANCHOR_E:
	case GTK_ANCHOR_SE:
		x -= text->max_width;
		clip_x -= text->clip_width;
		break;
	}

	switch (text->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_N:
	case GTK_ANCHOR_NE:
		break;

	case GTK_ANCHOR_W:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_E:
		y -= text->height / 2;
		clip_y -= text->clip_height / 2;
		break;

	case GTK_ANCHOR_SW:
	case GTK_ANCHOR_S:
	case GTK_ANCHOR_SE:
		y -= text->height;
		clip_y -= text->clip_height;
		break;
	}

	/* Bounds */

	if (text->clip) {
		/* maybe do bbox intersection here? */
		*px1 = clip_x;
		*py1 = clip_y;
		*px2 = clip_x + text->clip_width;
		*py2 = clip_y + text->clip_height;
	} else {
		*px1 = x + private->min_lbearing;
		*py1 = y;
		*px2 = x + private->max_rbearing;
		*py2 = y + text->height;
	}
}

static void
get_bounds (GtkCanvasText *text, double *px1, double *py1, double *px2, double *py2)
{
	GtkCanvasItem *item;
	struct TextPrivate *private;
	double wx, wy;
	int i;

	private = (struct TextPrivate*) text->lines;
	if (!private) {
		split_into_lines (text);
		private = (struct TextPrivate*) text->lines;
	}

	item = GTK_CANVAS_ITEM (text);

	/* Get canvas pixel coordinates for text position */
	wx = text->x;
	wy = text->y;
	gtk_canvas_item_i2w (item, &wx, &wy);
	gtk_canvas_w2c (item->canvas, wx + text->xofs, wy + text->yofs, &text->cx, &text->cy);

	/* Get canvas pixel coordinates for clip rectangle position */
	gtk_canvas_w2c (item->canvas, wx, wy, &text->clip_cx, &text->clip_cy);
	text->clip_cwidth = text->clip_width * item->canvas->pixels_per_unit;
	text->clip_cheight = text->clip_height * item->canvas->pixels_per_unit;

	/* Calculate text dimensions */

	if (text->text && text->font)
		text->height = (text->font->ascent + text->font->descent) * text->num_lines;
	else
		text->height = 0;

	/* Anchor text */

	switch (text->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_W:
	case GTK_ANCHOR_SW:
		break;

	case GTK_ANCHOR_N:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_S:
		text->cx -= text->max_width / 2;
		text->clip_cx -= text->clip_cwidth / 2;
		break;

	case GTK_ANCHOR_NE:
	case GTK_ANCHOR_E:
	case GTK_ANCHOR_SE:
		text->cx -= text->max_width;
		text->clip_cx -= text->clip_cwidth;
		break;
	}

	switch (text->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_N:
	case GTK_ANCHOR_NE:
		break;

	case GTK_ANCHOR_W:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_E:
		text->cy -= text->height / 2;
		text->clip_cy -= text->clip_cheight / 2;
		break;

	case GTK_ANCHOR_SW:
	case GTK_ANCHOR_S:
	case GTK_ANCHOR_SE:
		text->cy -= text->height;
		text->clip_cy -= text->clip_cheight;
		break;
	}

	/* Bounds */

	if (text->clip) {
		*px1 = text->clip_cx;
		*py1 = text->clip_cy;
		*px2 = text->clip_cx + text->clip_cwidth;
		*py2 = text->clip_cy + text->clip_cheight;
	} else {
		*px1 = text->cx + private->min_lbearing;
		*py1 = text->cy;
		*px2 = text->cx + private->max_rbearing;
		*py2 = text->cy + text->height;
	}
}

/* Recalculates the bounding box of the text item.  The bounding box is defined
 * by the text's extents if the clip rectangle is disabled.  If it is enabled,
 * the bounding box is defined by the clip rectangle itself.
 */
static void
recalc_bounds (GtkCanvasText *text)
{
	GtkCanvasItem *item;

	item = GTK_CANVAS_ITEM (text);

	get_bounds (text, &item->x1, &item->y1, &item->x2, &item->y2);

	gtk_canvas_group_child_bounds (GTK_CANVAS_GROUP (item->parent), item);
}

/* Calculates the line widths (in pixels) of the text's splitted lines */
static void
calc_line_widths (GtkCanvasText *text)
{
	struct TextPrivate *private;
	struct line *lines;
	int i;

	private = (struct TextPrivate*)text->lines;
	if (!private) {
		split_into_lines (text);
		private = (struct TextPrivate*) text->lines;
	}

	lines = private->lines;
	text->max_width = 0;

	if (!lines)
		return;

	for (i = 0; i < text->num_lines; i++) {
		if (lines->length != 0) {
			if (text->font) {
				gint rbearing, lbearing;
				gint width, ascent, descent;

				gdk_text_extents (text->font,
						  lines->text,
						  lines->length,
						  &lbearing,
						  &rbearing,
						  &width,
						  &ascent,
						  &descent);
				lines->rbearing = rbearing;
				lines->lbearing = lbearing;
				lines->width = width;
			}
			else
			{
				lines->rbearing = 0;
				lines->lbearing = 0;
			}
			if ((lines->width) > text->max_width)
				text->max_width = lines->width;
			if (lines->rbearing > private->max_rbearing)
				private->max_rbearing = lines->rbearing;
			if (lines->lbearing < private->min_lbearing)
				private->min_lbearing = lines->lbearing;
		}
		
		lines++;
	}
}

/* Splits the text of the text item into lines */
static void
split_into_lines (GtkCanvasText *text)
{
	char *p;
	struct TextPrivate *private;
	struct line *lines;
	int len;

	/* Free old array of lines */

	private = (struct TextPrivate*)text->lines;
	if (private) {
		if (private->lines)
			g_free (private->lines);
		g_free (private);
	}
	text->lines = (gpointer)g_new0 (struct TextPrivate, 1);
	private = (struct TextPrivate *)text->lines;

	private->lines = NULL;
	text->num_lines = 0;

	if (!text->text)
		return;

	/* First, count the number of lines */

	for (p = text->text; *p; p++)
		if (*p == '\n')
			text->num_lines++;

	text->num_lines++;

	/* Allocate array of lines and calculate split positions */

	private->lines = lines = g_new0 (struct line, text->num_lines);
	len = 0;

	for (p = text->text; *p; p++) {
		if (*p == '\n') {
			lines->length = len;
			lines++;
			len = 0;
		} else if (len == 0) {
			len++;
			lines->text = p;
		} else
			len++;
	}

	lines->length = len;

	calc_line_widths (text);
}

/* Convenience function to set the text's GC's foreground color */
static void
set_text_gc_foreground (GtkCanvasText *text)
{
	GdkColor c;

	if (!text->gc)
		return;

	c.pixel = text->pixel;
	gdk_gc_set_foreground (text->gc, &c);
}

/* Sets the stipple pattern for the text */
static void
set_stipple (GtkCanvasText *text, GdkBitmap *stipple, int reconfigure)
{
	if (text->stipple && !reconfigure)
		gdk_bitmap_unref (text->stipple);

	text->stipple = stipple;
	if (stipple && !reconfigure)
		gdk_bitmap_ref (stipple);

	if (text->gc) {
		if (stipple) {
			gdk_gc_set_stipple (text->gc, stipple);
			gdk_gc_set_fill (text->gc, GDK_STIPPLED);
		} else
			gdk_gc_set_fill (text->gc, GDK_SOLID);
	}
}

/* Set_arg handler for the text item */
static void
gtk_canvas_text_set_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	GtkCanvasItem *item;
	GtkCanvasText *text;
	GdkColor color = { 0, 0, 0, 0, };
	GdkColor *pcolor;
	gboolean color_changed;
	int have_pixel;

	item = GTK_CANVAS_ITEM (object);
	text = GTK_CANVAS_TEXT (object);

	color_changed = FALSE;
	have_pixel = FALSE;

	switch (arg_id) {
	case ARG_TEXT:
		if (text->text)
			g_free (text->text);

		text->text = g_strdup (GTK_VALUE_STRING (*arg));
		split_into_lines (text);
		recalc_bounds (text);
		break;

	case ARG_X:
		text->x = GTK_VALUE_DOUBLE (*arg);
		recalc_bounds (text);
		break;

	case ARG_Y:
		text->y = GTK_VALUE_DOUBLE (*arg);
		recalc_bounds (text);
		break;

	case ARG_FONT:
		if (text->font)
			gdk_font_unref (text->font);

		text->font = gdk_font_load (GTK_VALUE_STRING (*arg));

		if (item->canvas->aa) {
			if (text->suckfont)
				gtk_canvas_suck_font_free (text->suckfont);

			text->suckfont = gtk_canvas_suck_font (text->font);
		}

		calc_line_widths (text);
		recalc_bounds (text);
		break;

	case ARG_FONTSET:
		if (text->font)
			gdk_font_unref (text->font);

		text->font = gdk_fontset_load (GTK_VALUE_STRING (*arg));

		if (item->canvas->aa) {
			if (text->suckfont)
				gtk_canvas_suck_font_free (text->suckfont);

			text->suckfont = gtk_canvas_suck_font (text->font);
		}

		calc_line_widths (text);
		recalc_bounds (text);
		break;

	case ARG_FONT_GDK:
		if (text->font)
			gdk_font_unref (text->font);

		text->font = GTK_VALUE_BOXED (*arg);
		gdk_font_ref (text->font);

		if (item->canvas->aa) {
			if (text->suckfont)
				gtk_canvas_suck_font_free (text->suckfont);

			text->suckfont = gtk_canvas_suck_font (text->font);
		}

		calc_line_widths (text);
		recalc_bounds (text);
		break;

	case ARG_ANCHOR:
		text->anchor = GTK_VALUE_ENUM (*arg);
		recalc_bounds (text);
		break;

	case ARG_JUSTIFICATION:
		text->justification = GTK_VALUE_ENUM (*arg);
		break;

	case ARG_CLIP_WIDTH:
		text->clip_width = fabs (GTK_VALUE_DOUBLE (*arg));
		recalc_bounds (text);
		break;

	case ARG_CLIP_HEIGHT:
		text->clip_height = fabs (GTK_VALUE_DOUBLE (*arg));
		recalc_bounds (text);
		break;

	case ARG_CLIP:
		text->clip = GTK_VALUE_BOOL (*arg);
		recalc_bounds (text);
		break;

	case ARG_X_OFFSET:
		text->xofs = GTK_VALUE_DOUBLE (*arg);
		recalc_bounds (text);
		break;

	case ARG_Y_OFFSET:
		text->yofs = GTK_VALUE_DOUBLE (*arg);
		recalc_bounds (text);
		break;

        case ARG_FILL_COLOR:
		if (GTK_VALUE_STRING (*arg))
			gdk_color_parse (GTK_VALUE_STRING (*arg), &color);

		text->rgba = ((color.red & 0xff00) << 16 |
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

		text->rgba = ((color.red & 0xff00) << 16 |
			      (color.green & 0xff00) << 8 |
			      (color.blue & 0xff00) |
			      0xff);
		color_changed = TRUE;
		break;

        case ARG_FILL_COLOR_RGBA:
		text->rgba = GTK_VALUE_UINT (*arg);
		color_changed = TRUE;
		break;

	case ARG_FILL_STIPPLE:
		set_stipple (text, GTK_VALUE_BOXED (*arg), FALSE);
		break;

	default:
		break;
	}

	if (color_changed) {
		if (have_pixel)
			text->pixel = color.pixel;
		else
			text->pixel = gtk_canvas_get_color_pixel (item->canvas, text->rgba);

		if (!item->canvas->aa)
			set_text_gc_foreground (text);

		gtk_canvas_item_request_update (item);
	}
}

/* Get_arg handler for the text item */
static void
gtk_canvas_text_get_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	GtkCanvasText *text;
	GdkColor *color;

	text = GTK_CANVAS_TEXT (object);

	switch (arg_id) {
	case ARG_TEXT:
		GTK_VALUE_STRING (*arg) = g_strdup (text->text);
		break;

	case ARG_X:
		GTK_VALUE_DOUBLE (*arg) = text->x;
		break;

	case ARG_Y:
		GTK_VALUE_DOUBLE (*arg) = text->y;
		break;

	case ARG_FONT_GDK:
		GTK_VALUE_BOXED (*arg) = text->font;
		break;

	case ARG_ANCHOR:
		GTK_VALUE_ENUM (*arg) = text->anchor;
		break;

	case ARG_JUSTIFICATION:
		GTK_VALUE_ENUM (*arg) = text->justification;
		break;

	case ARG_CLIP_WIDTH:
		GTK_VALUE_DOUBLE (*arg) = text->clip_width;
		break;

	case ARG_CLIP_HEIGHT:
		GTK_VALUE_DOUBLE (*arg) = text->clip_height;
		break;

	case ARG_CLIP:
		GTK_VALUE_BOOL (*arg) = text->clip;
		break;

	case ARG_X_OFFSET:
		GTK_VALUE_DOUBLE (*arg) = text->xofs;
		break;

	case ARG_Y_OFFSET:
		GTK_VALUE_DOUBLE (*arg) = text->yofs;
		break;

	case ARG_FILL_COLOR_GDK:
		color = g_new (GdkColor, 1);
		color->pixel = text->pixel;
		gdk_color_context_query_color (text->item.canvas->cc, color);
		GTK_VALUE_BOXED (*arg) = color;
		break;

	case ARG_FILL_COLOR_RGBA:
		GTK_VALUE_UINT (*arg) = text->rgba;
		break;

	case ARG_FILL_STIPPLE:
		GTK_VALUE_BOXED (*arg) = text->stipple;
		break;

	case ARG_TEXT_WIDTH:
		GTK_VALUE_DOUBLE (*arg) = text->max_width / text->item.canvas->pixels_per_unit;
		break;

	case ARG_TEXT_HEIGHT:
		GTK_VALUE_DOUBLE (*arg) = text->height / text->item.canvas->pixels_per_unit;
		break;

	default:
		arg->type = GTK_TYPE_INVALID;
		break;
	}
}

/* Update handler for the text item */
static void
gtk_canvas_text_update (GtkCanvasItem *item, double *affine, ArtSVP *clip_path, int flags)
{
	GtkCanvasText *text;
	double x1, y1, x2, y2;
	ArtDRect i_bbox, c_bbox;
	int i;

	text = GTK_CANVAS_TEXT (item);

	if (parent_class->update)
		(* parent_class->update) (item, affine, clip_path, flags);

	if (!item->canvas->aa) {
		set_text_gc_foreground (text);
		set_stipple (text, text->stipple, TRUE);
		get_bounds (text, &x1, &y1, &x2, &y2);

		gtk_canvas_update_bbox (item, x1, y1, x2, y2);
	} else {
		/* aa rendering */
		for (i = 0; i < 6; i++)
			text->affine[i] = affine[i];
		get_bounds_item_relative (text, &i_bbox.x0, &i_bbox.y0, &i_bbox.x1, &i_bbox.y1);
		art_drect_affine_transform (&c_bbox, &i_bbox, affine);
		gtk_canvas_update_bbox (item, c_bbox.x0, c_bbox.y0, c_bbox.x1, c_bbox.y1);

	}
}

/* Realize handler for the text item */
static void
gtk_canvas_text_realize (GtkCanvasItem *item)
{
	GtkCanvasText *text;

	text = GTK_CANVAS_TEXT (item);

	if (parent_class->realize)
		(* parent_class->realize) (item);

	text->gc = gdk_gc_new (item->canvas->layout.bin_window);
}

/* Unrealize handler for the text item */
static void
gtk_canvas_text_unrealize (GtkCanvasItem *item)
{
	GtkCanvasText *text;

	text = GTK_CANVAS_TEXT (item);

	gdk_gc_unref (text->gc);
	text->gc = NULL;

	if (parent_class->unrealize)
		(* parent_class->unrealize) (item);
}

/* Calculates the x position of the specified line of text, based on the text's justification */
static double
get_line_xpos_item_relative (GtkCanvasText *text, struct line *line)
{
	double x;

	x = text->x;

	switch (text->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_W:
	case GTK_ANCHOR_SW:
		break;

	case GTK_ANCHOR_N:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_S:
		x -= text->max_width / 2;
		break;

	case GTK_ANCHOR_NE:
	case GTK_ANCHOR_E:
	case GTK_ANCHOR_SE:
		x -= text->max_width;
		break;
	}

	switch (text->justification) {
	case GTK_JUSTIFY_RIGHT:
		x += text->max_width - line->width;
		break;

	case GTK_JUSTIFY_CENTER:
		x += (text->max_width - line->width) * 0.5;
		break;

	default:
		/* For GTK_JUSTIFY_LEFT, we don't have to do anything.  We do not support
		 * GTK_JUSTIFY_FILL, yet.
		 */
		break;
	}

	return x;
}

/* Calculates the y position of the first line of text. */
static double
get_line_ypos_item_relative (GtkCanvasText *text)
{
	double x, y;

	y = text->y;

	switch (text->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_N:
	case GTK_ANCHOR_NE:
		break;

	case GTK_ANCHOR_W:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_E:
		y -= text->height / 2;
		break;

	case GTK_ANCHOR_SW:
	case GTK_ANCHOR_S:
	case GTK_ANCHOR_SE:
		y -= text->height;
		break;
	}

	return y;
}

/* Calculates the x position of the specified line of text, based on the text's justification */
static int
get_line_xpos (GtkCanvasText *text, struct line *line)
{
	int x;

	x = text->cx;

	switch (text->justification) {
	case GTK_JUSTIFY_RIGHT:
		x += text->max_width - line->width;
		break;

	case GTK_JUSTIFY_CENTER:
		x += (text->max_width - line->width) / 2;
		break;

	default:
		/* For GTK_JUSTIFY_LEFT, we don't have to do anything.  We do not support
		 * GTK_JUSTIFY_FILL, yet.
		 */
		break;
	}

	return x;
}

/* Draw handler for the text item */
static void
gtk_canvas_text_draw (GtkCanvasItem *item, GdkDrawable *drawable,
			int x, int y, int width, int height)
{
	GtkCanvasText *text;
	GdkRectangle rect;
	struct TextPrivate *private;
	struct line *lines;
	int i;
	int xpos, ypos;

	text = GTK_CANVAS_TEXT (item);

	if (!text->text || !text->font)
		return;

	if (text->clip) {
		rect.x = text->clip_cx - x;
		rect.y = text->clip_cy - y;
		rect.width = text->clip_cwidth;
		rect.height = text->clip_cheight;

		gdk_gc_set_clip_rectangle (text->gc, &rect);
	}

	private = (struct TextPrivate*)text->lines;
	lines = private->lines;
	ypos = text->cy + text->font->ascent;

	if (text->stipple)
		gtk_canvas_set_stipple_origin (item->canvas, text->gc);

	for (i = 0; i < text->num_lines; i++) {
		if (lines->length != 0) {
			xpos = get_line_xpos (text, lines);

			gdk_draw_text (drawable,
				       text->font,
				       text->gc,
				       xpos - x,
				       ypos - y,
				       lines->text,
				       lines->length);
		}

		ypos += text->font->ascent + text->font->descent;
		lines++;
	}

	if (text->clip)
		gdk_gc_set_clip_rectangle (text->gc, NULL);
}

/* Render handler for the text item */
static void
gtk_canvas_text_render (GtkCanvasItem *item, GtkCanvasBuf *buf)
{
	GtkCanvasText *text;
	guint32 fg_color;
	double x_start, y_start;
	double xpos, ypos;
	struct TextPrivate *private;
	struct line *lines;
	int i, j;
	double affine[6];
	GtkCanvasTextSuckFont *suckfont;
	int dx, dy;
	ArtPoint start_i, start_c;

	text = GTK_CANVAS_TEXT (item);

	if (!text->text || !text->font || !text->suckfont)
		return;

	suckfont = text->suckfont;

	fg_color = text->rgba;

        gtk_canvas_buf_ensure_buf (buf);

	private = (struct TextPrivate *)text->lines;
	lines = private->lines;
	start_i.y = get_line_ypos_item_relative (text);

	art_affine_scale (affine, item->canvas->pixels_per_unit, item->canvas->pixels_per_unit);
	for (i = 0; i < 6; i++)
		affine[i] = text->affine[i];

	for (i = 0; i < text->num_lines; i++) {
		if (lines->length != 0) {
			start_i.x = get_line_xpos_item_relative (text, lines);
			art_affine_point (&start_c, &start_i, text->affine);
			xpos = start_c.x;
			ypos = start_c.y;

			for (j = 0; j < lines->length; j++) {
				GtkCanvasTextSuckChar *ch;

				ch = &suckfont->chars[(unsigned char)((lines->text)[j])];

				affine[4] = xpos;
				affine[5] = ypos;
				art_rgb_bitmap_affine (
					buf->buf,
					buf->rect.x0, buf->rect.y0, buf->rect.x1, buf->rect.y1,
					buf->buf_rowstride,
					suckfont->bitmap + (ch->bitmap_offset >> 3),
					ch->width,
					suckfont->bitmap_height,
					suckfont->bitmap_width >> 3,
					fg_color,
					affine,
					ART_FILTER_NEAREST, NULL);

				dx = ch->left_sb + ch->width + ch->right_sb;
				xpos += dx * affine[0];
				ypos += dx * affine[1];
			}
		}

		dy = text->font->ascent + text->font->descent;
		start_i.y += dy;
		lines++;
	}

	buf->is_bg = 0;
}

/* Point handler for the text item */
static double
gtk_canvas_text_point (GtkCanvasItem *item, double x, double y,
			 int cx, int cy, GtkCanvasItem **actual_item)
{
	GtkCanvasText *text;
	int i;
	struct TextPrivate *private;
	struct line *lines;
	int x1, y1, x2, y2;
	int font_height;
	int dx, dy;
	double dist, best;

	text = GTK_CANVAS_TEXT (item);

	*actual_item = item;

	/* The idea is to build bounding rectangles for each of the lines of
	 * text (clipped by the clipping rectangle, if it is activated) and see
	 * whether the point is inside any of these.  If it is, we are done.
	 * Otherwise, calculate the distance to the nearest rectangle.
	 */

	if (text->font)
		font_height = text->font->ascent + text->font->descent;
	else
		font_height = 0;

	best = 1.0e36;

	private = (struct TextPrivate *)text->lines;
	lines = private->lines;

	for (i = 0; i < text->num_lines; i++) {
		/* Compute the coordinates of rectangle for the current line,
		 * clipping if appropriate.
		 */

		x1 = get_line_xpos (text, lines);
		y1 = text->cy + i * font_height;
		x2 = x1 + lines->width;
		y2 = y1 + font_height;

		if (text->clip) {
			if (x1 < text->clip_cx)
				x1 = text->clip_cx;

			if (y1 < text->clip_cy)
				y1 = text->clip_cy;

			if (x2 > (text->clip_cx + text->clip_width))
				x2 = text->clip_cx + text->clip_width;

			if (y2 > (text->clip_cy + text->clip_height))
				y2 = text->clip_cy + text->clip_height;

			if ((x1 >= x2) || (y1 >= y2))
				continue;
		}

		/* Calculate distance from point to rectangle */

		if (cx < x1)
			dx = x1 - cx;
		else if (cx >= x2)
			dx = cx - x2 + 1;
		else
			dx = 0;

		if (cy < y1)
			dy = y1 - cy;
		else if (cy >= y2)
			dy = cy - y2 + 1;
		else
			dy = 0;

		if ((dx == 0) && (dy == 0))
			return 0.0;

		dist = sqrt (dx * dx + dy * dy);
		if (dist < best)
			best = dist;

		/* Next! */

		lines++;
	}

	return best / item->canvas->pixels_per_unit;
}

/* Bounds handler for the text item */
static void
gtk_canvas_text_bounds (GtkCanvasItem *item, double *x1, double *y1, double *x2, double *y2)
{
	GtkCanvasText *text;
	struct TextPrivate *private;
	double width, height;

	text = GTK_CANVAS_TEXT (item);
	private = (struct TextPrivate*) text->lines;

	*x1 = text->x + private->min_lbearing;
	*y1 = text->y;

	if (text->clip) {
		width = text->clip_width;
		height = text->clip_height;
	} else {
		width = text->max_width / item->canvas->pixels_per_unit;
		height = text->height / item->canvas->pixels_per_unit;
	}

	switch (text->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_W:
	case GTK_ANCHOR_SW:
		break;

	case GTK_ANCHOR_N:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_S:
		*x1 -= width / 2.0;
		break;

	case GTK_ANCHOR_NE:
	case GTK_ANCHOR_E:
	case GTK_ANCHOR_SE:
		*x1 -= width;
		break;
	}

	switch (text->anchor) {
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_N:
	case GTK_ANCHOR_NE:
		break;

	case GTK_ANCHOR_W:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_E:
		*y1 -= height / 2.0;
		break;

	case GTK_ANCHOR_SW:
	case GTK_ANCHOR_S:
	case GTK_ANCHOR_SE:
		*y1 -= height;
		break;
	}

	*x2 = *x1 + private->max_rbearing;
	*y2 = *y1 + height;
}



/* Routines for sucking fonts from the X server */

static GtkCanvasTextSuckFont *
gtk_canvas_suck_font (GdkFont *font)
{
	GtkCanvasTextSuckFont *suckfont;
	int i;
	int x, y;
	char text[1];
	int lbearing, rbearing, ch_width, ascent, descent;
	GdkPixmap *pixmap;
	GdkColor black, white;
	GdkImage *image;
	GdkGC *gc;
	guchar *bitmap, *line;
	int width, height;
	int black_pixel, pixel;

	if (!font)
		return NULL;

	suckfont = g_new (GtkCanvasTextSuckFont, 1);

	height = font->ascent + font->descent;
	x = 0;
	for (i = 0; i < 256; i++) {
		text[0] = i;
		gdk_text_extents (font, text, 1,
				  &lbearing, &rbearing, &ch_width, &ascent, &descent);
		suckfont->chars[i].left_sb = lbearing;
		suckfont->chars[i].right_sb = ch_width - rbearing;
		suckfont->chars[i].width = rbearing - lbearing;
		suckfont->chars[i].ascent = ascent;
		suckfont->chars[i].descent = descent;
		suckfont->chars[i].bitmap_offset = x;
		x += (ch_width + 31) & -32;
	}

	width = x;

	suckfont->bitmap_width = width;
	suckfont->bitmap_height = height;
	suckfont->ascent = font->ascent;

	pixmap = gdk_pixmap_new (NULL, suckfont->bitmap_width,
				 suckfont->bitmap_height, 1);
	gc = gdk_gc_new (pixmap);
	gdk_gc_set_font (gc, font);

	black_pixel = BlackPixel (gdk_display, DefaultScreen (gdk_display));
	black.pixel = black_pixel;
	white.pixel = WhitePixel (gdk_display, DefaultScreen (gdk_display));
	gdk_gc_set_foreground (gc, &white);
	gdk_draw_rectangle (pixmap, gc, 1, 0, 0, width, height);

	gdk_gc_set_foreground (gc, &black);
	for (i = 0; i < 256; i++) {
		text[0] = i;
		gdk_draw_text (pixmap, font, gc,
			       suckfont->chars[i].bitmap_offset - suckfont->chars[i].left_sb,
			       font->ascent,
			       text, 1);
	}

	/* The handling of the image leaves me with distinct unease.  But this
	 * is more or less copied out of gimp/app/text_tool.c, so it _ought_ to
	 * work. -RLL
	 */

	image = gdk_image_get (pixmap, 0, 0, width, height);
	suckfont->bitmap = g_malloc0 ((width >> 3) * height);

	line = suckfont->bitmap;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pixel = gdk_image_get_pixel (image, x, y);
			if (pixel == black_pixel)
				line[x >> 3] |= 128 >> (x & 7);
		}
		line += width >> 3;
	}

	gdk_image_destroy (image);

	/* free the pixmap */
	gdk_pixmap_unref (pixmap);

	/* free the gc */
	gdk_gc_destroy (gc);

	return suckfont;
}

static void
gtk_canvas_suck_font_free (GtkCanvasTextSuckFont *suckfont)
{
	g_free (suckfont->bitmap);
	g_free (suckfont);
}
