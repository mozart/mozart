#include <config.h>
#include <gtk/gtktypeutils.h>
#include "gtk-canvas.h"
#include "gtk-canvastypebuiltins.h"

#include "gtk-canvastypebuiltins_vars.c"
#include "gtk-canvastypebuiltins_evals.c"
void gtk_canvas_type_init(void);

/* maybe this should be put in GTK */
static GtkType
gtk_canvas_type_register_boxed (const gchar *name) {
  GtkTypeInfo info;

  info.type_name = (char *)name;
  info.object_size = 0;
  info.class_size = 0;
  info.class_init_func = NULL;
  info.object_init_func = NULL;
  info.reserved_1 = NULL;
  info.reserved_2 = NULL;

  return gtk_type_unique(GTK_TYPE_BOXED, &info);
}

void
gtk_canvas_type_init() {
  int i;

  static struct {
    const gchar * type_name;
    GtkType *type_id;
    GtkType parent;
    const GtkEnumValue *values;
  } builtin_info[GNOME_TYPE_NUM_BUILTINS + 1] = {
#include "gtk-canvastypebuiltins_ids.c"
    { NULL }
  };

  for (i = 0; i < GNOME_TYPE_NUM_BUILTINS; i++)
    {
      GtkType type_id = GTK_TYPE_INVALID;
      g_assert (builtin_info[i].type_name != NULL);
      if ( builtin_info[i].parent == GTK_TYPE_ENUM )
      	type_id = gtk_type_register_enum (builtin_info[i].type_name, (GtkEnumValue *)builtin_info[i].values);
      else if ( builtin_info[i].parent == GTK_TYPE_FLAGS )
      	type_id = gtk_type_register_flags (builtin_info[i].type_name, (GtkFlagValue *)builtin_info[i].values);
      else if ( builtin_info[i].parent == GTK_TYPE_BOXED )
        type_id = gtk_canvas_type_register_boxed (builtin_info[i].type_name);

      g_assert (type_id != GTK_TYPE_INVALID);
      (*builtin_info[i].type_id) = type_id;
    }

}

