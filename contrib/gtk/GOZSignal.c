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

#include <mozart.h>
#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include "GOZData.h"

/*
 * Signal Handling/Marshalling from Host Language <-> G(D|T)K
 */

static OZ_Term signal_port = 0;

OZ_BI_define (native_initialize_signal_port, 1, 0) {
  OZ_declareTerm (0, port);
  if (signal_port == 0) {
    OZ_protect(&signal_port); /* prevent GC of port anchor */
  }
  signal_port = port;
  return OZ_ENTAILED;
} OZ_BI_end


/*
 * Process all events in the queue and tell the host side
 * whether there were events or not
 */

OZ_BI_define (native_handle_pending_events, 0, 1) {
  int had_events = 0;

  while (gtk_events_pending()) {
    had_events = 1;
    gtk_main_iteration();
  }
  OZ_out(0) = (had_events ? OZ_true() : OZ_false());
  return OZ_ENTAILED;
} OZ_BI_end

/*
 * Gdk Event Transformation
 */

static OZ_Term createExposeEvent(char *type, GdkEventExpose *event) {
  GdkRectangle *rect = (GdkRectangle *) malloc(sizeof(GdkRectangle));
  
  memcpy(rect, &(event->area), sizeof(GdkRectangle));

  return OZ_mkTuple(OZ_atom(type), 4,
		    OZ_makeForeignPointer(event->window),
		    OZ_int(event->send_event),
		    OZ_makeForeignPointer(rect),
		    OZ_int(event->count));
}

static OZ_Term computeSource(GdkInputSource source) {
  switch (source) {
  case GDK_SOURCE_MOUSE:
    return OZ_atom("GDK_SOURCE_MOUSE");
  case GDK_SOURCE_PEN:
    return OZ_atom("GDK_SOURCE_PEN");
  case GDK_SOURCE_ERASER:
    return OZ_atom("GDK_SOURCE_ERASER");
  case GDK_SOURCE_CURSOR:
    return OZ_atom("GDK_SOURCE_CURSOR");
  default:
    return OZ_atom("GDK_SOURCE_ALICEDUMMY");
  }
}

static OZ_Term computeCrossing(GdkCrossingMode mode) {
  switch (mode) {
  case GDK_CROSSING_NORMAL:
    return OZ_atom("GDK_CROSSING_NORMAL");
  case GDK_CROSSING_GRAB:
    return OZ_atom("GDK_CROSSING_GRAB");
  case GDK_CROSSING_UNGRAB:
    return OZ_atom("GDK_CROSSING_UNGRAB");
  default:
    return OZ_atom("GDK_CROSSING_ALICEDUMMY");
  }
}

static OZ_Term computeNotify(GdkNotifyType type) {
  switch (type) {
  case GDK_NOTIFY_ANCESTOR:
    return OZ_atom("GDK_NOTIFY_ANCESTOR");
  case GDK_NOTIFY_VIRTUAL:
    return OZ_atom("GDK_NOTIFY_VIRTUAL");
  case GDK_NOTIFY_INFERIOR:
    return OZ_atom("GDK_NOTIFY_INFERIOR");
  case GDK_NOTIFY_NONLINEAR:
    return OZ_atom("GDK_NOTIFY_NONLINEAR");
  case GDK_NOTIFY_NONLINEAR_VIRTUAL:
    return OZ_atom("GDK_NOTIFY_NONLINEAR_VIRTUAL");
  case GDK_NOTIFY_UNKNOWN:
    return OZ_atom("GDK_NOTIFY_UNKNOWN");
  default:
    return OZ_atom("GDK_NOTIFY_ALICEDUMMY");
  }
}

static OZ_Term computeVisibility(GdkVisibilityState state) {
  switch (state) {
  case GDK_VISIBILITY_UNOBSCURED:
    return OZ_atom("GDK_VISIBILITY_UNOBSCURED");
  case GDK_VISIBILITY_PARTIAL:
    return OZ_atom("GDK_VISIBILITY_PARTIAL");
  case GDK_VISIBILITY_FULLY_OBSCURED:
    return OZ_atom("GDK_VISIBILITY_FULLY_OBSCURED");
  default:
    return OZ_atom("GDK_VISIBILITY_ALICEDUMMY");
  }
}

