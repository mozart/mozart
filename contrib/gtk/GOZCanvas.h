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
 * Lowlevel Gtk Canvas Helper
 */
static GtkCanvasPoints *native_internal_allocate_points(OZ_Term ls) {
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

OZ_BI_define(native_allocate_canvas_points, 1, 1) {
  OZ_declareTerm(0, ls);
  OZ_out(0) = OZ_makeForeignPointer(native_internal_allocate_points(ls));
  return OZ_ENTAILED;
} OZ_BI_end

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
 * Due to a bug in the GtkCanvas Implementation, it is necessary
 * to provide these functions. Splitting the parameters yields seg-fault.
 */

OZ_BI_define(native_new_polygon_item, 3, 1) {
  GOZ_declareForeignType(GtkCanvasGroup *, 0, group);
  GOZ_declareInt(1, type);
  OZ_declareTerm(2, ls);
  GtkCanvasPoints *pts = native_internal_allocate_points(ls);
  GtkCanvasItem *ret;
  ret = gtk_canvas_item_new(group, (GtkType) type, "points", pts, NULL);
  /* Item imports a copy; therefore it can be freed */
  gtk_canvas_points_free(pts);
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define(native_new_image_item, 7, 1) {
  static GtkType type = gtk_canvas_image_get_type();
  GOZ_declareForeignType(GtkCanvasGroup *, 0, group);
  GOZ_declareForeignType(GdkImlibImage *, 1, image);
  GOZ_declareInt(2, x);
  GOZ_declareInt(3, y);
  GOZ_declareInt(4, width);
  GOZ_declareInt(5, height);
  GOZ_declareInt(6, anchor);
  GtkCanvasItem *ret;
  if (image == NULL) {
    fprintf(stderr, "new_image_item: illegal image ptr '%p'\n", image);
    ret = NULL;
  }
  else {
    ret = gtk_canvas_item_new(group, type,
                              "image", image,
                              "x", (double) x,
                              "y", (double) y,
                              "width", (double) width,
                              "height", (double) height,
                              "anchor", (GtkAnchorType) anchor, NULL);
  }
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

/* These routines are provided for optimisation. */

OZ_BI_define (native_new_text_item, 7, 1) {
  static GtkType type = gtk_canvas_text_get_type();
  GOZ_declareForeignType(GtkCanvasGroup *, 0, group);
  GOZ_declareString(1, text);
  GOZ_declareInt(2, x);
  GOZ_declareInt(3, y);
  GOZ_declareInt(4, anchor);
  GOZ_declareForeignType(GdkFont *, 5, font);
  GOZ_declareForeignType(GdkColor *, 6, color);
  GtkCanvasItem *ret;
  ret = gtk_canvas_item_new(group, type,
                            "text", text,
                            "x", (double) x,
                            "y", (double) y,
                            "anchor", (GtkAnchorType) anchor,
                            "font_gdk", font,
                            "fill_color_gdk", color, NULL);
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (native_new_tag_item, 3, 1) {
  static GtkType type = gtk_canvas_group_get_type();
  GOZ_declareForeignType(GtkCanvasGroup *, 0, group);
  GOZ_declareInt(1, x);
  GOZ_declareInt(2, y);
  GtkCanvasItem *ret;
  ret = gtk_canvas_item_new(group, type,
                            "x", (double) x,
                            "y", (double) y, NULL);
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (native_new_simple_tag_item, 1, 1) {
  static GtkType type = gtk_canvas_group_get_type();
  GOZ_declareForeignType(GtkCanvasGroup *, 0, group);
  GtkCanvasItem *ret;
  ret = gtk_canvas_item_new(group, type,
                            "x", (double) 0,
                            "y", (double) 0, NULL);
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

#endif
