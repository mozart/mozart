/*
 * Authors:
 *   Andreas Simon (1999-2000)
 *
 * Copyright:
 *   Andreas Simon (1999-2000)
 *
 * Last change:
 *   $Date$
 *   $Revision$
 *
 * This file is part of Mozart, an implementation
 * of Oz 3:
 *   http://www.mozart-oz.org
 *
 * See the file "LICENSE" or
 *   http://www.mozart-oz.org/LICENSE.html
 * for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

#ifndef __GOZ_SUPPORT_H__
#define __GOZ_SUPPORT_H__

#include <gtk/gtk.h>
#include <mozart.h>

/* This is for dummy variable identifiers used for casts */
#define GOZ_(name) name ## _

/*****************************************************************************
 * Prototypes
 *****************************************************************************/

OZ_Term             GOZ_GLIST_TO_OZTERM             (GList * glist);
OZ_Term             GOZ_GSLIST_TO_OZTERM            (GSList * gslist);

/*****************************************************************************
 * Declaration of GLib basic types
 *****************************************************************************/

#define GOZ_DECLARE_GBOOLEAN(i, val)        OZ_declareBool (i, GOZ_(val)); gboolean val = (gboolean) GOZ_(val)

#define GOZ_DECLARE_GPOINTER(i, val)        OZ_declareForeignType (i, val, gpointer)
#define GOZ_DECLARE_GCONSTPOINTER(i, val)   OZ_declareForeignType (i, val, gconstpointer)

#define GOZ_DECLARE_GCHAR(i, val)           OZ_declareInt (i, GOZ_(val)); gchar val = (gchar) GOZ_(val)
#define GOZ_DECLARE_GUCHAR(i, val)          OZ_declareInt (i, GOZ_(val)); guchar val = (gchar) GOZ_(val)

#define GOZ_DECLARE_GINT(i, val)            OZ_declareInt (i, GOZ_(val)); gint val = (gint) GOZ_(val)
#define GOZ_DECLARE_GUINT(i, val)           OZ_declareLong (i, GOZ_(val)); guint val = (guint) GOZ_(val)
#define GOZ_DECLARE_GSHORT(i, val)          OZ_declareInt (i, GOZ_(val)); gshort val = (gshort) GOZ_(val)
#define GOZ_DECLARE_GUSHORT(i, val)         OZ_declareInt (i, GOZ_(val)); gushort val = (gushort) GOZ_(val)
#define GOZ_DECLARE_GLONG(i, val)           OZ_declareLong (i, GOZ_(val)); glong val = (glong) GOZ_(val)
#define GOZ_DECLARE_GULONG(i, val)          OZ_declareLong (i, GOZ_(val)); gulong val = (gulong) GOZ_(val)

#define GOZ_DECLARE_GINT8(i, val)           OZ_declareInt (i, GOZ_(val)); gint8 val = (gint8) GOZ_(val)
#define GOZ_DECLARE_GUINT8(i, val)          OZ_declareInt (i, GOZ_(val)); guint16 val = (guint16) GOZ_(val)
#define GOZ_DECLARE_GINT16(i, val)          OZ_declareInt (i, GOZ_(val)); gint16 val = (gint16) GOZ_(val)
#define GOZ_DECLARE_GUINT16(i, val)         OZ_declareLong (i, GOZ_(val)); guint16 val = (guint16) GOZ_(val)
#define GOZ_DECLARE_GINT32(i, val)          OZ_declareLong (i, GOZ_(val)); gint32 val = (gint32) GOZ_(val)
#define GOZ_DECLARE_GUINT32(i, val)         OZ_declareLong (i, GOZ_(val)); guint32 val = (guint32) GOZ_(val)
#ifdef G_HAVE_GINT64
#define GOZ_DECLARE_GINT64(i, val)          OZ_declareLong (i, GOZ_(val)); gint64 val = (gint64) GOZ_(val)
#define GOZ_DECLARE_GUINT64(i, val)         OZ_declareLong (i, GOZ_(val)); guint64 val = (guint64) GOZ_(val)
#endif

#define GOZ_DECLARE_GFLOAT(i, val)          OZ_declareFloat (i, GOZ_(val)); gfloat val = (gfloat) GOZ_(val)
#define GOZ_DECLARE_GDOUBLE(i, val)         OZ_declareFloat (i, GOZ_(val)); gdouble val = (gdouble) GOZ_(val)