static OZ_Term createMotionEvent(char *type, GdkEventMotion *event) {
  return OZ_mkTuple(OZ_atom(type), 14,
		    OZ_makeForeignPointer(event->window),
		    OZ_int(event->send_event),
		    OZ_int(event->time),
		    OZ_float(event->x),
		    OZ_float(event->y),
		    OZ_float(event->pressure),
		    OZ_float(event->xtilt),
		    OZ_float(event->ytilt),
		    OZ_int(event->state),
		    OZ_int(event->is_hint),
		    computeSource(event->source),
		    OZ_int(event->deviceid),
		    OZ_float(event->x_root),
		    OZ_float(event->y_root));
}

static OZ_Term createKeyEvent(char *type, GdkEventKey *event) {
  return OZ_mkTuple(OZ_atom(type), 7,
		    OZ_makeForeignPointer(event->window),
		    OZ_int(event->send_event),
		    OZ_int(event->time),
		    OZ_int(event->state),
		    OZ_int(event->keyval),
		    OZ_int(event->length),
		    OZ_mkByteString(event->string, event->length));
}

static OZ_Term createCrossingEvent(char *type, GdkEventCrossing *event) {
  return OZ_mkTuple(OZ_atom(type), 12,
		    OZ_makeForeignPointer(event->window),
		    OZ_int(event->send_event),
		    OZ_makeForeignPointer(event->subwindow),
		    OZ_int(event->time),
		    OZ_float(event->x),
		    OZ_float(event->y),
		    OZ_float(event->x_root),
		    OZ_float(event->y_root),
		    computeCrossing(event->mode),
		    computeNotify(event->detail),
		    OZ_int((int) event->focus),
		    OZ_int((int) event->state));
}

static OZ_Term createFocusEvent(char *type, GdkEventFocus *event) {
  return OZ_mkTuple(OZ_atom(type), 3,
		    OZ_makeForeignPointer(event->window),
		    OZ_int(event->send_event),
		    OZ_int((int) event->in));
}

static OZ_Term createConfigureEvent(char *type, GdkEventConfigure *event) {
  return OZ_mkTuple(OZ_atom(type), 6,
		    OZ_makeForeignPointer(event->window),
		    OZ_int(event->send_event),
		    OZ_int(event->x),
		    OZ_int(event->y),
		    OZ_int(event->width),
		    OZ_int(event->height));
}

static OZ_Term createButtonEvent(char *type, GdkEventButton *event) {
  return OZ_mkTuple(OZ_atom(type), 14,
		    OZ_makeForeignPointer(event->window),
		    OZ_int(event->send_event),
		    OZ_int(event->time),
		    OZ_float(event->x),
		    OZ_float(event->y),
		    OZ_float(event->pressure),
		    OZ_float(event->xtilt),
		    OZ_float(event->ytilt),
		    OZ_int(event->state),
		    OZ_int(event->button),
		    computeSource(event->source),
		    OZ_int(event->deviceid),
		    OZ_float(event->x_root),
		    OZ_float(event->y_root));
}

static OZ_Term createVisibilityEvent(char *type, GdkEventVisibility *event) {
  return OZ_mkTuple(OZ_atom(type), 3,
		    OZ_makeForeignPointer(event->window),
		    OZ_int(event->send_event),
		    computeVisibility(event->state));
}

static OZ_Term createNoExposeEvent(char *type, GdkEventNoExpose *event) {
  return OZ_mkTuple(OZ_atom(type), 2,
		    OZ_makeForeignPointer(event->window),
		    OZ_int(event->send_event));
}

