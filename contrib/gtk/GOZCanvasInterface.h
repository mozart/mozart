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

#if defined(__GOZ_CANVAS_H__)
  {"allocateCanvasPoints", 1, 1, native_allocate_canvas_points},
  {"pushVisual", 0, 0, native_push_visual},
  {"popVisual", 0, 0, native_pop_visual},
  {"newPolygonItem", 3, 1, native_new_polygon_item},
  {"newImageItem", 7, 1, native_new_image_item},
  {"newTextItem", 7, 1, native_new_text_item},
  {"newTagItem", 3, 1, native_new_tag_item},
  {"newSimpleTagItem", 1, 1, native_new_simple_tag_item},
#endif