#define GOZ_DECLARE_GSIZE(i, val)           OZ_declareLong (i, GOZ_(val)); gsize val = (gsize) GOZ_(val)
#define GOZ_DECLARE_GSSIZE(i, val)          OZ_declareInt (i, GOZ_(val)); gssize val = (gssize) GOZ_(val)

/*****************************************************************************
 * Declaration of glib types
 *****************************************************************************/

GList * goz_oz_list_to_g_list(OZ_Term ozlist);

#define GOZ_DECLARE_GLIST(i, val)           \
  GList * val;                              \
  OZ_declareTerm (i, GOZ_(val));            \
  val = goz_oz_list_to_g_list (GOZ_(val))

#define GOZ_DECLARE_GSLIST(i, val)            GSList * val = NULL;
#define GOZ_DECLARE_GSLIST2(i, val)           GSList ** val = NULL;
#define GOZ_DECLARE_GHASHTABLE(i, val)        GHashTable * val = NULL;
#define GOZ_DECLARE_GQUARK(i, val)            OZ_declareLong (i, GOZ_(val)); GQuark val = (GQuark) GOZ_(val)
#define GOZ_DECLARE_GSCANNER(i, val)          OZ_declareForeignType (i, val, GScanner *)

#define GOZ_DECLARE_GDESTROYNOTIFY(i, val)    GDestroyNotify val = NULL;

/*****************************************************************************
 * Declaration of gdk types
 *****************************************************************************/

/* TODO: atom is unsigned long */
#define GOZ_DECLARE_GDKATOM(i, val)           OZ_declareLong (i, GOZ_(val)); GdkAtom val = (GdkAtom) GOZ_(val)

#define GOZ_DECLARE_GDKDESTROYNOTIFY(i, val)  GdkDestroyNotify val = NULL;
#define GOZ_DECLARE_GDKEVENTFUNC(i, val)      GdkEventFunc val = NULL;
#define GOZ_DECLARE_GDKFILTERFUNC(i, val)     GdkFilterFunc val = NULL;
#define GOZ_DECLARE_GDKINPUTFUNC(i, val)      GdkInputFunction val = NULL;

/*****************************************************************************
 * Declaration of gtk types
 *****************************************************************************/

#define GOZ_DECLARE_GTKARGINFO2(i, val)          GtkArgInfo ** val = NULL;
#define GOZ_DECLARE_GTKTYPE(i, val)              OZ_declareLong (i, GOZ_(val)); GtkType val = (GtkType) GOZ_(val)
#define GOZ_DECLARE_GTKTYPE1(i, val)             GtkType * val = NULL;

#define GOZ_DECLARE_GTKCALLBACK(i, val)          GtkCallback val = NULL;
#define GOZ_DECLARE_GTKCALLBACKMARSHAL(i, val)   GtkCallbackMarshal val = NULL;
#define GOZ_DECLARE_GTKDESTROYNOTIFY(i, val)     GtkDestroyNotify val = NULL;
#define GOZ_DECLARE_GTKEMISSIONHOOK(i, val)      GtkEmissionHook val = NULL;
#define GOZ_DECLARE_GTKFUNCTION(i, val)          GtkFunction val = NULL;
#define GOZ_DECLARE_GTKIMAGELOADER(i, val)       GtkImageLoader val = NULL;
#define GOZ_DECLARE_GTKSIGNALDESTROY(i, val)     GtkSignalDestroy val = NULL;
#define GOZ_DECLARE_GTKSIGNALMARSHAL(i, val)     GtkSignalMarshal val = NULL;
#define GOZ_DECLARE_GTKSIGNALMARSHALLER(i, val)  GtkSignalMarshaller val = NULL;

#define GOZ_DECLARE_FUNCTION(i, val, func)       func val = NULL; /* TODO */

/*
  Build a gchar* array out of an Oz list with virtual string elements
  TODO: Boy, you really have to check this!!!
*/
#define OZ_declareGCharArrayFromOzListOfVS(ARG, ARRAY)                        \
  gchar   **ARRAY;                                                            \
  {                                                                           \
    gint    length;                                                           \
    int     dummy;                                                            \
    int     i;                                                                \
    OZ_declareTerm (ARG, list);                                               \
                                                                              \
    length = OZ_length (list);                                                \
    g_assert (length >= 0);                                                   \
    ARRAY = g_new (gchar *, _length + 1);                                     \
    for (i=0; i < length; i++) {                                              \
      ARRAY[arg] = g_strdup (OZ_virtualStringToC (OZ_head (list), &_dummy))   \
      list = OZ_tail (list);                                                  \
    }                                                                         \
    ARRAY[_length] = NULL;                                                    \
  }

