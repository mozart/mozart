/*
 * Author:
 *   Thorsten Brunklaus <bruni@ps.uni-sb.de>
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


#ifndef __GOZ_DATA_H__
#define __GOZ_DATA_H__

#include <string.h>

#define GOZ_(name) name ## _

/*
 * Basic Types
 */

#define GOZ_declareBool(i, val) \
  OZ_declareInt(i, GOZ_(val)); gboolean val = (gboolean) GOZ_(val)

#define GOZ_declareChar(i, val) \
  OZ_declareInt(i, GOZ_(val)); gchar val = (gchar) GOZ_(val)

#define GOZ_declareUnsignedChar(i, val) \
  OZ_declareInt(i, GOZ_(val)); guchar val = (gchar) GOZ_(val)

#define GOZ_declareInt(i, val) \
  OZ_declareInt(i, GOZ_(val)); gint val = (gint) GOZ_(val)

#define GOZ_declareUnsignedInt(i, val) \
  OZ_declareLong(i, GOZ_(val)); guint val = (guint) GOZ_(val)

#define GOZ_declareShort(i, val) \
  OZ_declareInt(i, GOZ_(val)); gshort val = (gshort) GOZ_(val)

#define GOZ_declareUnsignedShort(i, val) \
  OZ_declareInt(i, GOZ_(val)); gushort val = (gushort) GOZ_(val)

#define GOZ_declareLong(i, val) \
  OZ_declareLong(i, GOZ_(val)); glong val = (glong) GOZ_(val)

#define GOZ_declareUnsignedLong(i, val) \
 OZ_declareLong(i, GOZ_(val)); gulong val = (gulong) GOZ_(val)

#define GOZ_declareInt8(i, val) \
  OZ_declareInt(i, GOZ_(val)); gint8 val = (gint8) GOZ_(val)

#define GOZ_declareUnsignedInt8(i, val) \
  OZ_declareInt(i, GOZ_(val)); guint16 val = (guint16) GOZ_(val)

#define GOZ_declareInt16(i, val) \
  OZ_declareInt(i, GOZ_(val)); gint16 val = (gint16) GOZ_(val)

#define GOZ_declareUnsignedInt16(i, val) \
  OZ_declareLong(i, GOZ_(val)); guint16 val = (guint16) GOZ_(val)

#define GOZ_declareInt32(i, val) \
  OZ_declareLong(i, GOZ_(val)); gint32 val = (gint32) GOZ_(val)

#define GOZ_declareUnsignedInt32(i, val) \
  OZ_declareLong(i, GOZ_(val)); guint32 val = (guint32) GOZ_(val)

#ifdef G_HAVE_GINT64
#define GOZ_declareInt64(i, val) \
  OZ_declareLong(i, GOZ_(val)); gint64 val = (gint64) GOZ_(val)

#define GOZ_declareUnsignedInt64(i, val) \
  OZ_declareLong(i, GOZ_(val)); guint64 val = (guint64) GOZ_(val)
#endif

#define GOZ_declareFloat(i, val) \
 OZ_declareFloat(i, GOZ_(val)); gfloat val = (gfloat) GOZ_(val)

#define GOZ_declareDouble(i, val) \
  OZ_declareFloat(i, GOZ_(val)); gdouble val = (gdouble) GOZ_(val)

#define GOZ_declareSize(i, val) \
  OZ_declareLong(i, GOZ_(val)); gsize val = (gsize) GOZ_(val)

#define GOZ_declareSignedSize(i, val) \
  OZ_declareInt(i, GOZ_(val)); gssize val = (gssize) GOZ_(val)

/* Generic Enum Type */
#define GOZ_declareEnumType(type, i, val) \
  OZ_declareLong(i, GOZ_(val)); type val = (type) GOZ_(val)

#define GOZ_long(val) \
  OZ_long((long) val)

/* Generic Fallback Type */
#define GOZ_declareForeignType(type, i, val) \
  OZ_declareForeignType(i, val, type)

/* Generic Gtk Object Type */
#define GOZ_declareObject(i, val) \
  OZ_declareForeignType(i, val, GtkObject *)

/* String Handling */
#if defined(__CYGWIN32__) || defined(__MINGW32__)
static inline gchar *goz_import_string(gchar *source) {
  GError *res;
  return g_locale_to_utf8(strdup(source), &res);
}

#define GOZ_importString(str) \
  goz_import_string(str);
#else
#define GOZ_importString(str) \
  strdup(str);
#endif

#define GOZ_declareString(i, val) \
  gchar *val; \
  OZ_declareTerm(i, GOZ_(val)) \
  OZ_isUnit (GOZ_(val)) ? \
  val = NULL : val = GOZ_importString((gchar *) OZ_virtualStringToC(GOZ_(val), NULL));

static inline gchar *GOZ_stringToC(OZ_Term val) {
  return GOZ_importString((gchar *) OZ_virtualStringToC(val, NULL));
}

/* GList Handling */
static inline OZ_Term goz_import_glist(GList *ptr) {
  GList *anchor = ptr;
  OZ_Term cons  = OZ_atom("nil");

  ptr = g_list_reverse(ptr);
  while (ptr != NULL) {
    cons = OZ_cons(OZ_makeForeignPointer(ptr->data), cons);
    ptr  = g_list_next(ptr);
  }
  /* To be checked */
  g_list_free(anchor);

  return cons;
}

static inline GList *goz_export_glist(OZ_Term cons) {
  GList *list = NULL;

  while (OZ_isCons(cons)) {
    OZ_Term hd = OZ_head(cons);

    if (OZ_isForeignPointer(hd)) {
      list = g_list_append(list, OZ_getForeignPointer(hd));
    }

    cons = OZ_tail(cons);
  }

  return list;
}

#define GOZ_declareGList(i, val) \
  GList *val; \
  OZ_declareTerm(i, GOZ_(val)) \
  val = goz_export_glist(GOZ_(val));

#define GOZ_makeGList(val) \
  goz_import_glist(val)

/* Generic Argument Handling */
#define GOZ_declareTerm(i, val) \
  OZ_declareTerm(i, val); \
  OZ_Term t_val = OZ_deref(val); \
  if (OZ_isVariable(t_val)) {\
    OZ_suspendOn(t_val); \
  }

#endif
