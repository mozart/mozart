#ifndef GTK_CANVAS_LOAD_H
#define GTK_CANVAS_LOAD_H

#include <gtk-canvas/gtk-canvas-defs.h>
#include <gdk_imlib.h>

BEGIN_GTK_CANVAS_DECLS

/* Note that it will only loads png files */
GdkImlibImage *gtk_canvas_load_alpha (const gchar *file);

void gtk_canvas_destroy_image (GdkImlibImage *image);

END_GTK_CANVAS_DECLS

#endif