#define OZ_declareArrayFromOzList(ARG, VAR, TYPE)                             \
  TYPE *VAR;                                                                  \
  {                                                                           \
    int  i;                                                                   \
    int len;                                                                  \
    OZ_declareTerm (ARG, list);                                               \
                                                                              \
    len = OZ_length (list);                                                   \
    g_assert (len >= 0);                                                      \
    for (i=0; i < len; i++) {                                                 \
    }                                                                         \
  }

#define OZ_declareArray(ARG, VAR, TYPE) \
TYPE VAR;

#define OZ_argInfo(ARG_INFO) \
(OZ_nil())

#define OZ_declareForeignArray(ARG, VAR, TYPE) \
TYPE VAR;

/*
  Oz list of string elements to C array (gchar ***)
*/
#define OZ_declareStringArray(ARG, VAR) \
gchar *** VAR; \
{ \
  int len; \
  int i, dummy; \
  char * string; \
\
  VAR = g_new (gchar **, len); \
  OZ_declareTerm (ARG, term); \
  len = OZ_length (term); \
  for (i=0; i<len; i++) { \
    string = OZ_stringToC (OZ_head (term), &dummy); \
    term = OZ_tail (term); \
    VAR[i] = &string; \
  } \
}

#define OZ_declareGdkEvent(ARG, VAR) \
GdkEvent * VAR;

#define OZ_declareGdkFunc(ARG, VAR, TYPE) \
TYPE VAR;

#define OZ_declareGdkEventFunc(ARG, VAR) \
OZ_declareGdkFunc(ARG, VAR, GdkEventFunc)

#define OZ_declareGdkInputFunction(ARG, VAR) \
OZ_declareGdkFunc(ARG, VAR, GdkInputFunction)

#define OZ_declareGdkFilterFunc(ARG, VAR) \
OZ_declareGdkFunc(ARG, VAR, GdkFilterFunc)

#define OZ_declareGdkDestroyNotify(ARG, VAR) \
OZ_declareGdkFunc(ARG, VAR, GdkDestroyNotify)

#define OZ_declareGDestroyNotify(ARG, VAR) \
OZ_declareGdkFunc(ARG, VAR, GDestroyNotify)

#define OZ_declareGtkDestroyNotify(ARG, VAR) \
OZ_declareGdkFunc(ARG, VAR, GtkDestroyNotify)

/*****************************************************************************
 * Conversions from C to Oz
 *****************************************************************************/

/* gdkEvent to Oz record (OZ_term *) */
OZ_Term
OZ_gdkEvent(GdkEvent *event) {
  OZ_Term     record;
  OZ_Term     features[2];
  OZ_Term     arity;

  features[0] = OZ_atom ("type");
  features[1] = OZ_atom ("window");
  features[2] = OZ_atom ("send_event");
  arity = OZ_toList (3, features);

  record = OZ_record (OZ_atom ("gdkEvent"), arity);

  OZ_putSubtree (record, features[0], OZ_int ((int) (*(GdkEventAny *) event).type));
  OZ_putSubtree (record, features[1], OZ_makeForeignPointer ((*(GdkEventAny *) event).window));
  OZ_putSubtree (record, features[2], OZ_int ((*(GdkEventAny *) event).send_event));

  return record;
}

/* Convert a GList to an Oz list of foreign elements */
OZ_Term
GOZ_GLIST_TO_OZTERM (GList * glist) {
  OZ_Term    ozlist;
  gpointer   element;

  ozlist = OZ_nil ();

  element = g_list_last (glist);
  do {
    ozlist = OZ_cons (OZ_makeForeignPointer (element) , ozlist);
  } while (element = g_list_previous (glist));

  g_list_free (glist);

  return ozlist;
}

/* Convert a GSList to an Oz list of foreign elements */
OZ_Term
GOZ_GSLIST_TO_OZTERM (GSList * gslist) {
  OZ_Term ozlist;

  ozlist = OZ_nil ();
  return ozlist;
}

/*****************************************************************************
 * Return values
 *****************************************************************************/

#define GOZ_RETURN_GFLOAT(val)               OZ_RETURN (OZ_float (val))
#define GOZ_RETURN_GCHAR2(val)               OZ_RETURN (OZ_nil ()) /* TODO */
#define GOZ_RETURN_GNODE(val)                OZ_RETURN (OZ_nil ()) /* TODO */

#define GOZ_BOOL(val)                        (val)?OZ_true():OZ_false()


#endif /* __GOZ_SUPPORT_H__ */