static OZ_Term createGdkEvent(GdkEvent *event) {
  switch (event->type) {
  case GDK_NOTHING:
    return OZ_atom("GDK_NOTHING");
  case GDK_DELETE:
    return OZ_atom("GDK_DELETE");
  case GDK_DESTROY:
    return OZ_atom("GDK_DESTROY");
  case GDK_EXPOSE:
    return createExposeEvent("GDK_EPOSE", (GdkEventExpose *) event); 
  case GDK_MOTION_NOTIFY:
    return createMotionEvent("GDK_MOTION_NOTIFY", (GdkEventMotion *) event);
  case GDK_BUTTON_PRESS:
    return createButtonEvent("GDK_BUTTON_PRESS", (GdkEventButton *) event);
  case GDK_2BUTTON_PRESS:
    return createButtonEvent("GDK_2BUTTON_PRESS", (GdkEventButton *) event);
  case GDK_3BUTTON_PRESS:
    return createButtonEvent("GDK_3BUTTON_PRESS", (GdkEventButton *) event);
  case GDK_BUTTON_RELEASE:
    return createButtonEvent("GDK_BUTTON_RELEASE", (GdkEventButton *) event);
  case GDK_KEY_PRESS:
    return createKeyEvent("GDK_KEY_PRESS", (GdkEventKey *) event);
  case GDK_KEY_RELEASE:
    return createKeyEvent("GDK_KEY_RELEASE", (GdkEventKey *) event);
    break;
  case GDK_ENTER_NOTIFY:
    return createCrossingEvent("GDK_ENTER_NOTIFY", (GdkEventCrossing *) event);
  case GDK_LEAVE_NOTIFY:
    return createCrossingEvent("GDK_LEAVE_NOTIFY", (GdkEventCrossing *) event);
  case GDK_FOCUS_CHANGE:
    return createFocusEvent("GDK_FOCUS_CHANGE", (GdkEventFocus *) event);
  case GDK_CONFIGURE:
    return createConfigureEvent("GDK_CONFIGURE", (GdkEventConfigure *) event);
  case GDK_MAP:
    return OZ_atom("GDK_MAP");
  case GDK_UNMAP:
    return OZ_atom("GDK_UNMAP");
  case GDK_PROPERTY_NOTIFY:
    return OZ_atom("GDK_PROPERTY_NOTIFY");
  case GDK_SELECTION_CLEAR:
    return OZ_atom("GDK_SELECTION_CLEAR");
  case GDK_SELECTION_REQUEST:
    return OZ_atom("GDK_SELECTION_REQUEST");
  case GDK_SELECTION_NOTIFY:
    return OZ_atom("GDK_SELECTION_NOTIFY");
  case GDK_PROXIMITY_IN:
    return OZ_atom("GDK_PROXIMITY_IN");
  case GDK_PROXIMITY_OUT:
    return OZ_atom("GDK_PROXIMITY_OUT");
  case GDK_DRAG_ENTER:
    return OZ_atom("GDK_DRAG_ENTER");
  case GDK_DRAG_LEAVE:
    return OZ_atom("GDK_DRAG_LEAVE");
  case GDK_DRAG_MOTION:
    return OZ_atom("GDK_DRAG_MOTION");
  case GDK_DRAG_STATUS:
    return OZ_atom("GDK_DRAG_STATUS");
  case GDK_DROP_START:
    return OZ_atom("GDK_DROP_START");
    break;
  case GDK_DROP_FINISHED:
    return OZ_atom("GDK_DROP_FINISHED");
  case GDK_CLIENT_EVENT:
    return OZ_atom("GDK_CLIENT_EVENT");
  case GDK_VISIBILITY_NOTIFY:
    return createVisibilityEvent("GDK_VISIBILITY_NOTIFY",
				 (GdkEventVisibility *) event);
  case GDK_NO_EXPOSE:
    return createNoExposeEvent("GDK_NO_EXPOSE", (GdkEventNoExpose *) event);
  default:
    return OZ_atom("UNSUPPORTED");
  }
}

/*
 * User Data is transmitted using the GtkArg Array.
 * Usually it consists of zero or one Argument, the GdkEvent Pointer.
 * The event pointer will be transformed to a tuple.
 * Additional Data will be ignored and should not be used.
 */

static void signal_marshal(GtkObject *object, gpointer oz_id,
			   guint n_args, GtkArg *args) {
  switch (n_args) {
  case 0: /* GtkWidget Event */
    OZ_send(signal_port, OZ_mkTuple(OZ_atom("event"), 2, OZ_int((guint) oz_id),
				    OZ_atom("UNSUPPORTED")));
    break;
  case 1: /* GdkEvent Type is stored as object */
    OZ_send(signal_port,
	    OZ_mkTuple(OZ_atom("event"), 2, OZ_int((guint) oz_id),
		       createGdkEvent((GdkEvent*) GTK_VALUE_OBJECT(args[0]))));
    break;
  default:
    fprintf(stderr, "signal_marshal: unable to handle event. IGNORED.\n");
    break;
  }

  /* Assign Result Type; this is fake because it ALWAYS indicates non-handling.
   * This should be changed later on but will work fine (but slowly) for now.
   * CAUTION: Returning FALSE yields the destruction of GTK object hierarchy
   * CAUTION: before the handler was actually executed in case of delete_event.
   */
  GtkArg result      = args[n_args + 1];
  result.type        = GTK_TYPE_BOOL;
  result.d.bool_data = FALSE;
}

