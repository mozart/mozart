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
#include <stdio.h>
#include <string.h> /* for memcpy */
#include "GOZData.h"
#if defined(__CYGWIN32__) || defined(__MINGW32__)
#include <windows.h>
#endif

/*
 * Signal Handling/Marshalling from Host Language <-> G(D|T)K
 */

/* This holds the port variable */
static OZ_Term signal_port = 0;
/* Indicates that the marshaller has been called */
static int had_events      = 0;

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
 * whether there were events or not.
 * had_events is set if the marshaller has been called.
 * Otherwise, Oz would be notified too often.
 */

OZ_BI_define (native_handle_pending_events, 0, 1) {
  while (gtk_events_pending()) {
    gtk_main_iteration();
  }
  OZ_out(0) = (had_events ? OZ_true() : OZ_false());
  had_events = 0;
  return OZ_ENTAILED;
} OZ_BI_end

/*
 * Gdk Event Transformation
 */

static OZ_Term createExposeEvent(char *type, GdkEventExpose *event) {
  GdkRectangle *rect = (GdkRectangle *) g_malloc(sizeof(GdkRectangle));
  
  memcpy(rect, &(event->area), sizeof(GdkRectangle));

  return OZ_mkTuple(OZ_atom(type), 4,
		    OZ_makeForeignPointer(event->window),
		    GOZ_bool(event->send_event),
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
		    GOZ_bool(event->send_event),
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
		    GOZ_bool(event->send_event),
		    OZ_int(event->time),
		    OZ_int(event->state),
		    OZ_int(event->keyval),
		    OZ_int(event->length),
		    OZ_mkByteString(event->string, event->length));
}

static OZ_Term createCrossingEvent(char *type, GdkEventCrossing *event) {
  return OZ_mkTuple(OZ_atom(type), 12,
		    OZ_makeForeignPointer(event->window),
		    GOZ_bool(event->send_event),
		    OZ_makeForeignPointer(event->subwindow),
		    OZ_int(event->time),
		    OZ_float(event->x),
		    OZ_float(event->y),
		    OZ_float(event->x_root),
		    OZ_float(event->y_root),
		    computeCrossing(event->mode),
		    computeNotify(event->detail),
		    GOZ_bool((int) event->focus),
		    OZ_int((int) event->state));
}

static OZ_Term createFocusEvent(char *type, GdkEventFocus *event) {
  return OZ_mkTuple(OZ_atom(type), 3,
		    OZ_makeForeignPointer(event->window),
		    GOZ_bool(event->send_event),
		    GOZ_bool((int) event->in));
}

static OZ_Term createConfigureEvent(char *type, GdkEventConfigure *event) {
  return OZ_mkTuple(OZ_atom(type), 6,
		    OZ_makeForeignPointer(event->window),
		    GOZ_bool(event->send_event),
		    OZ_int(event->x),
		    OZ_int(event->y),
		    OZ_int(event->width),
		    OZ_int(event->height));
}

static OZ_Term createButtonEvent(char *type, GdkEventButton *event) {
  return OZ_mkTuple(OZ_atom(type), 14,
		    OZ_makeForeignPointer(event->window),
		    GOZ_bool(event->send_event),
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
		    GOZ_bool(event->send_event),
		    computeVisibility(event->state));
}

static OZ_Term createNoExposeEvent(char *type, GdkEventNoExpose *event) {
  return OZ_mkTuple(OZ_atom(type), 2,
		    OZ_makeForeignPointer(event->window),
		    GOZ_bool(event->send_event));
}

