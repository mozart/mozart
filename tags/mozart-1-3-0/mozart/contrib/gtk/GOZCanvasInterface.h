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
  {"pushVisual", 0, 0, native_push_visual},
  {"popVisual", 0, 0, native_pop_visual},
  {"itemNew", 2, 1, native_item_new},
  {"polygonNew", 3, 1, native_polygon_new},
  {"imageNew", 7, 1, native_image_new},
  {"groupNew", 1, 1, native_item_group_new},
  {"textNew", 7, 1, native_item_text_new},
  {"widgetNew", 7, 1, native_item_widget_new},
  {"rectangleNew", 9, 1, native_item_rectangle_new},
  {"lineNew", 5, 1, native_item_line_new},
  {"fastPolygonNew", 5, 1, native_item_fastpolygon_new},
  {"pointsNew", 1, 1, native_points_new},
  {"colorNew", 1, 1, native_item_color_new},
  {"itemSet", 3, 0, native_item_set},
  {"itemPosition", 1, 2, native_item_position},
  {"itemMoveTo", 3, 0, native_item_move_to},
  {"itemRaise", 2, 0, native_item_raise},
  {"itemLower", 2, 0, native_item_lower},
  {"itemDestroy", 1, 0, native_item_destroy},
  {"inspectorTextNew", 6, 1, native_inspector_text_new},
  {"inspectorImageNew", 6, 1, native_inspector_image_new},
#endif