/*
 * 1. The callback function is always NULL. We use our marshaller instead.
 * 2. Signals are transmitted as atoms.
 * 3. the signal id of gtk_signal_connect_full is ignored.
 */

OZ_BI_define (native_signal_connect, 3, 0) {
  GOZ_declareObject(0, object);
  OZ_declareTerm(1, name);
  OZ_declareInt(2, oz_id);

  gtk_signal_connect_full(GTK_OBJECT (object),
			  (gchar *) OZ_virtualStringToC(name, NULL),
			  NULL, signal_marshal, (gpointer) oz_id,
			  NULL, FALSE, FALSE);

  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define(native_signal_disconnect, 2, 0) {
  GOZ_declareObject(0, object);
  OZ_declareInt(1, id);

  gtk_signal_disconnect(object, (guint) id);

  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define(native_signal_block, 2, 0) {
  GOZ_declareObject(0, object);
  OZ_declareInt(1, id);

  gtk_signal_handler_block(object, (guint) id);
  
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define(native_signal_unblock, 2, 0) {
  GOZ_declareObject(0, object);
  OZ_declareInt(1, id);

  gtk_signal_handler_unblock(object, (guint) id);
  
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define(native_signal_emit, 2, 0) {
  GOZ_declareObject(0, object);
  GOZ_declareString(1, signal);

  gtk_signal_emit_by_name(object, signal);

  return OZ_ENTAILED;
} OZ_BI_end

/*
 * Lowlevel Allocation Functions
 */

OZ_BI_define (native_alloc_int, 1, 1) {
  OZ_declareInt(0, val);
  int *ret = (int *) malloc(sizeof(int));
  
  *ret = val;
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (native_alloc_double, 1, 1) {
  OZ_declareFloat(0, val);
  double *ret = (double *) malloc(sizeof(double));
  
  *ret = val;
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

static char *use_str = NULL;

OZ_BI_define (native_alloc_str, 1, 1) {
  OZ_declareInt(0, len);

  if (use_str != NULL) {
    free(use_str);
  }
  use_str = (char *) malloc(len);
  OZ_out(0) = OZ_makeForeignPointer(&use_str);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (native_alloc_color, 3, 1) {
  GdkColor *ret = (GdkColor *) malloc(sizeof(GdkColor));
  
  OZ_declareInt(0, red);
  OZ_declareInt(1, blue);
  OZ_declareInt(2, green);
  
  ret->red   = red;
  ret->blue  = blue;
  ret->green = green;

  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

/*
 *  Lowlevel Type Access
 */

OZ_BI_define (native_get_int, 1, 1) {
  OZ_declareForeignType(0, val, int *);
  
  OZ_out(0) = OZ_int(*val);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (native_get_double, 1, 1) {
  OZ_declareForeignType(0, val, double *);
  
  OZ_out(0) = OZ_float(*val);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (native_get_str, 0, 1) {
  OZ_out(0) = OZ_string(use_str);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (native_null, 0, 1) {
  OZ_out(0) = OZ_makeForeignPointer((void *) NULL);
  return OZ_ENTAILED;
} OZ_BI_end

/*
 * Generic Lowlevel Deallocation
 */

OZ_BI_define (native_free_data, 1, 0) {
  OZ_declareForeignType(0, val, void *);

  if (val != NULL) {
    free(val);
  }
  return OZ_ENTAILED;
} OZ_BI_end

/*
 * Lowlevel Gtk Canvas Helper
 */

/* Hack Alert */
typedef struct {
  int num_points;
  double *coords;
  int ref_count;
} GtkCanvasPoints;

OZ_BI_define(native_points_put, 3, 0) {
  OZ_declareForeignType(0, val, GtkCanvasPoints *);
  OZ_declareInt(1, i);
  OZ_declareInt(2, x);
  val->coords[i] = x;
  return OZ_ENTAILED;
} OZ_BI_end

/* 
 * Lowlevel GtkArg Handling
 */ 

OZ_BI_define(native_make_arg, 2, 1) {
  GOZ_declareString(0, name);
  GOZ_declareTerm(1, val);
  GtkArg *ret = (GtkArg *) malloc(sizeof(GtkArg));
  ret->name = name;
  if (OZ_isInt(val)) {
    ret->type = GTK_TYPE_INT;
    ret->d.int_data = OZ_intToC(val);
  }
  else if (OZ_isFloat(val)) {
    ret->type = GTK_TYPE_DOUBLE;
    ret->d.double_data = OZ_floatToC(val);
  }
  else if (OZ_isBool(val)) {
    ret->type = GTK_TYPE_BOOL;
    ret->d.bool_data = OZ_boolToC(val);
  }
  else if (OZ_isVirtualString(val, NULL)) {
    ret->type = GTK_TYPE_STRING;
    ret->d.string_data = GOZ_stringToC(val);
  }
  else if (OZ_isForeignPointer(val)) {
    ret->type = GTK_TYPE_OBJECT;
    ret->d.object_data = (GtkObject *) OZ_getForeignPointer(val);
  }
  else {
    ret->type = GTK_TYPE_INVALID;
  }
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (native_get_arg, 1, 1) {
  GOZ_declareForeignType(GtkArg *, 0, val);

  switch (val->type) {
  case GTK_TYPE_INT:
    OZ_out(0) = OZ_int(val->d.int_data);
    break;
  case GTK_TYPE_DOUBLE:
    OZ_out(0) = OZ_float(val->d.double_data);
    break;
  case GTK_TYPE_BOOL:
    OZ_out(0) = OZ_int(val->d.bool_data);
    break;
  case GTK_TYPE_STRING:
    OZ_out(0) = OZ_string(val->d.string_data);
    break;
  case GTK_TYPE_OBJECT:
    OZ_out(0) = OZ_makeForeignPointer(val->d.object_data);
    break;
  default:
    OZ_out(0) = OZ_atom("unit");
    break;
  }
  return OZ_ENTAILED;
} OZ_BI_end

/*
 * Lowlevel String Array Handling
 */

OZ_BI_define (native_alloc_str_arr, 1, 1) {
  OZ_declareInt(0, len);
  char **arr = (char **) malloc(sizeof(char *) * len);
  for (;len--;) {
    arr[len] = NULL;
  }
  OZ_out(0) = OZ_makeForeignPointer(arr);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (native_make_str_arr, 2, 1) {
  OZ_declareInt(0, len);
  OZ_declareTerm(1, t);
  char **arr = (char **) malloc(sizeof(char *) * (len + 1));
  arr[len] = NULL;
  len = 0;
  while (OZ_isCons(t)) {
    arr[len++] = strdup(OZ_virtualStringToC(OZ_head(t), NULL));
    t = OZ_tail(t);
  }
  OZ_out(0) = OZ_makeForeignPointer(arr);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (native_get_str_arr, 1, 1) {
  GOZ_declareForeignType(char **, 0, arr);
  int i = 0;
  OZ_Term t = OZ_atom("nil");
  if (i > 0) {
    i--;
  }
  while (i >= 0) {
    t = OZ_cons(OZ_string(arr[i--]), t);
  }
  OZ_out(0) = t;
  return OZ_ENTAILED;
} OZ_BI_end

/*
 * Define Interface
 */

static OZ_C_proc_interface oz_interface[] = {
  {"initializeSignalPort", 1, 0, native_initialize_signal_port},
  {"handlePendingEvents", 0, 1, native_handle_pending_events},
  {"signalConnect", 3, 0, native_signal_connect},
  {"signalDisconnect", 2, 0, native_signal_disconnect},
  {"signalBlock", 2, 0, native_signal_block},
  {"signalUnblock", 2, 0, native_signal_unblock},
  {"signalEmit", 2, 0, native_signal_emit},
  {"allocInt", 1, 1, native_alloc_int},
  {"allocDouble", 1, 1, native_alloc_double},
  {"allocColor", 3, 1, native_alloc_color},
  {"allocStr", 1, 1, native_alloc_str},
  {"getInt", 1, 1, native_get_int},
  {"getDouble", 1, 1, native_get_double},
  {"getStr", 0, 1, native_get_str},
  {"null", 0, 1, native_null},
  {"freeData", 1, 0, native_free_data},
  {"pointsPut", 3, 0, native_points_put},
  {"makeArg", 2, 1, native_make_arg},
  {"getArg", 1, 1, native_get_arg},
  {"allocStrArr", 1, 1, native_alloc_str_arr},
  {"getStrArr", 1, 1, native_get_str_arr},
  {"makeStrArr", 2, 1, native_make_str_arr},
  {0, 0, 0, 0}
};

char oz_module_name[] = "GOZSignal";

OZ_C_proc_interface *oz_init_module() {
  return oz_interface;
}
