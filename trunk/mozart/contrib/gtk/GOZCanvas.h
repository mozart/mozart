/*
 * Author:
 *   Thorsten Brunklaus <brunklaus@ps.uni-sb.de>
 *
 * Copyright:
 *   Thorsten Brunklaus, 2001
 *
 * Last Change:
 *   $Date$ by $Author$
 *   $Revision$
 *
 * This file is part of Mozart, an implementation of Oz 3:
 *   http://www.mozart-oz.org
 *
 * See the file "LICENSE" or
 *   http://www.mozart-oz.org/LICENSE.html
 * for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#ifndef __GOZ_CANVAS_H__
#define __GOZ_CANVAS_H__

/*
 * Canvas visual handling
 */
OZ_BI_define(native_push_visual, 0, 0) {
  gtk_widget_push_visual(gdk_imlib_get_visual());
  gtk_widget_push_colormap(gdk_imlib_get_colormap());
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define(native_pop_visual, 0, 0) {
  gtk_widget_pop_colormap();
  gtk_widget_pop_visual();
  return OZ_ENTAILED;
} OZ_BI_end

/*
 * Canvas Item Handling
 */

/* Canvas Item Types */
typedef enum {
  CANVAS_TEXT    = 0,
  CANVAS_GROUP   = 1,
  CANVAS_WIDGET  = 2,
  CANVAS_RECT    = 3,
  CANVAS_LINE    = 4,
  CANVAS_ELLIPSE = 5,
  CANVAS_POLYGON = 6,
  CANVAS_IMAGE   = 7
} OZ_CANVAS_TYPE;

/* Static Data Area */
static GtkType text_type;
static GtkType group_type;
static GtkType widget_type;
static GtkType rect_type;
static GtkType line_type;
static GtkType ellipse_type;
static GtkType polygon_type;
static GtkType image_type;
static GdkColormap *colormap;

/* Create canvas points from Oz list */
static inline
GtkCanvasPoints *oz_alloc_points(OZ_Term ls) {
  unsigned int i = 0;
  OZ_Term bls    = ls;
  GtkCanvasPoints *pts;
  /* Compute array length  */
  while (OZ_isCons(bls)) {
    bls = OZ_tail(bls);
    i++;
  }
  /* Alloc points structure */
  pts = gtk_canvas_points_new((i >> 1));
  i = 0;
  /* Insert values */
  while (OZ_isCons(ls)) {
    pts->coords[i++] = (double) OZ_intToC(OZ_head(ls));
    ls = OZ_tail(ls);
  }
  return pts;
}

/* Build a canvas image item; requires all parameters given at once */
static inline
GtkCanvasItem *oz_new_image_item(GtkCanvasGroup *group,
				 GdkImlibImage *image,
				 int x,
				 int y,
				 int width,
				 int height,
				 int anchor) {
  return gtk_canvas_item_new(group, image_type,
			     "image", image,
			     "x", (double) x,
			     "y", (double) y,
			     "width", (double) width,
			     "height", (double) height,
			     "anchor", (GtkAnchorType) anchor, NULL);
}

/*
 * Generic Canvas Item Construction
 */

/* Create a standard item (text, group, widget, rectangle or ellipse) */
OZ_BI_define (native_item_new, 2, 1) {
  GOZ_declareForeignType(GtkCanvasGroup *, 0, group);
  GOZ_declareInt(1, type);
  GtkCanvasItem *ret;
  switch ((OZ_CANVAS_TYPE) type) {
  case CANVAS_TEXT:
    ret = gtk_canvas_item_new(group, text_type, NULL); break;
  case CANVAS_GROUP:
    ret = gtk_canvas_item_new(group, group_type, NULL); break;
  case CANVAS_RECT:
    ret = gtk_canvas_item_new(group, rect_type, NULL); break;
  case CANVAS_ELLIPSE:
    ret = gtk_canvas_item_new(group, ellipse_type, NULL); break;
  case CANVAS_WIDGET:
    ret = gtk_canvas_item_new(group, widget_type, NULL); break;
  default:
    fprintf(stderr, "native_item_new: illegal item type\n");
    ret = NULL;
  }
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

/* Create polygon items (line, polygon) */
OZ_BI_define(native_polygon_new, 3, 1) {
  GOZ_declareForeignType(GtkCanvasGroup *, 0, group);
  GOZ_declareInt(1, type);
  OZ_declareTerm(2, points);
  GtkCanvasPoints *pts = oz_alloc_points(points);
  GtkCanvasItem *ret;
  switch ((OZ_CANVAS_TYPE) type) {
  case CANVAS_LINE:
    ret = gtk_canvas_item_new(group, line_type, "points", pts, NULL); break;
  case CANVAS_POLYGON:
    ret = gtk_canvas_item_new(group, polygon_type, "points", pts, NULL); break;
  default:
    fprintf(stderr, "native_polygon_new: illegal item type\n");
    ret = NULL;
    break;
  }
  /* Canvas create copy; therefore we can free here */
  gtk_canvas_points_free(pts);
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

/* Create a image item (needs entire paramter set to be given */
OZ_BI_define(native_image_new, 7, 1) {
  GOZ_declareForeignType(GtkCanvasGroup *, 0, group);
  GOZ_declareForeignType(GdkImlibImage *, 1, image);
  GOZ_declareInt(2, x);
  GOZ_declareInt(3, y);
  GOZ_declareInt(4, width);
  GOZ_declareInt(5, height);
  GOZ_declareInt(6, anchor);
  GtkCanvasItem *ret;
  ret = oz_new_image_item(group, image, x, y, width, height, anchor);
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

/*
 * Fast item creation
 */

OZ_BI_define(native_item_group_new, 1, 1) {
  GOZ_declareForeignType(GtkCanvasGroup *, 0, group);
  GtkCanvasItem *ret;
  ret = gtk_canvas_item_new(group, group_type, NULL);
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define(native_item_text_new, 7, 1) {
  GOZ_declareForeignType(GtkCanvasGroup *, 0, group);
  GOZ_declareString(1, text);
  GOZ_declareInt(2, x);
  GOZ_declareInt(3, y);
  GOZ_declareInt(4, anchor);
  GOZ_declareForeignType(GdkFont *, 5, font);
  GOZ_declareForeignType(GdkColor *, 6, color);
  GtkCanvasItem *ret;
  ret = gtk_canvas_item_new(group, text_type,
			    "text", text,
			    "x", (double) x,
			    "y", (double) y,
			    "anchor", (GtkAnchorType) anchor,
			    "font_gdk", font,
			    "fill_color_gdk", color, NULL);
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define(native_item_widget_new, 7, 1) {
  GOZ_declareForeignType(GtkCanvasGroup *, 0, group);
  GOZ_declareForeignType(GtkWidget *, 1, widget);
  GOZ_declareInt(2, x);
  GOZ_declareInt(3, y);
  GOZ_declareInt(4, width);
  GOZ_declareInt(5, height);
  GOZ_declareInt(6, anchor);
  GtkCanvasItem *ret;
  ret = gtk_canvas_item_new(group, widget_type,
			    "widget", widget,
			    "x", (double) x,
			    "y", (double) y,
			    "width", (double) width,
			    "height", (double) height,
			    "anchor", (GtkAnchorType) anchor, NULL);
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

/* Includes rectangle and ellipse items */
OZ_BI_define(native_item_rectangle_new, 9, 1) {
  GOZ_declareForeignType(GtkCanvasGroup *, 0, group);
  GOZ_declareInt(1, x1);
  GOZ_declareInt(2, y1);
  GOZ_declareInt(3, x2);
  GOZ_declareInt(4, y2);
  GOZ_declareForeignType(GdkColor *, 5, fill_color);
  GOZ_declareForeignType(GdkColor *, 6, outline_color);
  GOZ_declareInt(7, width);
  GOZ_declareInt(8, isRect);
  GtkType type = (isRect == 0 ? ellipse_type : rect_type);
  GtkCanvasItem *ret;
  ret = gtk_canvas_item_new(group, type,
			    "x1", (double) x1,
			    "y1", (double) y1,
			    "x2", (double) x2,
			    "y2", (double) y2,
			    "fill_color_gdk", fill_color,
			    "outline_color_gdk", outline_color,
			    "width_pixels", width, NULL);
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define(native_item_line_new, 5, 1) {
  GOZ_declareForeignType(GtkCanvasGroup *, 0, group);
  OZ_declareTerm(1, points);
  GOZ_declareForeignType(GdkColor *, 2, fill_color);
  GOZ_declareInt(3, line_style);
  GOZ_declareInt(4, width);
  GtkCanvasItem *ret;
  GtkCanvasPoints *pts = oz_alloc_points(points);
  ret = gtk_canvas_item_new(group, line_type,
			    "points", pts,
			    "fill_color_gdk", fill_color,
			    "line_style", (GdkLineStyle) line_style,
			    "width_pixels", width, NULL);
  /* Canvas imports copy; therefore we can free here */
  gtk_canvas_points_free(pts);
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define(native_item_fastpolygon_new, 5, 1) {
  GOZ_declareForeignType(GtkCanvasGroup *, 0, group);
  OZ_declareTerm(1, points);
  GOZ_declareForeignType(GdkColor *, 2, fill_color);
  GOZ_declareForeignType(GdkColor *, 3, outline_color);
  GOZ_declareInt(4, width);
  GtkCanvasItem *ret;
  GtkCanvasPoints *pts = oz_alloc_points(points);
  ret = gtk_canvas_item_new(group, polygon_type,
			    "points", pts,
			    "fill_color_gdk", fill_color,
			    "outline_color_gdk", outline_color,
			    "width_pixels", width, NULL);
  /* Canvas imports copy; therefore we can free here */
  gtk_canvas_points_free(pts);
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

/*
 * Item configuration
 */

/* Set argument value (requires unwrapped item argument) */
OZ_BI_define(native_item_set, 3, 0) {
  GOZ_declareForeignType(GtkCanvasItem *, 0, item);
  GOZ_declareString(1, arg);
  OZ_declareTerm(2, value);
  if (OZ_isInt(value)) {
    gtk_canvas_item_set(item, arg, OZ_intToC(value), NULL);
  } else if (OZ_isFloat(value)) {
    gtk_canvas_item_set(item, arg, OZ_floatToC(value), NULL);
  } else if (OZ_isForeignPointer(value)) {
    gtk_canvas_item_set(item, arg, OZ_getForeignPointer(value), NULL);
  } else if (OZ_isVirtualString(value, NULL)) {
    arg = g_strdup(arg);
    gtk_canvas_item_set(item, arg, g_strdup(GOZ_stringToC(value)), NULL);
    g_free(arg);
  } else {
    /* We expect canvas points */
    GtkCanvasPoints *pts = oz_alloc_points(value);
    gtk_canvas_item_set(item, arg, pts, NULL);
    /* to be checked: freeing here */
  }
  return OZ_ENTAILED;
} OZ_BI_end

/* Create native points */
OZ_BI_define(native_points_new, 1, 1) {
  OZ_declareTerm(0, points);
  OZ_out(0) = OZ_makeForeignPointer(oz_alloc_points(points));
  return OZ_ENTAILED;
} OZ_BI_end

/* Create item color */
OZ_BI_define(native_item_color_new, 1, 1) {
  GOZ_declareString(0, desc);
  GdkColor *color = (GdkColor *) malloc(sizeof(GdkColor));
  gdk_color_parse(desc, color);
  gdk_colormap_alloc_color(colormap, color, 0, 1);
  OZ_out(0) = OZ_makeForeignPointer(color);
  return OZ_ENTAILED;
} OZ_BI_end

/* Query item position */
OZ_BI_define (native_item_position, 1, 2) {
  GOZ_declareForeignType(GtkCanvasItem *, 0, item);
  double x1, y1, x2, y2;
  gtk_canvas_item_get_bounds(item, &x1, &y1, &x2, &y2);
  OZ_out(0) = OZ_int((int) x1);
  OZ_out(1) = OZ_int((int) y1);
  return OZ_ENTAILED;
} OZ_BI_end

/* Item Placement (moves entire tree to absolute x/y) */
OZ_BI_define (native_item_move_to, 3, 0) {
  GOZ_declareForeignType(GtkCanvasItem *, 0, item);
  GOZ_declareInt(1, x);
  GOZ_declareInt(2, y);
  double x1, y1, x2, y2, dx, dy;
  gtk_canvas_item_get_bounds(item, &x1, &y1, &x2, &y2);
  dx = ((double) x - x1);
  dy = ((double) y - y1);
  if ((dx != 0.0) || (dy != 0.0)) {
    gtk_canvas_item_move(item, dx, dy);
  }
  return OZ_ENTAILED;
} OZ_BI_end

/* Raise item in stack (0 = raiseToTop) */
OZ_BI_define(native_item_raise, 2, 0) {
  GOZ_declareForeignType(GtkCanvasItem *, 0, item);
  GOZ_declareInt(1, level);
  if (level == 0) {
    gtk_canvas_item_raise_to_top(item);
  }
  else {
    gtk_canvas_item_raise(item, level);
  }
  return OZ_ENTAILED;
} OZ_BI_end

/* Lower item in stack (0 = lowerToBottom) */
OZ_BI_define(native_item_lower, 2, 0) {
  GOZ_declareForeignType(GtkCanvasItem *, 0, item);
  GOZ_declareInt(1, level);
  if (level == 0) {
    gtk_canvas_item_lower_to_bottom(item);
  }
  else {
    gtk_canvas_item_lower(item, level);
  }
  return OZ_ENTAILED;
} OZ_BI_end

/* Item Destruction */
OZ_BI_define (native_item_destroy, 1, 0) {
  GOZ_declareForeignType(GtkObject *, 0, object);
  gtk_object_destroy(object);
  return OZ_ENTAILED;
} OZ_BI_end

/* Inspector Tool Optimisations */
OZ_BI_define(native_inspector_text_new, 6, 1) {
  GOZ_declareForeignType(GtkCanvasGroup *, 0, parent);
  GOZ_declareString(1, text);
  GOZ_declareInt(2, x);
  GOZ_declareInt(3, y);
  GOZ_declareForeignType(GdkFont *, 4, font);
  GOZ_declareForeignType(GdkColor *, 5, color);
  GtkCanvasItem *group = gtk_canvas_item_new(parent, group_type, NULL);
  gtk_canvas_item_new((GtkCanvasGroup *) group, text_type,
		      "text", text,
		      "x", (double) x,
		      "y", (double) y,
		      "anchor", GTK_ANCHOR_NORTH_WEST,
		      "font_gdk", font,
		      "fill_color_gdk", color, NULL);
  /* The inspector only needs the tag */
  OZ_out(0) = OZ_makeForeignPointer(group);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define(native_inspector_image_new, 6, 1) {
  GOZ_declareForeignType(GtkCanvasGroup *, 0, parent);
  GOZ_declareForeignType(GdkImlibImage *, 1, image);
  GOZ_declareInt(2, x);
  GOZ_declareInt(3, y);
  GOZ_declareInt(4, width);
  GOZ_declareInt(5, height);
  GtkCanvasItem *group = gtk_canvas_item_new(parent, group_type, NULL);
  oz_new_image_item((GtkCanvasGroup *) group,
		    image, x, y, width, height, GTK_ANCHOR_NORTH_WEST);
  /* The inspector only needs the tag */
  OZ_out(0) = OZ_makeForeignPointer(group);
  return OZ_ENTAILED;
} OZ_BI_end

/* System Init */

static void native_system_type_init() {
  text_type    = gtk_canvas_text_get_type();
  group_type   = gtk_canvas_group_get_type();
  widget_type  = gtk_canvas_widget_get_type();
  rect_type    = gtk_canvas_rect_get_type();
  line_type    = gtk_canvas_line_get_type();
  ellipse_type = gtk_canvas_ellipse_get_type();
  polygon_type = gtk_canvas_polygon_get_type();
  image_type   = gtk_canvas_image_get_type();
  colormap     = gdk_colormap_get_system();
}

#endif
