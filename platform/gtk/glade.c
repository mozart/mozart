/*
 * Authors:
 *   Andreas Simon (2000)
 *
 * Copyright:
 *   Andreas Simon (2000)
 *
 * Last change:
 *   $Date$ by $Author$
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

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <mozart.h>

extern void signal_marshal
(GtkObject *object, gpointer oz_id, guint n_args, GtkArg *args);

/*****************************************************************************
 * Signals
 *****************************************************************************/

connect_function (const gchar *handler_name,
                  GtkObject   *object,
                  const gchar *signal_name,
                  const gchar *signal_data,
                  GtkObject   *connect_object,
                  gboolean    after,
                  gpointer    user_data) /* This pointer contains an guint (the signal id) */
{
  signal_marshal (object, user_data, 0, NULL);
}

/*****************************************************************************
 * Wrappers
 *****************************************************************************/

OZ_BI_define (oz_glade_init, 0, 0)
{
  glade_init ();
  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (oz_glade_xml_new, 1, 2)
{
  GladeXML *obj;

  OZ_declareString (0, fname);
  OZ_declareString (1, root);

  obj = glade_xml_new ((const char *) fname,
                       (const char *) root);

  OZ_RETURN (OZ_makeForeignPointer (obj));
} OZ_BI_end

OZ_BI_define (oz_glade_xml_new_with_domain, 1, 3)
{
  GladeXML *obj;

  OZ_declareString (0, fname);
  OZ_declareString (1, root);
  OZ_declareString (2, domain);

  obj = glade_xml_new_with_domain ((const char *) fname,
                                   (const char *) root,
                                   (const char *) domain);

  OZ_RETURN (OZ_makeForeignPointer (obj));
} OZ_BI_end

OZ_BI_define (oz_glade_xml_signal_connect_full, 0, 3)
{
  OZ_declareForeignType (0, self, GladeXML* );
  OZ_declareAtom        (1, handler_name);
  OZ_declareInt         (2, id);

  glade_xml_signal_connect_full (self,
                                 handler_name,
                                 (GladeXMLConnectFunc) connect_function,
                                 (gpointer) id);

  return OZ_ENTAILED;
} OZ_BI_end

OZ_BI_define (oz_glade_xml_get_widget, 1, 2)
{
  GtkWidget *widget;

  OZ_declareForeignType (0, self, GladeXML*);
  OZ_declareAtom        (1, name);

  widget = glade_xml_get_widget (self, name);

  OZ_RETURN (OZ_makeForeignPointer (widget));
}Z_BI_end

/*****************************************************************************
 * Oz interface definition
 *****************************************************************************/

OZ_C_proc_interface *
oz_init_module()
{
  static OZ_C_proc_interface interface[] = {
    {"init",                       0, 0, oz_glade_init},
    {"xmlNew",                     1, 2, oz_glade_xml_new},
    {"xmlNewWithDomain",           1, 3, oz_glade_xml_new_with_domain},
    {"xmlSignalConnectFull",       0, 4, oz_glade_xml_signal_connect_full},
    {"xmlGetWidget",               1, 2, oz_glade_xml_get_widget},
    {0, 0, 0, 0}
  };

  glade_init ();

  return interface;
}
