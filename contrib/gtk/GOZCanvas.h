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

OZ_BI_define(native_points_put, 3, 0) {
  OZ_declareForeignType(0, val, GtkCanvasPoints *);
  OZ_declareInt(1, i);
  OZ_declareInt(2, x);
  val->coords[i] = x;
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
 * to provide this function. Splitting the parameters yields seg-fault.
 */

OZ_BI_define(native_new_image_item, 7, 1) {
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
    ret = gtk_canvas_item_new(group, gtk_canvas_image_get_type(),
			      "image", image,
			      "x", (double) x,
			      "y", (double) y,
			      "width", (double) width,
			      "height", (double) height,
			      "anchor", anchor, NULL);
  }
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

#endif