OZ_Term createGdkEvent(GdkEvent *event) {
  switch (event->type) {
  case GDK_NOTHING:
    return OZ_atom("GDK_NOTHING");
  case GDK_DELETE:
    return OZ_atom("GDK_DELETE");
  case GDK_DESTROY:
    return OZ_atom("GDK_DESTROY");
  case GDK_EXPOSE:
    return createExposeEvent("GDK_EXPOSE", (GdkEventExpose *) event); 
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
 * GtkArg Handling (generated function)
 */

extern OZ_Term makeArgTerm(GtkArg *arg);


/*
 * We need two marshallers; one for "normal" events and one for delete
 * events which behave differently due to the MAGIC value.
 * Furthermore, it is necessary to override the default delete event
 * handler directly on the C side to prevent premature object destruction.
 *
 * User Data is transmitted using the GtkArg Array.
 * The event pointer will be transformed to a tuple.
 */

static void signal_standard_marshal(GtkObject *object, gpointer oz_id,
				    guint n_args, GtkArg *args) {
  OZ_Term event = OZ_nil();

  for (int i = n_args; i--;) {
    event = OZ_cons(makeArgTerm(&(args[i])), event);
  }
  event = OZ_cons(OZ_int((guint) oz_id), event);
#if defined(DEBUG)
  fprintf(stderr, "signal_standard_marshal: sending `%s'\n",
	  OZ_toC(event, 10, 10));
#endif
  OZ_send(signal_port, event);
  
  /* Now tell the oz side that meaningful events ocurred.
   * This is checked during handle_pending_events.
   */
  had_events = 1;
  
  /* Assign Result Type; this is fake because it ALWAYS indicates non-handling.
   * This should be changed later on but will work fine (but slowly) for now.
   * Normal events need FALSE to indicate non-handling.
   */
  GtkArg result      = args[n_args + 1];
  result.type        = GTK_TYPE_BOOL;
  result.d.bool_data = FALSE;
}

static void signal_delete_marshal(GtkObject *object, gpointer oz_id,
				  guint n_args, GtkArg *args) {
  OZ_Term event = OZ_nil();

  for (int i = n_args; i--;) {
    event = OZ_cons(makeArgTerm(&(args[i])), event);
  }
  event = OZ_cons(OZ_int((guint) oz_id), event);
#if defined(DEBUG)
  fprintf(stderr, "signal_delete_marshal: sending `%s'\n",
	  OZ_toC(event, 10, 10));
#endif
  OZ_send(signal_port, event);
  
  /* Now tell the oz side that meaningful events ocurred.
   * This is checked during handle_pending_events.
   */
  had_events = 1;
  
  /* Assign Result Type; this is fake because it ALWAYS indicates non-handling.
   * This should be changed later on but will work fine (but slowly) for now.
   * Delete events need TRUE to indicate non-handling.
   */
  GtkArg result      = args[n_args + 1];
  result.type        = GTK_TYPE_BOOL;
  result.d.bool_data = TRUE;
}

/* The standard oz delete event handler */
static gboolean oz_delete_event(GtkWidget *widget, GdkEventAny *event) {
  return TRUE;
}

/*
 * 1. The callback function is always NULL. We use our marshaller instead.
 * 2. Signals are transmitted as atoms.
 * 3. the signal id of gtk_signal_connect_full is ignored.
 */


OZ_BI_define (native_signal_connect, 4, 1) {
  GOZ_declareObject(0, object);
  OZ_declareTerm(1, name);
  OZ_declareInt(2, oz_id);
  OZ_declareInt(3, normal_event);

  /* Delete Events need special care */
  guint id;
  if (normal_event) {
    id = gtk_signal_connect_full(GTK_OBJECT (object),
				 (gchar *) OZ_virtualStringToC(name, NULL),
				 NULL, signal_standard_marshal,
				 (gpointer) oz_id,
				 NULL, FALSE, FALSE);
  }
  else {
    GtkWidgetClass *cl = (GtkWidgetClass *) (GTK_OBJECT (object)->klass);
    cl->delete_event = oz_delete_event;
    
    id = gtk_signal_connect_full(GTK_OBJECT (object),
				 (gchar *) OZ_virtualStringToC(name, NULL),
				 NULL, signal_delete_marshal, (gpointer) oz_id,
				 NULL, FALSE, FALSE);
  }
  OZ_out(0) = OZ_int(id);
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
  int *ret = (int *) g_malloc(sizeof(int));
  
  *ret = val;
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (native_alloc_double, 1, 1) {
  OZ_declareFloat(0, val);
  double *ret = (double *) g_malloc(sizeof(double));
  
  *ret = val;
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

static char *use_str = NULL;

OZ_BI_define (native_alloc_str, 1, 1) {
  OZ_declareInt(0, len);

  if (use_str != NULL) {
    g_free(use_str);
  }
  use_str = (char *) g_malloc(len);
  OZ_out(0) = OZ_makeForeignPointer(&use_str);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (native_alloc_color, 3, 1) {
  GdkColor *ret = (GdkColor *) g_malloc(sizeof(GdkColor));
  
  OZ_declareInt(0, red);
  OZ_declareInt(1, green);
  OZ_declareInt(2, blue);
  
  ret->red   = red;
  ret->green = green;
  ret->blue  = blue;

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

static OZ_Term gtk_null;

OZ_BI_define (native_null, 0, 1) {
  OZ_out(0) = gtk_null;
  return OZ_ENTAILED;
} OZ_BI_end

/*
 * Generic Lowlevel Deallocation
 */

OZ_BI_define (native_free_data, 1, 0) {
  OZ_declareForeignType(0, val, void *);

  if (val != NULL) {
    g_free(val);
  }
  return OZ_ENTAILED;
} OZ_BI_end

/* 
 * Lowlevel GtkArg Handling
 */ 

OZ_BI_define(native_make_empty_arg, 1, 1) {
  GOZ_declareString(0, name);
  GtkArg *ret = (GtkArg *) g_malloc(sizeof(GtkArg));
  ret->type = GTK_TYPE_DOUBLE;
  ret->name = g_strdup(name);
  OZ_out(0) = OZ_makeForeignPointer(ret);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define(native_make_arg, 2, 1) {
  GOZ_declareString(0, name);
  GOZ_declareTerm(1, val);
  GtkArg *ret = (GtkArg *) g_malloc(sizeof(GtkArg));
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
  OZ_out(0) = makeArgTerm(val);
  return OZ_ENTAILED;
} OZ_BI_end

/*
 * GtkObject Type Functions
 */

OZ_BI_define(native_is_object, 1, 1) {
  GOZ_declareObject(0, obj);
  OZ_out(0) = (GTK_IS_OBJECT(obj) ? OZ_true() : OZ_false());
  return OZ_ENTAILED;
} OZ_BI_end


OZ_BI_define(native_get_object_type, 1, 1) {
  GOZ_declareObject(0, obj);
  OZ_out(0) = OZ_int((int) GTK_OBJECT_TYPE(obj));
  return OZ_ENTAILED;
} OZ_BI_end

/*
 * Lowlevel String (Array) Handling
 */

OZ_BI_define (native_make_native_string, 1, 1) {
  GOZ_declareString(0, arg);
  OZ_out(0) = OZ_makeForeignPointer(arg);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (native_alloc_str_arr, 1, 1) {
  OZ_declareInt(0, len);
  char **arr = (char **) g_malloc(sizeof(char *) * len);
  for (;len--;) {
    arr[len] = NULL;
  }
  OZ_out(0) = OZ_makeForeignPointer(arr);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (native_make_str_arr, 2, 1) {
  OZ_declareInt(0, len);
  OZ_declareTerm(1, t);
  char **arr = (char **) g_malloc(sizeof(char *) * (len + 1));
  arr[len] = NULL;
  len = 0;
  while (OZ_isCons(t)) {
    arr[len++] = g_strdup(OZ_virtualStringToC(OZ_head(t), NULL));
    t = OZ_tail(t);
  }
  OZ_out(0) = OZ_makeForeignPointer(arr);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (native_get_str_arr, 1, 1) {
  GOZ_declareForeignType(char **, 0, arr);
  int i = 0;
  OZ_Term t = OZ_atom("nil");
  while (arr[i++] != NULL);
  if (i > 0) {
    i--;
  }
  while (i >= 0) {
    t = OZ_cons(OZ_string(arr[i--]), t);
  }
  OZ_out(0) = t;
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (native_free_str_arr, 1, 0) {
  GOZ_declareForeignType(char **, 0, arr);
  int i = 0;
  while (arr[i] != NULL) {
    free(arr[i++]);
  }
  free(arr);
  return OZ_ENTAILED;
} OZ_BI_end

/*
 * Lowlevel Color Array Handling
 */

OZ_BI_define (native_make_color_array, 1, 1) {
  OZ_declareTerm(0, colors);
  double *arr = (double *) g_malloc(4 * sizeof(double));
  for (int i = 0; i < 4; i++) {
    arr[i] = OZ_floatToC(OZ_head(colors));
    colors = OZ_tail(colors);
  }
  OZ_out(0) = OZ_makeForeignPointer(arr);
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (native_get_color_list, 1, 1) {
  GOZ_declareForeignType(double *, 0, arr);
  OZ_Term colors = OZ_nil();
  for (int i = 4; i--;) {
    colors = OZ_cons(OZ_float(arr[i]), colors);
  }
  OZ_out(0) = colors;
  return OZ_ENTAILED;
} OZ_BI_end

#if defined(__CYGWIN32__) || defined(__MINGW32__)

/*
 * On Windows, Gdk blocks on stdin during init if  input redirection is used,
 * as for example within emacs.
 * The solution is to reconnect stdin to a pipe and undo this change after
 * gdk/gtk init. gdk still obtains its input.
 */

static HANDLE stdInHandle, pipeInHandle, pipeOutHandle;

OZ_BI_define(native_redirect_stdin, 0, 0) {
  SECURITY_ATTRIBUTES saAttr;
  saAttr.nLength              = sizeof(SECURITY_ATTRIBUTES);
  saAttr.lpSecurityDescriptor = NULL;
  saAttr.bInheritHandle       = TRUE;
  stdInHandle = GetStdHandle(STD_INPUT_HANDLE);
  if (!CreatePipe(&pipeInHandle, &pipeOutHandle, &saAttr, 0)) {
    fprintf(stderr, "error creating pipe\n"); fflush(stderr);
    exit(0);
  }
  if (!SetStdHandle(STD_INPUT_HANDLE, pipeInHandle)) {
    fprintf(stderr, "error redirecting stdin\n"); fflush(stderr);
    exit(0);
  }
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define(native_reset_stdin, 0, 0) {
  if (!SetStdHandle(STD_INPUT_HANDLE, stdInHandle)) {
    fprintf(stderr, "error redirecting stdin\n"); fflush(stderr);
    exit(0);
  }
  return OZ_ENTAILED;
} OZ_BI_end

#endif

/*
 * Define Interface
 */

static OZ_C_proc_interface oz_interface[] = {
  {"initializeSignalPort", 1, 0, native_initialize_signal_port},
  {"handlePendingEvents", 0, 1, native_handle_pending_events},
  {"signalConnect", 4, 1, native_signal_connect},
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
  {"makeEmptyArg", 1, 1, native_make_empty_arg},
  {"makeArg", 2, 1, native_make_arg},
  {"getArg", 1, 1, native_get_arg},
  {"isObject", 1, 1, native_is_object},
  {"getObjectType", 1, 1, native_get_object_type},
  {"makeNativeString", 1, 1, native_make_native_string},
  {"allocStrArr", 1, 1, native_alloc_str_arr},
  {"getStrArr", 1, 1, native_get_str_arr},
  {"freeStrArr", 1, 0, native_free_str_arr},
  {"makeStrArr", 2, 1, native_make_str_arr},
  {"makeColorArr", 1, 1, native_make_color_array},
  {"getColorList", 1, 1, native_get_color_list},
#if defined(__CYGWIN32__) || defined(__MINGW32__)
  {"redirectStdIn", 0, 0, native_redirect_stdin},
  {"resetStdIn", 0, 0, native_reset_stdin},
#endif
  {0, 0, 0, 0}
};

char oz_module_name[] = "GOZSignal";

OZ_C_proc_interface *oz_init_module() {
  gtk_null = OZ_makeForeignPointer((void *) NULL);
  OZ_protect(&gtk_null);
  return oz_interface;
}
