/*
 * GtkCanvas widget - Tk-like canvas widget for Gnome
 *
 * GtkCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 * Copyright (C) 1998 The Free Software Foundation
 *
 * Authors: Federico Mena <federico@nuclecu.unam.mx>
 *          Raph Levien <raph@gimp.org>
 */

/*
 * TO-DO list for the canvas:
 *
 * - Allow to specify whether GtkCanvasImage sizes are in units or pixels (scale or don't scale).
 *
 * - Implement a flag for gtk_canvas_item_reparent() that tells the function to keep the item
 *   visually in the same place, that is, to keep it in the same place with respect to the canvas
 *   origin.
 *
 * - GC put functions for items.
 *
 * - Widget item (finish it).
 *
 * - GList *gtk_canvas_gimme_all_items_contained_in_this_area (GtkCanvas *canvas, Rectangle area);
 *
 * - Retrofit all the primitive items with microtile support.
 *
 * - Curve support for line item.
 *
 * - Arc item (Havoc has it; to be integrated in GtkCanvasEllipse).
 *
 * - Sane font handling API.
 *
 * - Get_arg methods for items:
 *   - How to fetch the outline width and know whether it is in pixels or units?
 */

/*
 * Raph's TODO list for the antialiased canvas integration:
 *
 * - ::point() method for text item not accurate when affine transformed.
 *
 * - Clip rectangle not implemented in aa renderer for text item.
 *
 * - Clip paths only partially implemented.
 *
 * - Add more image loading techniques to work around imlib deficiencies.
 */

#include <stdio.h>
#include <config.h>
#include <math.h>
#include <gdk/gdkprivate.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include "gtk-canvas.h"
#include "libart_lgpl/art_rect.h"
#include "libart_lgpl/art_rect_uta.h"
#include "libart_lgpl/art_uta_rect.h"
#include "libart_lgpl/art_uta_ops.h"


static void group_add    (GtkCanvasGroup *group, GtkCanvasItem *item);
static void group_remove (GtkCanvasGroup *group, GtkCanvasItem *item);



/*** GtkCanvasItem ***/


enum {
	ITEM_EVENT,
	ITEM_LAST_SIGNAL
};


static void gtk_canvas_request_update (GtkCanvas *canvas);


typedef gint (* GtkCanvasItemSignal1) (GtkObject *item, gpointer arg1, gpointer data);

static void gtk_canvas_item_marshal_signal_1 (GtkObject     *object,
						GtkSignalFunc  func,
						gpointer       func_data,
						GtkArg        *args);

static void gtk_canvas_item_class_init (GtkCanvasItemClass *class);
static void gtk_canvas_item_init       (GtkCanvasItem      *item);
static void gtk_canvas_item_shutdown   (GtkObject            *object);

static void gtk_canvas_item_realize   (GtkCanvasItem *item);
static void gtk_canvas_item_unrealize (GtkCanvasItem *item);
static void gtk_canvas_item_map       (GtkCanvasItem *item);
static void gtk_canvas_item_unmap     (GtkCanvasItem *item);
static void gtk_canvas_item_update    (GtkCanvasItem *item, double *affine,
					 ArtSVP *clip_path, int flags);

static int emit_event (GtkCanvas *canvas, GdkEvent *event);

static guint item_signals[ITEM_LAST_SIGNAL] = { 0 };

static GtkObjectClass *item_parent_class;


/**
 * gtk_canvas_item_get_type:
 *
 * Registers the &GtkCanvasItem class if necessary, and returns the type ID
 * associated to it.
 *
 * Return value:  The type ID of the &GtkCanvasItem class.
 **/
GtkType
gtk_canvas_item_get_type (void)
{
	static GtkType canvas_item_type = 0;

	if (!canvas_item_type) {
		static const GtkTypeInfo canvas_item_info = {
			"GtkCanvasItem",
			sizeof (GtkCanvasItem),
			sizeof (GtkCanvasItemClass),
			(GtkClassInitFunc) gtk_canvas_item_class_init,
			(GtkObjectInitFunc) gtk_canvas_item_init,
			NULL, /* reserved_1 */
			NULL, /* reserved_1 */
			(GtkClassInitFunc) NULL
		};

		canvas_item_type = gtk_type_unique (gtk_object_get_type (), &canvas_item_info);
	}

	return canvas_item_type;
}

/* Class initialization function for GtkCanvasItemClass */
static void
gtk_canvas_item_class_init (GtkCanvasItemClass *class)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) class;

	item_parent_class = gtk_type_class (gtk_object_get_type ());

	item_signals[ITEM_EVENT] =
		gtk_signal_new ("event",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkCanvasItemClass, event),
				gtk_canvas_item_marshal_signal_1,
				GTK_TYPE_BOOL, 1,
				GTK_TYPE_GDK_EVENT);

	gtk_object_class_add_signals (object_class, item_signals, ITEM_LAST_SIGNAL);

	object_class->shutdown = gtk_canvas_item_shutdown;

	class->realize = gtk_canvas_item_realize;
	class->unrealize = gtk_canvas_item_unrealize;
	class->map = gtk_canvas_item_map;
	class->unmap = gtk_canvas_item_unmap;
	class->update = gtk_canvas_item_update;
}

/* Object initialization function for GtkCanvasItem */
static void
gtk_canvas_item_init (GtkCanvasItem *item)
{
	item->object.flags |= GTK_CANVAS_ITEM_VISIBLE;
}

/**
 * gtk_canvas_item_new:
 * @parent: The parent group for the new item.
 * @type: The object type of the item.
 * @first_arg_name: A list of object argument name/value pairs, NULL-terminated,
 * used to configure the item.  For example, "fill_color", "black",
 * "width_units", 5.0, NULL.
 * @Varargs:
 *
 * Creates a new canvas item with @parent as its parent group.  The item is
 * created at the top of its parent's stack, and starts up as visible.  The item
 * is of the specified @type, for example, it can be
 * gtk_canvas_rect_get_type().  The list of object arguments/value pairs is
 * used to configure the item.
 *
 * Return value: The newly-created item.
 **/
GtkCanvasItem *
gtk_canvas_item_new (GtkCanvasGroup *parent, GtkType type, const gchar *first_arg_name, ...)
{
	GtkCanvasItem *item;
	va_list args;

	g_return_val_if_fail (parent != NULL, NULL);
	g_return_val_if_fail (GTK_CANVAS_IS_CANVAS_GROUP (parent), NULL);
	g_return_val_if_fail (gtk_type_is_a (type, gtk_canvas_item_get_type ()), NULL);

	item = GTK_CANVAS_ITEM (gtk_type_new (type));

	va_start (args, first_arg_name);
	gtk_canvas_item_construct (item, parent, first_arg_name, args);
	va_end (args);

	return item;
}

/**
 * gtk_canvas_item_newv:
 * @parent: The parent group for the new item.
 * @type: The object type of the item.
 * @nargs: The number of arguments used to configure the item.
 * @args: The list of arguments used to configure the item.
 *
 * Creates a new canvas item with @parent as its parent group.  The item is
 * created at the top of its parent's stack, and starts up as visible.  The item
 * is of the specified @type, for example, it can be
 * gtk_canvas_rect_get_type().  The list of object arguments is used to
 * configure the item.
 *
 * Return value: The newly-created item.
 **/
GtkCanvasItem *
gtk_canvas_item_newv (GtkCanvasGroup *parent, GtkType type, guint nargs, GtkArg *args)
{
	GtkCanvasItem *item;

	g_return_val_if_fail (parent != NULL, NULL);
	g_return_val_if_fail (GTK_CANVAS_IS_CANVAS_GROUP (parent), NULL);
	g_return_val_if_fail (gtk_type_is_a (type, gtk_canvas_item_get_type ()), NULL);

	item = GTK_CANVAS_ITEM(gtk_type_new (type));

	gtk_canvas_item_constructv (item, parent, nargs, args);

	return item;
}

/* Performs post-creation operations on a canvas item (adding it to its parent
 * group, etc.)
 */
static void
item_post_create_setup (GtkCanvasItem *item)
{
	GtkObject *obj;

	obj = GTK_OBJECT (item);

	group_add (GTK_CANVAS_GROUP (item->parent), item);

	gtk_canvas_request_redraw (item->canvas, item->x1, item->y1, item->x2 + 1, item->y2 + 1);
	item->canvas->need_repick = TRUE;
}

/**
 * gtk_canvas_item_construct:
 * @item: An unconstructed canvas item.
 * @parent: The parent group for the item.
 * @first_arg_name: The name of the first argument for configuring the item.
 * @args: The list of arguments used to configure the item.
 *
 * Constructs a canvas item; meant for use only by item implementations.
 **/
void
gtk_canvas_item_construct (GtkCanvasItem *item, GtkCanvasGroup *parent,
			     const gchar *first_arg_name, va_list args)
{
        GtkObject *obj;
	GSList *arg_list;
	GSList *info_list;
	char *error;

	g_return_if_fail (parent != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_GROUP (parent));

	obj = GTK_OBJECT(item);

	item->parent = GTK_CANVAS_ITEM (parent);
	item->canvas = item->parent->canvas;

	arg_list = NULL;
	info_list = NULL;

	error = gtk_object_args_collect (GTK_OBJECT_TYPE (obj), &arg_list, &info_list,
					 first_arg_name, args);

	if (error) {
		g_warning ("gtk_canvas_item_construct(): %s", error);
		g_free (error);
	} else {
		GSList *arg, *info;

		for (arg = arg_list, info = info_list; arg; arg = arg->next, info = info->next)
			gtk_object_arg_set (obj, arg->data, info->data);

		gtk_args_collect_cleanup (arg_list, info_list);
	}

	item_post_create_setup (item);
}

/**
 * gtk_canvas_item_constructv:
 * @item: An unconstructed canvas item.
 * @parent: The parent group for the item.
 * @nargs: The number of arguments used to configure the item.
 * @args: The list of arguments used to configure the item.
 *
 * Constructs a canvas item; meant for use only by item implementations.
 **/
void
gtk_canvas_item_constructv(GtkCanvasItem *item, GtkCanvasGroup *parent,
			     guint nargs, GtkArg *args)
{
	GtkObject *obj;

	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));
	g_return_if_fail (parent != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_GROUP (parent));

	obj = GTK_OBJECT (item);

	item->parent = GTK_CANVAS_ITEM (parent);
	item->canvas = item->parent->canvas;

	gtk_object_setv (obj, nargs, args);

	item_post_create_setup (item);
}

/* If the item is visible, requests a redraw of it. */
static void
redraw_if_visible (GtkCanvasItem *item)
{
	if (item->object.flags & GTK_CANVAS_ITEM_VISIBLE)
		gtk_canvas_request_redraw (item->canvas, item->x1, item->y1, item->x2 + 1, item->y2 + 1);
}

/* Standard object shutdown function for canvas items */
static void
gtk_canvas_item_shutdown (GtkObject *object)
{
	GtkCanvasItem *item;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (object));

	item = GTK_CANVAS_ITEM (object);

	redraw_if_visible (item);

	/* Make the canvas forget about us */

	if (item == item->canvas->current_item) {
		item->canvas->current_item = NULL;
		item->canvas->need_repick = TRUE;
	}

	if (item == item->canvas->new_current_item) {
		item->canvas->new_current_item = NULL;
		item->canvas->need_repick = TRUE;
	}

	if (item == item->canvas->grabbed_item) {
		item->canvas->grabbed_item = NULL;
		gdk_pointer_ungrab (GDK_CURRENT_TIME);
	}

	if (item == item->canvas->focused_item)
		item->canvas->focused_item = NULL;

	/* Normal destroy stuff */

	if (item->object.flags & GTK_CANVAS_ITEM_MAPPED)
		(* GTK_CANVAS_ITEM_CLASS (item->object.klass)->unmap) (item);

	if (item->object.flags & GTK_CANVAS_ITEM_REALIZED)
		(* GTK_CANVAS_ITEM_CLASS (item->object.klass)->unrealize) (item);

	if (item->parent)
		group_remove (GTK_CANVAS_GROUP (item->parent), item);

	if (item->xform)
		g_free (item->xform);

	if (GTK_OBJECT_CLASS (item_parent_class)->shutdown)
		(* GTK_OBJECT_CLASS (item_parent_class)->shutdown) (object);
}

/* Realize handler for canvas items */
static void
gtk_canvas_item_realize (GtkCanvasItem *item)
{
	GTK_OBJECT_SET_FLAGS (item, GTK_CANVAS_ITEM_REALIZED);

	gtk_canvas_item_request_update (item);
}

/* Unrealize handler for canvas items */
static void
gtk_canvas_item_unrealize (GtkCanvasItem *item)
{
	GTK_OBJECT_UNSET_FLAGS (item, GTK_CANVAS_ITEM_REALIZED);
}

/* Map handler for canvas items */
static void
gtk_canvas_item_map (GtkCanvasItem *item)
{
	GTK_OBJECT_SET_FLAGS (item, GTK_CANVAS_ITEM_MAPPED);
}

/* Unmap handler for canvas items */
static void
gtk_canvas_item_unmap (GtkCanvasItem *item)
{
	GTK_OBJECT_UNSET_FLAGS (item, GTK_CANVAS_ITEM_MAPPED);
}

/* Update handler for canvas items */
static void
gtk_canvas_item_update (GtkCanvasItem *item, double *affine, ArtSVP *clip_path, int flags)
{
	GTK_OBJECT_UNSET_FLAGS (item, GTK_CANVAS_ITEM_NEED_UPDATE);
	GTK_OBJECT_UNSET_FLAGS (item, GTK_CANVAS_ITEM_NEED_AFFINE);
	GTK_OBJECT_UNSET_FLAGS (item, GTK_CANVAS_ITEM_NEED_CLIP);
	GTK_OBJECT_UNSET_FLAGS (item, GTK_CANVAS_ITEM_NEED_VIS);
}

#define HACKISH_AFFINE

/* This routine invokes the update method of the item. */
static void
gtk_canvas_item_invoke_update (GtkCanvasItem *item, double *affine, ArtSVP *clip_path, int flags)
{
	int child_flags;
	double *child_affine;
	double new_affine[6];

#ifdef HACKISH_AFFINE
	double i2w[6], w2c[6], i2c[6];
#endif

	child_flags = flags;
	if (!(item->object.flags & GTK_CANVAS_ITEM_VISIBLE))
		child_flags &= ~GTK_CANVAS_UPDATE_IS_VISIBLE;

	/* Apply the child item's transform */
	if (item->xform == NULL)
		child_affine = affine;
	else if (item->object.flags & GTK_CANVAS_ITEM_AFFINE_FULL) {
		art_affine_multiply (new_affine, item->xform, affine);
		child_affine = new_affine;
	} else {
		int j;

		for (j = 0; j < 4; j++)
			new_affine[j] = affine[j];
		new_affine[4] = item->xform[0] * affine[0] + item->xform[1] * affine[2] + affine[4];
		new_affine[5] = item->xform[0] * affine[1] + item->xform[1] * affine[3] + affine[5];
		child_affine = new_affine;
	}

#ifdef HACKISH_AFFINE
	gtk_canvas_item_i2w_affine (item, i2w);
	gtk_canvas_w2c_affine (item->canvas, w2c);
	art_affine_multiply (i2c, i2w, w2c);
	/* invariant (doesn't hold now): child_affine == i2c */
	child_affine = i2c;
#endif

	/* apply object flags to child flags */

	child_flags &= ~GTK_CANVAS_UPDATE_REQUESTED;

	if (item->object.flags & GTK_CANVAS_ITEM_NEED_UPDATE)
		child_flags |= GTK_CANVAS_UPDATE_REQUESTED;

	if (item->object.flags & GTK_CANVAS_ITEM_NEED_AFFINE)
		child_flags |= GTK_CANVAS_UPDATE_AFFINE;

	if (item->object.flags & GTK_CANVAS_ITEM_NEED_CLIP)
		child_flags |= GTK_CANVAS_UPDATE_CLIP;

	if (item->object.flags & GTK_CANVAS_ITEM_NEED_VIS)
		child_flags |= GTK_CANVAS_UPDATE_VISIBILITY;

	if ((child_flags & (GTK_CANVAS_UPDATE_REQUESTED
			    | GTK_CANVAS_UPDATE_AFFINE
			    | GTK_CANVAS_UPDATE_CLIP
			    | GTK_CANVAS_UPDATE_VISIBILITY))
	    && GTK_CANVAS_ITEM_CLASS (item->object.klass)->update)
		(* GTK_CANVAS_ITEM_CLASS (item->object.klass)->update) (
			item, child_affine, clip_path, child_flags);
}

/* This routine invokes the point method of the item.  The argument x, y should
 * be in the parent's item-relative coordinate system.  This routine applies the
 * inverse of the item's transform, maintaining the affine invariant.
 */
static double
gtk_canvas_item_invoke_point (GtkCanvasItem *item, double x, double y, int cx, int cy,
				GtkCanvasItem **actual_item)
{
#ifdef HACKISH_AFFINE
	double i2w[6], w2c[6], i2c[6], c2i[6];
	ArtPoint c, i;
#endif

#ifdef HACKISH_AFFINE
	gtk_canvas_item_i2w_affine (item, i2w);
	gtk_canvas_w2c_affine (item->canvas, w2c);
	art_affine_multiply (i2c, i2w, w2c);
	art_affine_invert (c2i, i2c);
	c.x = cx;
	c.y = cy;
	art_affine_point (&i, &c, c2i);
	x = i.x;
	y = i.y;
#endif

	return (* GTK_CANVAS_ITEM_CLASS (item->object.klass)->point) (
		item, x, y, cx, cy, actual_item);
}

/* Marshaler for the "event" signal of canvas items */
static void
gtk_canvas_item_marshal_signal_1 (GtkObject *object,
				    GtkSignalFunc func,
				    gpointer func_data,
				    GtkArg *args)
{
	GtkCanvasItemSignal1 rfunc;
	gint *return_val;

	rfunc = (GtkCanvasItemSignal1) func;
	return_val = GTK_RETLOC_BOOL (args[1]);

	*return_val = (* rfunc) (object,
				 GTK_VALUE_BOXED (args[0]),
				 func_data);
}


/**
 * gtk_canvas_item_set:
 * @item: A canvas item.
 * @first_arg_name: The list of object argument name/value pairs used to configure the item.
 * @Varargs:
 *
 * Configures a canvas item.  The arguments in the item are set to the specified
 * values, and the item is repainted as appropriate.
 **/
void
gtk_canvas_item_set (GtkCanvasItem *item, const gchar *first_arg_name, ...)
{
	va_list args;

	va_start (args, first_arg_name);
	gtk_canvas_item_set_valist (item, first_arg_name, args);
	va_end (args);
}


/**
 * gtk_canvas_item_set_valist:
 * @item: A canvas item.
 * @first_arg_name: The name of the first argument used to configure the item.
 * @args: The list of object argument name/value pairs used to configure the item.
 *
 * Configures a canvas item.  The arguments in the item are set to the specified
 * values, and the item is repainted as appropriate.
 **/
void
gtk_canvas_item_set_valist (GtkCanvasItem *item, const gchar *first_arg_name, va_list args)
{
	GSList *arg_list;
	GSList *info_list;
	char *error;

	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));

	arg_list = NULL;
	info_list = NULL;

	error = gtk_object_args_collect (GTK_OBJECT_TYPE (item),
					 &arg_list, &info_list, first_arg_name, args);

	if (error) {
		g_warning ("gtk_canvas_item_set(): %s", error);
		g_free (error);
	} else if (arg_list) {
		GSList *arg;
		GSList *info;
		GtkObject *object;

		redraw_if_visible (item);

		object = GTK_OBJECT (item);

		for (arg = arg_list, info = info_list; arg; arg = arg->next, info = info->next)
			gtk_object_arg_set (object, arg->data, info->data);

		gtk_args_collect_cleanup (arg_list, info_list);

		redraw_if_visible (item);
		item->canvas->need_repick = TRUE;
	}
}


/**
 * gtk_canvas_item_setv:
 * @item: A canvas item.
 * @nargs: The number of arguments used to configure the item.
 * @args: The arguments used to configure the item.
 *
 * Configures a canvas item.  The arguments in the item are set to the specified
 * values, and the item is repainted as appropriate.
 **/
void
gtk_canvas_item_setv (GtkCanvasItem *item, guint nargs, GtkArg *args)
{
	redraw_if_visible (item);
	gtk_object_setv (GTK_OBJECT (item), nargs, args);
	redraw_if_visible (item);

	item->canvas->need_repick = TRUE;
}

/**
 * gtk_canvas_item_affine_relative:
 * @item: A canvas item.
 * @affine: An affine transformation matrix.
 *
 * Combines the specified affine transformation matrix with the item's current
 * transformation.
 **/
#define GCIAR_EPSILON 1e-6
void
gtk_canvas_item_affine_relative (GtkCanvasItem *item, const double affine[6])
{
	double *new_affine;
	int i;

	if (fabs (affine[0] - 1.0) < GCIAR_EPSILON &&
	    fabs (affine[1]) < GCIAR_EPSILON &&
	    fabs (affine[2]) < GCIAR_EPSILON &&
	    fabs (affine[3] - 1.0) < GCIAR_EPSILON) {
		/* translation only */
		if (item->xform) {
			if (item->object.flags & GTK_CANVAS_ITEM_AFFINE_FULL) {
				item->xform[4] += affine[4];
				item->xform[5] += affine[5];
			} else {
				item->xform[0] += affine[4];
				item->xform[1] += affine[5];
			}
		} else {
			item->object.flags &= ~GTK_CANVAS_ITEM_AFFINE_FULL;
			new_affine = g_new (double, 2);
			new_affine[0] = affine[4];
			new_affine[1] = affine[5];
			item->xform = new_affine;
		}
	} else {
		/* need full affine */
		if (item->xform) {
			/* add to existing transform */
			if (item->object.flags & GTK_CANVAS_ITEM_AFFINE_FULL) {
				art_affine_multiply (item->xform, item->xform, affine);
			} else {
				item->object.flags |= GTK_CANVAS_ITEM_AFFINE_FULL;
				new_affine = g_new (double, 6);
				for (i = 0; i < 4; i++)
					new_affine[i] = affine[i];
				new_affine[4] = (item->xform[0] * affine[0]
						 + item->xform[1] * affine[2]
						 + affine[4]);
				new_affine[5] = (item->xform[0] * affine[1]
						 + item->xform[1] * affine[3]
						 + affine[5]);
				g_free (item->xform);
				item->xform = new_affine;
			}
		} else {
			item->object.flags |= GTK_CANVAS_ITEM_AFFINE_FULL;
			new_affine = g_new (double, 6);
			for (i = 0; i < 6; i++)
				new_affine[i] = affine[i];
			item->xform = new_affine;
		}
	}

	if (!(item->object.flags & GTK_CANVAS_ITEM_NEED_AFFINE)) {
		item->object.flags |= GTK_CANVAS_ITEM_NEED_AFFINE;
		if (item->parent != NULL)
			gtk_canvas_item_request_update (item->parent);
		else
			gtk_canvas_request_update (item->canvas);
	}

	item->canvas->need_repick = TRUE;
}

/**
 * gtk_canvas_item_affine_absolute:
 * @item: A canvas item.
 * @affine: An affine transformation matrix.
 *
 * Makes the item's affine transformation matrix be equal to the specified
 * matrix.
 **/
void
gtk_canvas_item_affine_absolute (GtkCanvasItem *item, const double affine[6])
{
	int i;

	if (fabs (affine[0] - 1.0) < GCIAR_EPSILON &&
	    fabs (affine[1]) < GCIAR_EPSILON &&
	    fabs (affine[2]) < GCIAR_EPSILON &&
	    fabs (affine[3] - 1.0) < GCIAR_EPSILON) {
		/* translation only */
		if (item->xform && (item->object.flags & GTK_CANVAS_ITEM_AFFINE_FULL)) {
			g_free (item->xform);
			item->xform = NULL;
		}
		if (item->xform == NULL) {
			item->object.flags &= ~GTK_CANVAS_ITEM_AFFINE_FULL;
			item->xform = g_new (double, 2);
		}
		item->xform[0] = affine[4];
		item->xform[1] = affine[5];
	} else {
		/* need full affine */
		if (item->xform && !(item->object.flags & GTK_CANVAS_ITEM_AFFINE_FULL)) {
			g_free (item->xform);
			item->xform = NULL;
		}
		if (item->xform == NULL) {
			item->object.flags |= GTK_CANVAS_ITEM_AFFINE_FULL;
			item->xform = g_new (double, 6);
		}
		for (i = 0; i < 6; i++) {
			item->xform[i] = affine[i];
		}
	}

	if (!(item->object.flags & GTK_CANVAS_ITEM_NEED_AFFINE)) {
		item->object.flags |= GTK_CANVAS_ITEM_NEED_AFFINE;
		if (item->parent != NULL)
			gtk_canvas_item_request_update (item->parent);
		else
			gtk_canvas_request_update (item->canvas);
	}

	item->canvas->need_repick = TRUE;
}


/**
 * gtk_canvas_item_move:
 * @item: A canvas item.
 * @dx: Horizontal offset.
 * @dy: Vertical offset.
 *
 * Moves a canvas item by creating an affine transformation matrix for
 * translation by using the specified values.
 **/
#ifndef OLD_XFORM
void
gtk_canvas_item_move (GtkCanvasItem *item, double dx, double dy)
{
	double translate[6];

	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));

	art_affine_translate (translate, dx, dy);

	gtk_canvas_item_affine_relative (item, translate);
}
#else
void
gtk_canvas_item_move (GtkCanvasItem *item, double dx, double dy)
{
	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));

	if (!GTK_CANVAS_ITEM_CLASS (item->object.klass)->translate) {
		g_warning ("Item type %s does not implement translate method.\n",
			   gtk_type_name (GTK_OBJECT_TYPE (item)));
		return;
	}

	if (!item->canvas->aa)
	redraw_if_visible (item);
	(* GTK_CANVAS_ITEM_CLASS (item->object.klass)->translate) (item, dx, dy);
	if (!item->canvas->aa)
	redraw_if_visible (item);

	item->canvas->need_repick = TRUE;
}
#endif

/* Convenience function to reorder items in a group's child list.  This puts the
 * specified link after the "before" link.
 */
static void
put_item_after (GList *link, GList *before)
{
	GtkCanvasGroup *parent;

	if (link == before)
		return;

	parent = GTK_CANVAS_GROUP (GTK_CANVAS_ITEM (link->data)->parent);

	if (before == NULL) {
		if (link == parent->item_list)
			return;

		link->prev->next = link->next;

		if (link->next)
			link->next->prev = link->prev;
		else
			parent->item_list_end = link->prev;

		link->prev = before;
		link->next = parent->item_list;
		link->next->prev = link;
		parent->item_list = link;
	} else {
		if ((link == parent->item_list_end) && (before == parent->item_list_end->prev))
			return;

		if (link->next)
			link->next->prev = link->prev;

		if (link->prev)
			link->prev->next = link->next;
		else {
			parent->item_list = link->next;
			parent->item_list->prev = NULL;
		}

		link->prev = before;
		link->next = before->next;

		link->prev->next = link;

		if (link->next)
			link->next->prev = link;
		else
			parent->item_list_end = link;
	}
}


/**
 * gtk_canvas_item_raise:
 * @item: A canvas item.
 * @positions: Number of steps to raise the item.
 *
 * Raises the item in its parent's stack by the specified number of positions.
 * If the number of positions is greater than the distance to the top of the
 * stack, then the item is put at the top.
 **/
void
gtk_canvas_item_raise (GtkCanvasItem *item, int positions)
{
	GList *link, *before;
	GtkCanvasGroup *parent;

	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));
	g_return_if_fail (positions >= 0);

	if (!item->parent || positions == 0)
		return;

	parent = GTK_CANVAS_GROUP (item->parent);
	link = g_list_find (parent->item_list, item);
	g_assert (link != NULL);

	for (before = link; positions && before; positions--)
		before = before->next;

	if (!before)
		before = parent->item_list_end;

	put_item_after (link, before);

	redraw_if_visible (item);
	item->canvas->need_repick = TRUE;
}


/**
 * gtk_canvas_item_lower:
 * @item: A canvas item.
 * @positions: Number of steps to lower the item.
 *
 * Lowers the item in its parent's stack by the specified number of positions.
 * If the number of positions is greater than the distance to the bottom of the
 * stack, then the item is put at the bottom.
 **/
void
gtk_canvas_item_lower (GtkCanvasItem *item, int positions)
{
	GList *link, *before;
	GtkCanvasGroup *parent;

	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));
	g_return_if_fail (positions >= 1);

	if (!item->parent || positions == 0)
		return;

	parent = GTK_CANVAS_GROUP (item->parent);
	link = g_list_find (parent->item_list, item);
	g_assert (link != NULL);

	if (link->prev)
		for (before = link->prev; positions && before; positions--)
			before = before->prev;
	else
		before = NULL;

	put_item_after (link, before);

	redraw_if_visible (item);
	item->canvas->need_repick = TRUE;
}


/**
 * gtk_canvas_item_raise_to_top:
 * @item: A canvas item.
 *
 * Raises an item to the top of its parent's stack.
 **/
void
gtk_canvas_item_raise_to_top (GtkCanvasItem *item)
{
	GList *link;
	GtkCanvasGroup *parent;

	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));

	if (!item->parent)
		return;

	parent = GTK_CANVAS_GROUP (item->parent);
	link = g_list_find (parent->item_list, item);
	g_assert (link != NULL);

	put_item_after (link, parent->item_list_end);

	redraw_if_visible (item);
	item->canvas->need_repick = TRUE;
}


/**
 * gtk_canvas_item_lower_to_bottom:
 * @item: A canvas item.
 *
 * Lowers an item to the bottom of its parent's stack.
 **/
void
gtk_canvas_item_lower_to_bottom (GtkCanvasItem *item)
{
	GList *link;
	GtkCanvasGroup *parent;

	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));

	if (!item->parent)
		return;

	parent = GTK_CANVAS_GROUP (item->parent);
	link = g_list_find (parent->item_list, item);
	g_assert (link != NULL);

	put_item_after (link, NULL);

	redraw_if_visible (item);
	item->canvas->need_repick = TRUE;
}


/**
 * gtk_canvas_item_show:
 * @item: A canvas item.
 *
 * Shows a canvas item.  If the item was already shown, then no action is taken.
 **/
void
gtk_canvas_item_show (GtkCanvasItem *item)
{
	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));

	if (item->object.flags & GTK_CANVAS_ITEM_VISIBLE)
		return;

	item->object.flags |= GTK_CANVAS_ITEM_VISIBLE;

	gtk_canvas_request_redraw (item->canvas, item->x1, item->y1, item->x2 + 1, item->y2 + 1);
	item->canvas->need_repick = TRUE;
}


/**
 * gtk_canvas_item_hide:
 * @item: A canvas item.
 *
 * Hides a canvas item.  If the item was already hidden, then no action is
 * taken.
 **/
void
gtk_canvas_item_hide (GtkCanvasItem *item)
{
	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));

	if (!(item->object.flags & GTK_CANVAS_ITEM_VISIBLE))
		return;

	item->object.flags &= ~GTK_CANVAS_ITEM_VISIBLE;

	gtk_canvas_request_redraw (item->canvas, item->x1, item->y1, item->x2 + 1, item->y2 + 1);
	item->canvas->need_repick = TRUE;
}


/**
 * gtk_canvas_item_grab:
 * @item: A canvas item.
 * @event_mask: Mask of events that will be sent to this item.
 * @cursor: If non-NULL, the cursor that will be used while the grab is active.
 * @etime: The timestamp required for grabbing the mouse, or GDK_CURRENT_TIME.
 *
 * Specifies that all events that match the specified event mask should be sent
 * to the specified item, and also grabs the mouse by calling
 * gdk_pointer_grab().  The event mask is also used when grabbing the pointer.
 * If @cursor is not NULL, then that cursor is used while the grab is active.
 * The @etime parameter is the timestamp required for grabbing the mouse.
 *
 * Return value: If an item was already grabbed, it returns %AlreadyGrabbed.  If
 * the specified item was hidden by calling gtk_canvas_item_hide(), then it
 * returns %GrabNotViewable.  Else, it returns the result of calling
 * gdk_pointer_grab().
 **/
int
gtk_canvas_item_grab (GtkCanvasItem *item, guint event_mask, GdkCursor *cursor, guint32 etime)
{
	int retval;

	g_return_val_if_fail (item != NULL, GrabNotViewable);
	g_return_val_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item), GrabNotViewable);
	g_return_val_if_fail (GTK_WIDGET_MAPPED (item->canvas), GrabNotViewable);

	if (item->canvas->grabbed_item)
		return AlreadyGrabbed;

	if (!(item->object.flags & GTK_CANVAS_ITEM_VISIBLE))
		return GrabNotViewable;

	retval = gdk_pointer_grab (item->canvas->layout.bin_window,
				   FALSE,
				   event_mask,
				   NULL,
				   cursor,
				   etime);

	if (retval != GrabSuccess)
		return retval;

	item->canvas->grabbed_item = item;
	item->canvas->grabbed_event_mask = event_mask;
	item->canvas->current_item = item; /* So that events go to the grabbed item */

	return retval;
}


/**
 * gtk_canvas_item_ungrab:
 * @item: A canvas item that holds a grab.
 * @etime: The timestamp for ungrabbing the mouse.
 *
 * Ungrabs the item, which must have been grabbed in the canvas, and ungrabs the
 * mouse.
 **/
void
gtk_canvas_item_ungrab (GtkCanvasItem *item, guint32 etime)
{
	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));

	if (item->canvas->grabbed_item != item)
		return;

	item->canvas->grabbed_item = NULL;

	gdk_pointer_ungrab (etime);
}


/**
 * gtk_canvas_item_i2w_affine:
 * @item: A canvas item
 * @affine: An affine transformation matrix (return value).
 *
 * Gets the affine transform that converts from the item's coordinate system to
 * world coordinates.
 **/
#ifdef OLD_XFORM
void
gtk_canvas_item_i2w_affine (GtkCanvasItem *item, double affine[6])
{
	GtkCanvasGroup *group;

	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));
	g_return_if_fail (affine != NULL);

	art_affine_identity (affine);

	while (item->parent) {
		group = GTK_CANVAS_GROUP (item->parent);

		affine[4] += group->xpos;
		affine[5] += group->ypos;

		item = item->parent;
	}
}
#else
void
gtk_canvas_item_i2w_affine (GtkCanvasItem *item, double affine[6])
{
	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));
	g_return_if_fail (affine != NULL);

	art_affine_identity (affine);

	while (item) {
		if (item->xform != NULL) {
			if (item->object.flags & GTK_CANVAS_ITEM_AFFINE_FULL) {
				art_affine_multiply (affine, affine, item->xform);
			} else {
				affine[4] += item->xform[0];
				affine[5] += item->xform[1];
			}
		}

		item = item->parent;
	}
}
#endif

/**
 * gtk_canvas_item_w2i:
 * @item: A canvas item.
 * @x: X coordinate to convert (input/output value).
 * @y: Y coordinate to convert (input/output value).
 *
 * Converts a coordinate pair from world coordinates to item-relative
 * coordinates.
 **/
void
gtk_canvas_item_w2i (GtkCanvasItem *item, double *x, double *y)
{
	double affine[6], inv[6];
	ArtPoint w, i;

	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));
	g_return_if_fail (x != NULL);
	g_return_if_fail (y != NULL);

	gtk_canvas_item_i2w_affine (item, affine);
	art_affine_invert (inv, affine);
	w.x = *x;
	w.y = *y;
	art_affine_point (&i, &w, inv);
	*x = i.x;
	*y = i.y;
}


/**
 * gtk_canvas_item_i2w:
 * @item: A canvas item.
 * @x: X coordinate to convert (input/output value).
 * @y: Y coordinate to convert (input/output value).
 *
 * Converts a coordinate pair from item-relative coordinates to world
 * coordinates.
 **/
void
gtk_canvas_item_i2w (GtkCanvasItem *item, double *x, double *y)
{
	double affine[6];
	ArtPoint w, i;

	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));
	g_return_if_fail (x != NULL);
	g_return_if_fail (y != NULL);

	gtk_canvas_item_i2w_affine (item, affine);
	i.x = *x;
	i.y = *y;
	art_affine_point (&w, &i, affine);
	*x = w.x;
	*y = w.y;
}

/**
 * gtk_canvas_item_i2c_affine:
 * @item: A canvas item.
 * @affine: An affine transformation matrix (return value).
 *
 * Gets the affine transform that converts from item-relative coordinates to
 * canvas pixel coordinates.
 **/
void
gtk_canvas_item_i2c_affine (GtkCanvasItem *item, double affine[6])
{
	double i2w[6], w2c[6];

	gtk_canvas_item_i2w_affine (item, i2w);
	gtk_canvas_w2c_affine (item->canvas, w2c);
	art_affine_multiply (affine, i2w, w2c);
}

/* Returns whether the item is an inferior of or is equal to the parent. */
static int
is_descendant (GtkCanvasItem *item, GtkCanvasItem *parent)
{
	for (; item; item = item->parent)
		if (item == parent)
			return TRUE;

	return FALSE;
}

/**
 * gtk_canvas_item_reparent:
 * @item: A canvas item.
 * @new_group: A canvas group.
 *
 * Changes the parent of the specified item to be the new group.  The item keeps
 * its group-relative coordinates as for its old parent, so the item may change
 * its absolute position within the canvas.
 **/
void
gtk_canvas_item_reparent (GtkCanvasItem *item, GtkCanvasGroup *new_group)
{
	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));
	g_return_if_fail (new_group != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_GROUP (new_group));

	/* Both items need to be in the same canvas */
	g_return_if_fail (item->canvas == GTK_CANVAS_ITEM (new_group)->canvas);

	/* The group cannot be an inferior of the item or be the item itself --
	 * this also takes care of the case where the item is the root item of
	 * the canvas.  */
	g_return_if_fail (!is_descendant (GTK_CANVAS_ITEM (new_group), item));

	/* Everything is ok, now actually reparent the item */

	gtk_object_ref (GTK_OBJECT (item)); /* protect it from the unref in group_remove */

	redraw_if_visible (item);

	group_remove (GTK_CANVAS_GROUP (item->parent), item);
	item->parent = GTK_CANVAS_ITEM (new_group);
	group_add (new_group, item);

	/* Rebuild the new parent group's bounds.  This will take care of
	 * reconfiguring the item and all its children.
	 */
	gtk_canvas_group_child_bounds (new_group, NULL);

	/* Redraw and repick */

	redraw_if_visible (item);
	item->canvas->need_repick = TRUE;

	gtk_object_unref (GTK_OBJECT (item));
}

/**
 * gtk_canvas_item_grab_focus:
 * @item: A canvas item.
 *
 * Makes the specified item take the keyboard focus, so all keyboard events will
 * be sent to it.  If the canvas widget itself did not have the focus, it grabs
 * it as well.
 **/
void
gtk_canvas_item_grab_focus (GtkCanvasItem *item)
{
	GtkCanvasItem *focused_item;
	GdkEvent ev;

	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));
	g_return_if_fail (GTK_WIDGET_CAN_FOCUS (GTK_WIDGET (item->canvas)));

	focused_item = item->canvas->focused_item;

	if (focused_item) {
		ev.focus_change.type = GDK_FOCUS_CHANGE;
		ev.focus_change.window = GTK_LAYOUT (item->canvas)->bin_window;
		ev.focus_change.send_event = FALSE;
		ev.focus_change.in = FALSE;

		emit_event (item->canvas, &ev);
	}

	item->canvas->focused_item = item;
	gtk_widget_grab_focus (GTK_WIDGET (item->canvas));

	if (focused_item) {
		ev.focus_change.type = GDK_FOCUS_CHANGE;
		ev.focus_change.window = GTK_LAYOUT (item->canvas)->bin_window;
		ev.focus_change.send_event = FALSE;
		ev.focus_change.in = TRUE;
		
		emit_event (item->canvas, &ev);
	}
}


/**
 * gtk_canvas_item_get_bounds:
 * @item: A canvas item.
 * @x1: Leftmost edge of the bounding box (return value).
 * @y1: Upper edge of the bounding box (return value).
 * @x2: Rightmost edge of the bounding box (return value).
 * @y2: Lower edge of the bounding box (return value).
 *
 * Queries the bounding box of a canvas item.  The bounds are returned in the
 * coordinate system of the item's parent.
 **/
void
gtk_canvas_item_get_bounds (GtkCanvasItem *item, double *x1, double *y1, double *x2, double *y2)
{
	double tx1, ty1, tx2, ty2;
	ArtPoint p1, p2, p3, p4;
	ArtPoint q1, q2, q3, q4;
	double min_x1, min_y1, min_x2, min_y2;
	double max_x1, max_y1, max_x2, max_y2;

	g_return_if_fail (item != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_ITEM (item));

	tx1 = ty1 = tx2 = ty2 = 0.0;

	/* Get the item's bounds in its coordinate system */

	if (GTK_CANVAS_ITEM_CLASS (item->object.klass)->bounds)
		(* GTK_CANVAS_ITEM_CLASS (item->object.klass)->bounds) (
			item, &tx1, &ty1, &tx2, &ty2);

	/* Make the bounds relative to the item's parent coordinate system */

	if (item->object.flags & GTK_CANVAS_ITEM_AFFINE_FULL) {
		p1.x = p2.x = tx1;
		p1.y = p4.y = ty1;
		p3.x = p4.x = tx2;
		p2.y = p3.y = ty2;

		art_affine_point (&q1, &p1, item->xform);
		art_affine_point (&q2, &p2, item->xform);
		art_affine_point (&q3, &p3, item->xform);
		art_affine_point (&q4, &p4, item->xform);

		if (q1.x < q2.x) {
			min_x1 = q1.x;
			max_x1 = q2.x;
		} else {
			min_x1 = q2.x;
			max_x1 = q1.x;
		}

		if (q1.y < q2.y) {
			min_y1 = q1.y;
			max_y1 = q2.y;
		} else {
			min_y1 = q2.y;
			max_y1 = q1.y;
		}

		if (q3.x < q4.x) {
			min_x2 = q3.x;
			max_x2 = q4.x;
		} else {
			min_x2 = q4.x;
			max_x2 = q3.x;
		}

		if (q3.y < q4.y) {
			min_y2 = q3.y;
			max_y2 = q4.y;
		} else {
			min_y2 = q4.y;
			max_y2 = q3.y;
		}

		tx1 = MIN (min_x1, min_x2);
		ty1 = MIN (min_y1, min_y2);
		tx2 = MAX (max_x1, max_x2);
		ty2 = MAX (max_y1, max_y2);
	} else if (item->xform) {
		tx1 += item->xform[0];
		ty1 += item->xform[1];
		tx2 += item->xform[0];
		ty2 += item->xform[1];
	}

	/* Return the values */

	if (x1)
		*x1 = tx1;

	if (y1)
		*y1 = ty1;

	if (x2)
		*x2 = tx2;

	if (y2)
		*y2 = ty2;
}


/**
 * gtk_canvas_item_request_update
 * @item: A canvas item.
 *
 * To be used only by item implementations.  Requests that the canvas queue an
 * update for the specified item.
 **/
void
gtk_canvas_item_request_update (GtkCanvasItem *item)
{
	if (item->object.flags & GTK_CANVAS_ITEM_NEED_UPDATE)
		return;

	item->object.flags |= GTK_CANVAS_ITEM_NEED_UPDATE;

	if (item->parent != NULL) {
		/* Recurse up the tree */
		gtk_canvas_item_request_update (item->parent);
	} else {
		/* Have reached the top of the tree, make sure the update call gets scheduled. */
		gtk_canvas_request_update (item->canvas);
	}
}

/*** GtkCanvasGroup ***/


enum {
	GROUP_ARG_0,
	GROUP_ARG_X,
	GROUP_ARG_Y
};


static void gtk_canvas_group_class_init  (GtkCanvasGroupClass *class);
static void gtk_canvas_group_init        (GtkCanvasGroup      *group);
static void gtk_canvas_group_set_arg     (GtkObject             *object,
					    GtkArg                *arg,
					    guint                  arg_id);
static void gtk_canvas_group_get_arg     (GtkObject             *object,
					    GtkArg                *arg,
					    guint                  arg_id);
static void gtk_canvas_group_destroy     (GtkObject             *object);

static void   gtk_canvas_group_update      (GtkCanvasItem *item, double *affine,
					      ArtSVP *clip_path, int flags);
static void   gtk_canvas_group_realize     (GtkCanvasItem *item);
static void   gtk_canvas_group_unrealize   (GtkCanvasItem *item);
static void   gtk_canvas_group_map         (GtkCanvasItem *item);
static void   gtk_canvas_group_unmap       (GtkCanvasItem *item);
static void   gtk_canvas_group_draw        (GtkCanvasItem *item, GdkDrawable *drawable,
					      int x, int y, int width, int height);
static double gtk_canvas_group_point       (GtkCanvasItem *item, double x, double y,
					      int cx, int cy,
					      GtkCanvasItem **actual_item);
static void   gtk_canvas_group_bounds      (GtkCanvasItem *item, double *x1, double *y1,
					      double *x2, double *y2);
static void   gtk_canvas_group_render      (GtkCanvasItem *item,
					      GtkCanvasBuf *buf);


static GtkCanvasItemClass *group_parent_class;


/**
 * gtk_canvas_group_get_type:
 *
 * Registers the &GtkCanvasGroup class if necessary, and returns the type ID
 * associated to it.
 *
 * Return value:  The type ID of the &GtkCanvasGroup class.
 **/
GtkType
gtk_canvas_group_get_type (void)
{
	static GtkType group_type = 0;

	if (!group_type) {
		static const GtkTypeInfo group_info = {
			"GtkCanvasGroup",
			sizeof (GtkCanvasGroup),
			sizeof (GtkCanvasGroupClass),
			(GtkClassInitFunc) gtk_canvas_group_class_init,
			(GtkObjectInitFunc) gtk_canvas_group_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		group_type = gtk_type_unique (gtk_canvas_item_get_type (), &group_info);
	}

	return group_type;
}

/* Class initialization function for GtkCanvasGroupClass */
static void
gtk_canvas_group_class_init (GtkCanvasGroupClass *class)
{
	GtkObjectClass *object_class;
	GtkCanvasItemClass *item_class;

	object_class = (GtkObjectClass *) class;
	item_class = (GtkCanvasItemClass *) class;

	group_parent_class = gtk_type_class (gtk_canvas_item_get_type ());

	gtk_object_add_arg_type ("GtkCanvasGroup::x", GTK_TYPE_DOUBLE, GTK_ARG_READWRITE,
				 GROUP_ARG_X);
	gtk_object_add_arg_type ("GtkCanvasGroup::y", GTK_TYPE_DOUBLE, GTK_ARG_READWRITE,
				 GROUP_ARG_Y);

	object_class->set_arg = gtk_canvas_group_set_arg;
	object_class->get_arg = gtk_canvas_group_get_arg;
	object_class->destroy = gtk_canvas_group_destroy;

	item_class->update = gtk_canvas_group_update;
	item_class->realize = gtk_canvas_group_realize;
	item_class->unrealize = gtk_canvas_group_unrealize;
	item_class->map = gtk_canvas_group_map;
	item_class->unmap = gtk_canvas_group_unmap;
	item_class->draw = gtk_canvas_group_draw;
	item_class->render = gtk_canvas_group_render;
	item_class->point = gtk_canvas_group_point;
	item_class->bounds = gtk_canvas_group_bounds;
}

/* Object initialization function for GtkCanvasGroup */
static void
gtk_canvas_group_init (GtkCanvasGroup *group)
{
#if 0
	group->xpos = 0.0;
	group->ypos = 0.0;
#endif
}

/* Translate handler for canvas groups */
static double *
gtk_canvas_ensure_translate (GtkCanvasItem *item)
{
	if (item->xform == NULL) {
		GTK_OBJECT_UNSET_FLAGS (item, GTK_CANVAS_ITEM_AFFINE_FULL);
		item->xform = g_new (double, 2);
		item->xform[0] = 0.0;
		item->xform[1] = 0.0;
		return item->xform;
	} else if (item->object.flags & GTK_CANVAS_ITEM_AFFINE_FULL) {
		return item->xform + 4;
	} else {
		return item->xform;
	}
}

/* Set_arg handler for canvas groups */
static void
gtk_canvas_group_set_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	GtkCanvasItem *item;
	GtkCanvasGroup *group;
	int recalc;
	double *xlat;

	item = GTK_CANVAS_ITEM (object);
	group = GTK_CANVAS_GROUP (object);

	recalc = FALSE;

	switch (arg_id) {
	case GROUP_ARG_X:
#ifdef OLD_XFORM
		group->xpos = GTK_VALUE_DOUBLE (*arg);
#else
		xlat = gtk_canvas_ensure_translate (item);
		xlat[0] = GTK_VALUE_DOUBLE (*arg);
#endif
		recalc = TRUE;
		break;

	case GROUP_ARG_Y:
#ifdef OLD_XFORM
		group->ypos = GTK_VALUE_DOUBLE (*arg);
#else
		xlat = gtk_canvas_ensure_translate (item);
		xlat[1] = GTK_VALUE_DOUBLE (*arg);
#endif
		recalc = TRUE;
		break;

	default:
		break;
	}

	if (recalc) {
		gtk_canvas_group_child_bounds (group, NULL);

		if (item->parent)
			gtk_canvas_group_child_bounds (GTK_CANVAS_GROUP (item->parent), item);
	}
}

/* Get_arg handler for canvas groups */
static void
gtk_canvas_group_get_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	GtkCanvasItem *item;
	GtkCanvasGroup *group;

	item = GTK_CANVAS_ITEM (object);
	group = GTK_CANVAS_GROUP (object);

	switch (arg_id) {
	case GROUP_ARG_X:
#ifdef OLD_XFORM
		GTK_VALUE_DOUBLE (*arg) = group->ypos;
#else
		if (item->xform == NULL)
			GTK_VALUE_DOUBLE (*arg) = 0;
		else if (object->flags & GTK_CANVAS_ITEM_AFFINE_FULL)
			GTK_VALUE_DOUBLE (*arg) = item->xform[4];
		else
			GTK_VALUE_DOUBLE (*arg) = item->xform[0];
#endif
		break;

	case GROUP_ARG_Y:
#ifdef OLD_XFORM
		GTK_VALUE_DOUBLE (*arg) = group->ypos;
#else
		if (item->xform == NULL)
			GTK_VALUE_DOUBLE (*arg) = 0;
		else if (object->flags & GTK_CANVAS_ITEM_AFFINE_FULL)
			GTK_VALUE_DOUBLE (*arg) = item->xform[5];
		else
			GTK_VALUE_DOUBLE (*arg) = item->xform[1];
#endif
		break;

	default:
		arg->type = GTK_TYPE_INVALID;
		break;
	}
}

/* Destroy handler for canvas groups */
static void
gtk_canvas_group_destroy (GtkObject *object)
{
	GtkCanvasGroup *group;
	GtkCanvasItem *child;
	GList *list;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_GROUP (object));

	group = GTK_CANVAS_GROUP (object);

	list = group->item_list;
	while (list) {
		child = list->data;
		list = list->next;

		gtk_object_destroy (GTK_OBJECT (child));
	}

	if (GTK_OBJECT_CLASS (group_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (group_parent_class)->destroy) (object);
}

/* Update handler for canvas groups */
static void
gtk_canvas_group_update (GtkCanvasItem *item, double *affine, ArtSVP *clip_path, int flags)
{
	GtkCanvasGroup *group;
	GList *list;
	GtkCanvasItem *i;
	ArtDRect bbox, child_bbox;

	group = GTK_CANVAS_GROUP (item);

	(* group_parent_class->update) (item, affine, clip_path, flags);

	bbox.x0 = 0;
	bbox.y0 = 0;
	bbox.x1 = 0;
	bbox.y1 = 0;

	for (list = group->item_list; list; list = list->next) {
		i = list->data;

		gtk_canvas_item_invoke_update (i, affine, clip_path, flags);

		child_bbox.x0 = i->x1;
		child_bbox.y0 = i->y1;
		child_bbox.x1 = i->x2;
		child_bbox.y1 = i->y2;
		art_drect_union (&bbox, &bbox, &child_bbox);
	}
	item->x1 = bbox.x0;
	item->y1 = bbox.y0;
	item->x2 = bbox.x1;
	item->y2 = bbox.y1;
}

/* Realize handler for canvas groups */
static void
gtk_canvas_group_realize (GtkCanvasItem *item)
{
	GtkCanvasGroup *group;
	GList *list;
	GtkCanvasItem *i;

	group = GTK_CANVAS_GROUP (item);

	for (list = group->item_list; list; list = list->next) {
		i = list->data;

		if (!(i->object.flags & GTK_CANVAS_ITEM_REALIZED))
			(* GTK_CANVAS_ITEM_CLASS (i->object.klass)->realize) (i);
	}

	(* group_parent_class->realize) (item);
}

/* Unrealize handler for canvas groups */
static void
gtk_canvas_group_unrealize (GtkCanvasItem *item)
{
	GtkCanvasGroup *group;
	GList *list;
	GtkCanvasItem *i;

	group = GTK_CANVAS_GROUP (item);

	for (list = group->item_list; list; list = list->next) {
		i = list->data;

		if (i->object.flags & GTK_CANVAS_ITEM_REALIZED)
			(* GTK_CANVAS_ITEM_CLASS (i->object.klass)->unrealize) (i);
	}

	(* group_parent_class->unrealize) (item);
}

/* Map handler for canvas groups */
static void
gtk_canvas_group_map (GtkCanvasItem *item)
{
	GtkCanvasGroup *group;
	GList *list;
	GtkCanvasItem *i;

	group = GTK_CANVAS_GROUP (item);

	for (list = group->item_list; list; list = list->next) {
		i = list->data;

		if (!(i->object.flags & GTK_CANVAS_ITEM_MAPPED))
			(* GTK_CANVAS_ITEM_CLASS (i->object.klass)->map) (i);
	}

	(* group_parent_class->map) (item);
}

/* Unmap handler for canvas groups */
static void
gtk_canvas_group_unmap (GtkCanvasItem *item)
{
	GtkCanvasGroup *group;
	GList *list;
	GtkCanvasItem *i;

	group = GTK_CANVAS_GROUP (item);

	for (list = group->item_list; list; list = list->next) {
		i = list->data;

		if (i->object.flags & GTK_CANVAS_ITEM_MAPPED)
			(* GTK_CANVAS_ITEM_CLASS (i->object.klass)->unmap) (i);
	}

	(* group_parent_class->unmap) (item);
}

/* Draw handler for canvas groups */
static void
gtk_canvas_group_draw (GtkCanvasItem *item, GdkDrawable *drawable,
			 int x, int y, int width, int height)
{
	GtkCanvasGroup *group;
	GList *list;
	GtkCanvasItem *child = 0;

	group = GTK_CANVAS_GROUP (item);

	for (list = group->item_list; list; list = list->next) {
		child = list->data;

		if (((child->object.flags & GTK_CANVAS_ITEM_VISIBLE)
		     && ((child->x1 < (x + width))
			 && (child->y1 < (y + height))
			 && (child->x2 > x)
			 && (child->y2 > y)))
		    || ((GTK_OBJECT_FLAGS (child) & GTK_CANVAS_ITEM_ALWAYS_REDRAW)
			&& (child->x1 < child->canvas->redraw_x2)
			&& (child->y1 < child->canvas->redraw_y2)
			&& (child->x2 > child->canvas->redraw_x1)
			&& (child->y2 > child->canvas->redraw_y2)))
			if (GTK_CANVAS_ITEM_CLASS (child->object.klass)->draw)
				(* GTK_CANVAS_ITEM_CLASS (child->object.klass)->draw) (
					child, drawable, x, y, width, height);
	}
}

/* Point handler for canvas groups */
static double
gtk_canvas_group_point (GtkCanvasItem *item, double x, double y, int cx, int cy,
			  GtkCanvasItem **actual_item)
{
	GtkCanvasGroup *group;
	GList *list;
	GtkCanvasItem *child, *point_item;
	int x1, y1, x2, y2;
	double gx, gy;
	double dist, best;
	int has_point;

	group = GTK_CANVAS_GROUP (item);

	x1 = cx - item->canvas->close_enough;
	y1 = cy - item->canvas->close_enough;
	x2 = cx + item->canvas->close_enough;
	y2 = cy + item->canvas->close_enough;

	best = 0.0;
	*actual_item = NULL;

#ifdef OLD_XFORM
	gx = x - group->xpos;
	gy = y - group->ypos;
#else
	gx = x;
	gy = y;
#endif

	dist = 0.0; /* keep gcc happy */

	for (list = group->item_list; list; list = list->next) {
		child = list->data;

		if ((child->x1 > x2) || (child->y1 > y2) || (child->x2 < x1) || (child->y2 < y1))
			continue;

		point_item = NULL; /* cater for incomplete item implementations */

		if ((child->object.flags & GTK_CANVAS_ITEM_VISIBLE)
		    && GTK_CANVAS_ITEM_CLASS (child->object.klass)->point) {
			dist = gtk_canvas_item_invoke_point (child, gx, gy, cx, cy, &point_item);
			has_point = TRUE;
		} else
			has_point = FALSE;

		if (has_point
		    && point_item
		    && ((int) (dist * item->canvas->pixels_per_unit + 0.5)
			<= item->canvas->close_enough)) {
			best = dist;
			*actual_item = point_item;
		}
	}

	return best;
}

/* Bounds handler for canvas groups */
static void
gtk_canvas_group_bounds (GtkCanvasItem *item, double *x1, double *y1, double *x2, double *y2)
{
	GtkCanvasGroup *group;
	GtkCanvasItem *child;
	GList *list;
	double tx1, ty1, tx2, ty2;
	double minx, miny, maxx, maxy;
	int set;

	group = GTK_CANVAS_GROUP (item);

	/* Get the bounds of the first visible item */

	child = NULL; /* Unnecessary but eliminates a warning. */

	set = FALSE;

	for (list = group->item_list; list; list = list->next) {
		child = list->data;

		if (child->object.flags & GTK_CANVAS_ITEM_VISIBLE) {
			set = TRUE;
			gtk_canvas_item_get_bounds (child, &minx, &miny, &maxx, &maxy);
			break;
		}
	}

	/* If there were no visible items, return an empty bounding box */

	if (!set) {
#ifdef OLD_XFORM
		*x1 = *x2 = group->xpos;
		*y1 = *y2 = group->ypos;
#endif
		*x1 = *y1 = *x2 = *y2 = 0.0;
		return;
	}

	/* Now we can grow the bounds using the rest of the items */

	list = list->next;

	for (; list; list = list->next) {
		child = list->data;

		if (!(child->object.flags & GTK_CANVAS_ITEM_VISIBLE))
			continue;

		gtk_canvas_item_get_bounds (child, &tx1, &ty1, &tx2, &ty2);

		if (tx1 < minx)
			minx = tx1;

		if (ty1 < miny)
			miny = ty1;

		if (tx2 > maxx)
			maxx = tx2;

		if (ty2 > maxy)
			maxy = ty2;
	}

#ifdef OLD_XFORM
	/* Make the bounds be relative to our parent's coordinate system */

	if (item->parent) {
		minx += group->xpos;
		miny += group->ypos;
		maxx += group->xpos;
		maxy += group->ypos;
	}
#endif

	*x1 = minx;
	*y1 = miny;
	*x2 = maxx;
	*y2 = maxy;
}

/* Render handler for canvas groups */
static void
gtk_canvas_group_render (GtkCanvasItem *item, GtkCanvasBuf *buf)
{
	GtkCanvasGroup *group;
	GtkCanvasItem *child;
	GList *list;

	group = GTK_CANVAS_GROUP (item);

	for (list = group->item_list; list; list = list->next) {
		child = list->data;

		if (((child->object.flags & GTK_CANVAS_ITEM_VISIBLE)
		     && ((child->x1 < buf->rect.x1)
			 && (child->y1 < buf->rect.y1)
			 && (child->x2 > buf->rect.x0)
			 && (child->y2 > buf->rect.y0)))
		    || ((GTK_OBJECT_FLAGS (child) & GTK_CANVAS_ITEM_ALWAYS_REDRAW)
			&& (child->x1 < child->canvas->redraw_x2)
			&& (child->y1 < child->canvas->redraw_y2)
			&& (child->x2 > child->canvas->redraw_x1)
			&& (child->y2 > child->canvas->redraw_y2)))
			if (GTK_CANVAS_ITEM_CLASS (child->object.klass)->render)
				(* GTK_CANVAS_ITEM_CLASS (child->object.klass)->render) (
					child, buf);
	}
}

/* Adds an item to a group */
static void
group_add (GtkCanvasGroup *group, GtkCanvasItem *item)
{
	gtk_object_ref (GTK_OBJECT (item));
	gtk_object_sink (GTK_OBJECT (item));

	if (!group->item_list) {
		group->item_list = g_list_append (group->item_list, item);
		group->item_list_end = group->item_list;
	} else
		group->item_list_end = g_list_append (group->item_list_end, item)->next;

	if (group->item.object.flags & GTK_CANVAS_ITEM_REALIZED)
		(* GTK_CANVAS_ITEM_CLASS (item->object.klass)->realize) (item);

	if (group->item.object.flags & GTK_CANVAS_ITEM_MAPPED)
		(* GTK_CANVAS_ITEM_CLASS (item->object.klass)->map) (item);

	gtk_canvas_group_child_bounds (group, item);
}

/* Removes an item from a group */
static void
group_remove (GtkCanvasGroup *group, GtkCanvasItem *item)
{
	GList *children;

	g_return_if_fail (group != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_GROUP (group));
	g_return_if_fail (item != NULL);

	for (children = group->item_list; children; children = children->next)
		if (children->data == item) {
			if (item->object.flags & GTK_CANVAS_ITEM_MAPPED)
				(* GTK_CANVAS_ITEM_CLASS (item->object.klass)->unmap) (item);

			if (item->object.flags & GTK_CANVAS_ITEM_REALIZED)
				(* GTK_CANVAS_ITEM_CLASS (item->object.klass)->unrealize) (item);

			/* Unparent the child */

			item->parent = NULL;
			gtk_object_unref (GTK_OBJECT (item));

			/* Remove it from the list */

			if (children == group->item_list_end)
				group->item_list_end = children->prev;

			group->item_list = g_list_remove_link (group->item_list, children);
			g_list_free (children);
			break;
		}
}


/**
 * gtk_canvas_group_child_bounds:
 * @group:
 * @item:
 *
 * Deprecated.
 **/
void
gtk_canvas_group_child_bounds (GtkCanvasGroup *group, GtkCanvasItem *item)
{
#if 0
	GtkCanvasItem *gitem;
	GList *list;
	int first;

	g_return_if_fail (group != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS_GROUP (group));
	g_return_if_fail (!item || GTK_CANVAS_IS_CANVAS_ITEM (item));

	gitem = GTK_CANVAS_ITEM (group);

	if (item) {
		/* Just add the child's bounds to whatever we have now */

		if ((item->x1 == item->x2) || (item->y1 == item->y2))
			return; /* empty bounding box */

		if (item->x1 < gitem->x1)
			gitem->x1 = item->x1;

		if (item->y1 < gitem->y1)
			gitem->y1 = item->y1;

		if (item->x2 > gitem->x2)
			gitem->x2 = item->x2;

		if (item->y2 > gitem->y2)
			gitem->y2 = item->y2;

		/* Propagate upwards */

		if (gitem->parent)
			gtk_canvas_group_child_bounds (GTK_CANVAS_GROUP (gitem->parent), gitem);
	} else {
		/* Rebuild every sub-group's bounds and reconstruct our own bounds */

		for (list = group->item_list, first = TRUE; list; list = list->next, first = FALSE) {
			item = list->data;

			if (GTK_CANVAS_IS_CANVAS_GROUP (item))
				gtk_canvas_group_child_bounds (GTK_CANVAS_GROUP (item), NULL);
#if 0
			else if (GTK_CANVAS_ITEM_CLASS (item->object.klass)->update)
					(* GTK_CANVAS_ITEM_CLASS (item->object.klass)->update) (item, NULL, NULL, 0);
#endif

			if (first) {
				gitem->x1 = item->x1;
				gitem->y1 = item->y1;
				gitem->x2 = item->x2;
				gitem->y2 = item->y2;
			} else {
				if (item->x1 < gitem->x1)
					gitem->x1 = item->x1;

				if (item->y1 < gitem->y1)
					gitem->y1 = item->y1;

				if (item->x2 > gitem->x2)
					gitem->x2 = item->x2;

				if (item->y2 > gitem->y2)
					gitem->y2 = item->y2;
			}
		}
	}
#endif
}


/*** GtkCanvas ***/


static void gtk_canvas_class_init     (GtkCanvasClass *class);
static void gtk_canvas_init           (GtkCanvas      *canvas);
static void gtk_canvas_destroy        (GtkObject        *object);
static void gtk_canvas_map            (GtkWidget        *widget);
static void gtk_canvas_unmap          (GtkWidget        *widget);
static void gtk_canvas_realize        (GtkWidget        *widget);
static void gtk_canvas_unrealize      (GtkWidget        *widget);
static void gtk_canvas_draw           (GtkWidget        *widget,
					 GdkRectangle     *area);
static void gtk_canvas_size_allocate  (GtkWidget        *widget,
					 GtkAllocation    *allocation);
static gint gtk_canvas_button         (GtkWidget        *widget,
					 GdkEventButton   *event);
static gint gtk_canvas_motion         (GtkWidget        *widget,
					 GdkEventMotion   *event);
static gint gtk_canvas_expose         (GtkWidget        *widget,
					 GdkEventExpose   *event);
static gint gtk_canvas_key            (GtkWidget        *widget,
					 GdkEventKey      *event);
static gint gtk_canvas_crossing       (GtkWidget        *widget,
					 GdkEventCrossing *event);

static gint gtk_canvas_focus_in       (GtkWidget        *widget,
					 GdkEventFocus    *event);
static gint gtk_canvas_focus_out      (GtkWidget        *widget,
					 GdkEventFocus    *event);

static GtkLayoutClass *canvas_parent_class;


#define DISPLAY_X1(canvas) (GTK_CANVAS (canvas)->layout.xoffset)
#define DISPLAY_Y1(canvas) (GTK_CANVAS (canvas)->layout.yoffset)



/**
 * gtk_canvas_get_type:
 *
 * Registers the &GtkCanvas class if necessary, and returns the type ID
 * associated to it.
 *
 * Return value:  The type ID of the &GtkCanvas class.
 **/
GtkType
gtk_canvas_get_type (void)
{
	static GtkType canvas_type = 0;

	if (!canvas_type) {
		static const GtkTypeInfo canvas_info = {
			"GtkCanvas",
			sizeof (GtkCanvas),
			sizeof (GtkCanvasClass),
			(GtkClassInitFunc) gtk_canvas_class_init,
			(GtkObjectInitFunc) gtk_canvas_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		canvas_type = gtk_type_unique (gtk_layout_get_type (), &canvas_info);
	}

	return canvas_type;
}

/* Class initialization function for GtkCanvasClass */
static void
gtk_canvas_class_init (GtkCanvasClass *class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	canvas_parent_class = gtk_type_class (gtk_layout_get_type ());

	object_class->destroy = gtk_canvas_destroy;

	widget_class->map = gtk_canvas_map;
	widget_class->unmap = gtk_canvas_unmap;
	widget_class->realize = gtk_canvas_realize;
	widget_class->unrealize = gtk_canvas_unrealize;
	widget_class->draw = gtk_canvas_draw;
	widget_class->size_allocate = gtk_canvas_size_allocate;
	widget_class->button_press_event = gtk_canvas_button;
	widget_class->button_release_event = gtk_canvas_button;
	widget_class->motion_notify_event = gtk_canvas_motion;
	widget_class->expose_event = gtk_canvas_expose;
	widget_class->key_press_event = gtk_canvas_key;
	widget_class->key_release_event = gtk_canvas_key;
	widget_class->enter_notify_event = gtk_canvas_crossing;
	widget_class->leave_notify_event = gtk_canvas_crossing;
	widget_class->focus_in_event = gtk_canvas_focus_in;
	widget_class->focus_out_event = gtk_canvas_focus_out;
}

/* Callback used when the root item of a canvas is destroyed.  The user should
 * never ever do this, so we panic if this happens.
 */
static void
panic_root_destroyed (GtkObject *object, gpointer data)
{
	g_error ("Eeeek, root item %p of canvas %p was destroyed!", object, data);
}

/* Object initialization function for GtkCanvas */
static void
gtk_canvas_init (GtkCanvas *canvas)
{
	GTK_WIDGET_SET_FLAGS (canvas, GTK_CAN_FOCUS);

	canvas->scroll_x1 = 0.0;
	canvas->scroll_y1 = 0.0;
	canvas->scroll_x2 = canvas->layout.width;
	canvas->scroll_y2 = canvas->layout.height;

	canvas->pixels_per_unit = 1.0;

	canvas->pick_event.type = GDK_LEAVE_NOTIFY;
	canvas->pick_event.crossing.x = 0;
	canvas->pick_event.crossing.y = 0;

	gtk_layout_set_hadjustment (GTK_LAYOUT (canvas), NULL);
	gtk_layout_set_vadjustment (GTK_LAYOUT (canvas), NULL);

#if 0
	canvas->ic = NULL;
	canvas->ic_attr = NULL;
#endif
	
	canvas->cc = gdk_color_context_new (gtk_widget_get_visual (GTK_WIDGET (canvas)),
					    gtk_widget_get_colormap (GTK_WIDGET (canvas)));

	/* Create the root item as a special case */

	canvas->root = GTK_CANVAS_ITEM (gtk_type_new (gtk_canvas_group_get_type ()));
	canvas->root->canvas = canvas;

	gtk_object_ref (GTK_OBJECT (canvas->root));
	gtk_object_sink (GTK_OBJECT (canvas->root));

	canvas->root_destroy_id = gtk_signal_connect (GTK_OBJECT (canvas->root), "destroy",
						      (GtkSignalFunc) panic_root_destroyed,
						      canvas);

	canvas->need_repick = TRUE;
}

/* Convenience function to remove the idle handler of a canvas */
static void
remove_idle (GtkCanvas *canvas)
{
	if (canvas->idle_id == 0)
		return;

	gtk_idle_remove (canvas->idle_id);
	canvas->idle_id = 0;
}

/* Removes the transient state of the canvas (idle handler, grabs). */
static void
shutdown_transients (GtkCanvas *canvas)
{
	/* We turn off the need_redraw flag, since if the canvas is mapped again
	 * it will request a redraw anyways.  We do not turn off the need_update
	 * flag, though, because updates are not queued when the canvas remaps
	 * itself.
	 */
	if (canvas->need_redraw) {
		canvas->need_redraw = FALSE;
		art_uta_free (canvas->redraw_area);
		canvas->redraw_area = NULL;
		canvas->redraw_x1 = 0;
		canvas->redraw_y1 = 0;
		canvas->redraw_x2 = 0;
		canvas->redraw_y2 = 0;
	}

	if (canvas->grabbed_item) {
		canvas->grabbed_item = NULL;
		gdk_pointer_ungrab (GDK_CURRENT_TIME);
	}

	remove_idle (canvas);
}

/* Destroy handler for GtkCanvas */
static void
gtk_canvas_destroy (GtkObject *object)
{
	GtkCanvas *canvas;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (object));

	canvas = GTK_CANVAS (object);

	gtk_signal_disconnect (GTK_OBJECT (canvas->root), canvas->root_destroy_id);
	gtk_object_unref (GTK_OBJECT (canvas->root));

	shutdown_transients (canvas);

	gdk_color_context_free (canvas->cc);

	if (GTK_OBJECT_CLASS (canvas_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (canvas_parent_class)->destroy) (object);
}

/**
 * gtk_canvas_new:
 * @void:
 *
 * Creates a new empty canvas in non-antialiased mode.  If you wish to use the
 * &GtkCanvasImage item inside this canvas, then you must push the gdk_imlib
 * visual and colormap before calling this function, and they can be popped
 * afterwards.
 *
 * Return value: A newly-created canvas.
 **/
GtkWidget *
gtk_canvas_new (void)
{
	return GTK_WIDGET (gtk_type_new (gtk_canvas_get_type ()));
}

/**
 * gtk_canvas_new_aa:
 * @void:
 *
 * Creates a new empty canvas in antialiased mode.  You should push the GdkRGB
 * visual and colormap before calling this functions, and they can be popped
 * afterwards.
 *
 * Return value: A newly-created antialiased canvas.
 **/
GtkWidget *
gtk_canvas_new_aa (void)
{
	GtkCanvas *canvas;

	gtk_widget_push_colormap (gdk_rgb_get_cmap ());
	gtk_widget_push_visual (gdk_rgb_get_visual ());
	canvas = gtk_type_new (gtk_canvas_get_type ());
	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
	
	canvas->aa = 1;
	return GTK_WIDGET (canvas);
}

/* Map handler for the canvas */
static void
gtk_canvas_map (GtkWidget *widget)
{
	GtkCanvas *canvas;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (widget));

	/* Normal widget mapping stuff */

	if (GTK_WIDGET_CLASS (canvas_parent_class)->map)
		(* GTK_WIDGET_CLASS (canvas_parent_class)->map) (widget);

	canvas = GTK_CANVAS (widget);

	/* Map items */

	if (GTK_CANVAS_ITEM_CLASS (canvas->root->object.klass)->map)
		(* GTK_CANVAS_ITEM_CLASS (canvas->root->object.klass)->map) (canvas->root);
}

/* Unmap handler for the canvas */
static void
gtk_canvas_unmap (GtkWidget *widget)
{
	GtkCanvas *canvas;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (widget));

	canvas = GTK_CANVAS (widget);

	shutdown_transients (canvas);

	/* Unmap items */

	if (GTK_CANVAS_ITEM_CLASS (canvas->root->object.klass)->unmap)
		(* GTK_CANVAS_ITEM_CLASS (canvas->root->object.klass)->unmap) (canvas->root);

	/* Normal widget unmapping stuff */

	if (GTK_WIDGET_CLASS (canvas_parent_class)->unmap)
		(* GTK_WIDGET_CLASS (canvas_parent_class)->unmap) (widget);
}

/* Realize handler for the canvas */
static void
gtk_canvas_realize (GtkWidget *widget)
{
	GtkCanvas *canvas;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (widget));

	/* Normal widget realization stuff */

	if (GTK_WIDGET_CLASS (canvas_parent_class)->realize)
		(* GTK_WIDGET_CLASS (canvas_parent_class)->realize) (widget);

	canvas = GTK_CANVAS (widget);

	gdk_window_set_events (canvas->layout.bin_window,
			       (gdk_window_get_events (canvas->layout.bin_window)
				 | GDK_EXPOSURE_MASK
				 | GDK_BUTTON_PRESS_MASK
				 | GDK_BUTTON_RELEASE_MASK
				 | GDK_POINTER_MOTION_MASK
				 | GDK_KEY_PRESS_MASK
				 | GDK_KEY_RELEASE_MASK
				 | GDK_ENTER_NOTIFY_MASK
				 | GDK_LEAVE_NOTIFY_MASK
				 | GDK_FOCUS_CHANGE_MASK));

#if 0
	if (gdk_im_ready () && (canvas->ic_attr = gdk_ic_attr_new ()) != NULL) {
		GdkEventMask mask;
		GdkICAttr *attr = canvas->ic_attr;
		GdkICAttributesType attrmask = GDK_IC_ALL_REQ;
		GdkIMStyle style;
		GdkIMStyle supported_style = GDK_IM_PREEDIT_NONE |
			GDK_IM_PREEDIT_NOTHING |
			GDK_IM_STATUS_NONE |
			GDK_IM_STATUS_NOTHING;

		attr->style = style = gdk_im_decide_style (supported_style);
		attr->client_window = canvas->layout.bin_window;

		canvas->ic = gdk_ic_new (attr, attrmask);
		if (canvas->ic != NULL) {
			mask = gdk_window_get_events (attr->client_window);
			mask |= gdk_ic_get_events (canvas->ic);
			gdk_window_set_events (attr->client_window, mask);

			if (GTK_WIDGET_HAS_FOCUS (widget))
				gdk_im_begin (canvas->ic, attr->client_window);
		} else
			g_warning ("Can't create input context.");
	}
#endif
	
	/* Create our own temporary pixmap gc and realize all the items */

	canvas->pixmap_gc = gdk_gc_new (canvas->layout.bin_window);

	(* GTK_CANVAS_ITEM_CLASS (canvas->root->object.klass)->realize) (canvas->root);
}

/* Unrealize handler for the canvas */
static void
gtk_canvas_unrealize (GtkWidget *widget)
{
	GtkCanvas *canvas;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (widget));

	canvas = GTK_CANVAS (widget);

	shutdown_transients (canvas);

#if 0
	if (canvas->ic) {
		gdk_ic_destroy (canvas->ic);
		canvas->ic = NULL;
	}
	if (canvas->ic_attr) {
		gdk_ic_attr_destroy (canvas->ic_attr);
		canvas->ic_attr = NULL;
	}
#endif
	
	/* Unrealize items and parent widget */

	(* GTK_CANVAS_ITEM_CLASS (canvas->root->object.klass)->unrealize) (canvas->root);

	gdk_gc_destroy (canvas->pixmap_gc);
	canvas->pixmap_gc = NULL;

	if (GTK_WIDGET_CLASS (canvas_parent_class)->unrealize)
		(* GTK_WIDGET_CLASS (canvas_parent_class)->unrealize) (widget);
}

/* Draw handler for the canvas */
static void
gtk_canvas_draw (GtkWidget *widget, GdkRectangle *area)
{
	GtkCanvas *canvas;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (widget));

	if (!GTK_WIDGET_DRAWABLE (widget))
		return;

	canvas = GTK_CANVAS (widget);

	gtk_canvas_request_redraw (canvas,
				     area->x + DISPLAY_X1 (canvas) - canvas->zoom_xofs,
				     area->y + DISPLAY_Y1 (canvas) - canvas->zoom_yofs,
				     (area->x + area->width + DISPLAY_X1 (canvas)
				      - canvas->zoom_xofs) + 1,
				     (area->y + area->height + DISPLAY_Y1 (canvas)
				      - canvas->zoom_yofs) + 1);
}

/* Handles scrolling of the canvas.  Adjusts the scrolling and zooming offset to
 * keep as much as possible of the canvas scrolling region in view.
 */
static void
scroll_to (GtkCanvas *canvas, int cx, int cy)
{
	int scroll_width, scroll_height;
	int right_limit, bottom_limit;
	int old_zoom_xofs, old_zoom_yofs;
	int changed, changed_x, changed_y;
	int canvas_width, canvas_height;

	canvas_width = GTK_WIDGET (canvas)->allocation.width;
	canvas_height = GTK_WIDGET (canvas)->allocation.height;

	gtk_canvas_w2c (canvas, canvas->scroll_x2, canvas->scroll_y2,
			  &scroll_width, &scroll_height);

	/* The values computed indicate the maximum pixel offset, so we add one
	 * to get the width and height.
	 */
	scroll_width++;
	scroll_height++;

	right_limit = scroll_width - canvas_width;
	bottom_limit = scroll_height - canvas_height;

	old_zoom_xofs = canvas->zoom_xofs;
	old_zoom_yofs = canvas->zoom_yofs;

	if (right_limit < 0) {
		cx = 0;
		canvas->zoom_xofs = (canvas_width - scroll_width) / 2;
		scroll_width = canvas_width;
	} else if (cx < 0) {
		cx = 0;
		canvas->zoom_xofs = 0;
	} else if (cx > right_limit) {
		cx = right_limit;
		canvas->zoom_xofs = 0;
	} else
		canvas->zoom_xofs = 0;

	if (bottom_limit < 0) {
		cy = 0;
		canvas->zoom_yofs = (canvas_height - scroll_height) / 2;
		scroll_height = canvas_height;
	} else if (cy < 0) {
		cy = 0;
		canvas->zoom_yofs = 0;
	} else if (cy > bottom_limit) {
		cy = bottom_limit;
		canvas->zoom_yofs = 0;
	} else
		canvas->zoom_yofs = 0;

	changed_x = ((int) canvas->layout.hadjustment->value) != cx;
	changed_y = ((int) canvas->layout.vadjustment->value) != cy;

	changed = ((canvas->zoom_xofs != old_zoom_xofs)
		   || (canvas->zoom_yofs != old_zoom_yofs)
		   || (changed_x && changed_y));

	if (changed)
		gtk_layout_freeze (GTK_LAYOUT (canvas));

	if ((scroll_width != (int) canvas->layout.width)
	    || (scroll_height != (int) canvas->layout.height))
		gtk_layout_set_size (GTK_LAYOUT (canvas), scroll_width, scroll_height);

	if (changed_x) {
		canvas->layout.hadjustment->value = cx;
		gtk_signal_emit_by_name (GTK_OBJECT (canvas->layout.hadjustment), "value_changed");
	}

	if (changed_y) {
		canvas->layout.vadjustment->value = cy;
		gtk_signal_emit_by_name (GTK_OBJECT (canvas->layout.vadjustment), "value_changed");
	}

	if (changed)
		gtk_layout_thaw (GTK_LAYOUT (canvas));
}

/* Size allocation handler for the canvas */
static void
gtk_canvas_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GtkCanvas *canvas;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (widget));
	g_return_if_fail (allocation != NULL);

	if (GTK_WIDGET_CLASS (canvas_parent_class)->size_allocate)
		(* GTK_WIDGET_CLASS (canvas_parent_class)->size_allocate) (widget, allocation);

	canvas = GTK_CANVAS (widget);

	/* Recenter the view, if appropriate */

	scroll_to (canvas, DISPLAY_X1 (canvas), DISPLAY_Y1 (canvas));

	canvas->layout.hadjustment->page_size = allocation->width;
	canvas->layout.hadjustment->page_increment = allocation->width / 2;
	gtk_signal_emit_by_name (GTK_OBJECT (canvas->layout.hadjustment), "changed");

	canvas->layout.vadjustment->page_size = allocation->height;
	canvas->layout.vadjustment->page_increment = allocation->height / 2;
	gtk_signal_emit_by_name (GTK_OBJECT (canvas->layout.vadjustment), "changed");
}

/* Emits an event for an item in the canvas, be it the current item, grabbed
 * item, or focused item, as appropriate.
 */
static int
emit_event (GtkCanvas *canvas, GdkEvent *event)
{
	GdkEvent ev;
	gint finished;
	GtkCanvasItem *item;
	GtkCanvasItem *parent;
	guint mask;

	/* Perform checks for grabbed items */

	if (canvas->grabbed_item && !is_descendant (canvas->current_item, canvas->grabbed_item))
		return FALSE;

	if (canvas->grabbed_item) {
		switch (event->type) {
		case GDK_ENTER_NOTIFY:
			mask = GDK_ENTER_NOTIFY_MASK;
			break;

		case GDK_LEAVE_NOTIFY:
			mask = GDK_LEAVE_NOTIFY_MASK;
			break;

		case GDK_MOTION_NOTIFY:
			mask = GDK_POINTER_MOTION_MASK;
			break;

		case GDK_BUTTON_PRESS:
		case GDK_2BUTTON_PRESS:
		case GDK_3BUTTON_PRESS:
			mask = GDK_BUTTON_PRESS_MASK;
			break;

		case GDK_BUTTON_RELEASE:
			mask = GDK_BUTTON_RELEASE_MASK;
			break;

		case GDK_KEY_PRESS:
			mask = GDK_KEY_PRESS_MASK;
			break;

		case GDK_KEY_RELEASE:
			mask = GDK_KEY_RELEASE_MASK;
			break;

		default:
			mask = 0;
			break;
		}

		if (!(mask & canvas->grabbed_event_mask))
			return FALSE;
	}

	/* Convert to world coordinates -- we have two cases because of diferent
	 * offsets of the fields in the event structures.
	 */

	ev = *event;

	switch (ev.type) {
	case GDK_ENTER_NOTIFY:
	case GDK_LEAVE_NOTIFY:
		gtk_canvas_window_to_world (canvas,
					      ev.crossing.x, ev.crossing.y,
					      &ev.crossing.x, &ev.crossing.y);
		break;

	case GDK_MOTION_NOTIFY:
	case GDK_BUTTON_PRESS:
	case GDK_2BUTTON_PRESS:
	case GDK_3BUTTON_PRESS:
	case GDK_BUTTON_RELEASE:
		gtk_canvas_window_to_world (canvas,
					      ev.motion.x, ev.motion.y,
					      &ev.motion.x, &ev.motion.y);
		break;

	default:
		break;
	}

	/* Choose where we send the event */

	item = canvas->current_item;

	if (canvas->focused_item
	    && ((event->type == GDK_KEY_PRESS)
		|| (event->type == GDK_KEY_RELEASE)
		|| (event->type == GDK_FOCUS_CHANGE)))
		item = canvas->focused_item;

	/* The event is propagated up the hierarchy (for if someone connected to
	 * a group instead of a leaf event), and emission is stopped if a
	 * handler returns TRUE, just like for GtkWidget events.
	 */

	finished = FALSE;

	while (item && !finished) {
		gtk_object_ref (GTK_OBJECT (item));

		gtk_signal_emit (GTK_OBJECT (item), item_signals[ITEM_EVENT],
				 &ev,
				 &finished);

		if (GTK_OBJECT_DESTROYED (item))
			finished = TRUE;

		parent = item->parent;
		gtk_object_unref (GTK_OBJECT (item));

		item = parent;
	}

	return finished;
}

/* Re-picks the current item in the canvas, based on the event's coordinates.
 * Also emits enter/leave events for items as appropriate.
 */
static int
pick_current_item (GtkCanvas *canvas, GdkEvent *event)
{
	int button_down;
	double x, y;
	int cx, cy;
	int retval;

	retval = FALSE;

	/* If a button is down, we'll perform enter and leave events on the
	 * current item, but not enter on any other item.  This is more or less
	 * like X pointer grabbing for canvas items.
	 */
	button_down = canvas->state & (GDK_BUTTON1_MASK
				       | GDK_BUTTON2_MASK
				       | GDK_BUTTON3_MASK
				       | GDK_BUTTON4_MASK
				       | GDK_BUTTON5_MASK);
	if (!button_down)
		canvas->left_grabbed_item = FALSE;

	/* Save the event in the canvas.  This is used to synthesize enter and
	 * leave events in case the current item changes.  It is also used to
	 * re-pick the current item if the current one gets deleted.  Also,
	 * synthesize an enter event.
	 */
	if (event != &canvas->pick_event) {
		if ((event->type == GDK_MOTION_NOTIFY) || (event->type == GDK_BUTTON_RELEASE)) {
			/* these fields have the same offsets in both types of events */

			canvas->pick_event.crossing.type       = GDK_ENTER_NOTIFY;
			canvas->pick_event.crossing.window     = event->motion.window;
			canvas->pick_event.crossing.send_event = event->motion.send_event;
			canvas->pick_event.crossing.subwindow  = NULL;
			canvas->pick_event.crossing.x          = event->motion.x;
			canvas->pick_event.crossing.y          = event->motion.y;
			canvas->pick_event.crossing.mode       = GDK_CROSSING_NORMAL;
			canvas->pick_event.crossing.detail     = GDK_NOTIFY_NONLINEAR;
			canvas->pick_event.crossing.focus      = FALSE;
			canvas->pick_event.crossing.state      = event->motion.state;

			/* these fields don't have the same offsets in both types of events */

			if (event->type == GDK_MOTION_NOTIFY) {
				canvas->pick_event.crossing.x_root = event->motion.x_root;
				canvas->pick_event.crossing.y_root = event->motion.y_root;
			} else {
				canvas->pick_event.crossing.x_root = event->button.x_root;
				canvas->pick_event.crossing.y_root = event->button.y_root;
			}
		} else
			canvas->pick_event = *event;
	}

	/* Don't do anything else if this is a recursive call */

	if (canvas->in_repick)
		return retval;

	/* LeaveNotify means that there is no current item, so we don't look for one */

	if (canvas->pick_event.type != GDK_LEAVE_NOTIFY) {
		/* these fields don't have the same offsets in both types of events */

		if (canvas->pick_event.type == GDK_ENTER_NOTIFY) {
			x = canvas->pick_event.crossing.x + DISPLAY_X1 (canvas) - canvas->zoom_xofs;
			y = canvas->pick_event.crossing.y + DISPLAY_Y1 (canvas) - canvas->zoom_yofs;
		} else {
			x = canvas->pick_event.motion.x + DISPLAY_X1 (canvas) - canvas->zoom_xofs;
			y = canvas->pick_event.motion.y + DISPLAY_Y1 (canvas) - canvas->zoom_yofs;
		}

		/* canvas pixel coords */

		cx = (int) (x + 0.5);
		cy = (int) (y + 0.5);

		/* world coords */

		x = canvas->scroll_x1 + x / canvas->pixels_per_unit;
		y = canvas->scroll_y1 + y / canvas->pixels_per_unit;

		/* find the closest item */

		if (canvas->root->object.flags & GTK_CANVAS_ITEM_VISIBLE)
			gtk_canvas_item_invoke_point (canvas->root, x, y, cx, cy,
							&canvas->new_current_item);
		else
			canvas->new_current_item = NULL;
	} else
		canvas->new_current_item = NULL;

	if ((canvas->new_current_item == canvas->current_item) && !canvas->left_grabbed_item)
		return retval; /* current item did not change */

	/* Synthesize events for old and new current items */

	if ((canvas->new_current_item != canvas->current_item)
	    && (canvas->current_item != NULL)
	    && !canvas->left_grabbed_item) {
		GdkEvent new_event;
		GtkCanvasItem *item;

		item = canvas->current_item;

		new_event = canvas->pick_event;
		new_event.type = GDK_LEAVE_NOTIFY;

		new_event.crossing.detail = GDK_NOTIFY_ANCESTOR;
		new_event.crossing.subwindow = NULL;
		canvas->in_repick = TRUE;
		retval = emit_event (canvas, &new_event);
		canvas->in_repick = FALSE;
	}

	/* new_current_item may have been set to NULL during the call to emit_event() above */

	if ((canvas->new_current_item != canvas->current_item) && button_down) {
		canvas->left_grabbed_item = TRUE;
		return retval;
	}

	/* Handle the rest of cases */

	canvas->left_grabbed_item = FALSE;
	canvas->current_item = canvas->new_current_item;

	if (canvas->current_item != NULL) {
		GdkEvent new_event;

		new_event = canvas->pick_event;
		new_event.type = GDK_ENTER_NOTIFY;
		new_event.crossing.detail = GDK_NOTIFY_ANCESTOR;
		new_event.crossing.subwindow = NULL;
		retval = emit_event (canvas, &new_event);
	}

	return retval;
}

/* Button event handler for the canvas */
static gint
gtk_canvas_button (GtkWidget *widget, GdkEventButton *event)
{
	GtkCanvas *canvas;
	int mask;
	int retval;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_CANVAS_IS_CANVAS (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	retval = FALSE;

	canvas = GTK_CANVAS (widget);

	if (event->window != canvas->layout.bin_window)
		return retval;

	switch (event->button) {
	case 1:
		mask = GDK_BUTTON1_MASK;
		break;
	case 2:
		mask = GDK_BUTTON2_MASK;
		break;
	case 3:
		mask = GDK_BUTTON3_MASK;
		break;
	case 4:
		mask = GDK_BUTTON4_MASK;
		break;
	case 5:
		mask = GDK_BUTTON5_MASK;
		break;
	default:
		mask = 0;
	}

	switch (event->type) {
	case GDK_BUTTON_PRESS:
	case GDK_2BUTTON_PRESS:
	case GDK_3BUTTON_PRESS:
		/* Pick the current item as if the button were not pressed, and
		 * then process the event.
		 */
		canvas->state = event->state;
		pick_current_item (canvas, (GdkEvent *) event);
		canvas->state ^= mask;
		retval = emit_event (canvas, (GdkEvent *) event);
		break;

	case GDK_BUTTON_RELEASE:
		/* Process the event as if the button were pressed, then repick
		 * after the button has been released
		 */
		canvas->state = event->state;
		retval = emit_event (canvas, (GdkEvent *) event);
		event->state ^= mask;
		canvas->state = event->state;
		pick_current_item (canvas, (GdkEvent *) event);
		event->state ^= mask;
		break;

	default:
		g_assert_not_reached ();
	}

	return retval;
}

/* Motion event handler for the canvas */
static gint
gtk_canvas_motion (GtkWidget *widget, GdkEventMotion *event)
{
	GtkCanvas *canvas;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_CANVAS_IS_CANVAS (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	canvas = GTK_CANVAS (widget);

	if (event->window != canvas->layout.bin_window)
		return FALSE;

	canvas->state = event->state;
	pick_current_item (canvas, (GdkEvent *) event);
	return emit_event (canvas, (GdkEvent *) event);
}

/* Expose handler for the canvas */
static gint
gtk_canvas_expose (GtkWidget *widget, GdkEventExpose *event)
{
	GtkCanvas *canvas;
	ArtIRect rect;
	ArtUta *uta;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_CANVAS_IS_CANVAS (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	canvas = GTK_CANVAS (widget);

	if (!GTK_WIDGET_DRAWABLE (widget) || (event->window != canvas->layout.bin_window))
		return FALSE;

	rect.x0 = event->area.x + DISPLAY_X1 (canvas) - canvas->zoom_xofs;
	rect.y0 = event->area.y + DISPLAY_Y1 (canvas) - canvas->zoom_yofs;
	rect.x1 = event->area.x + event->area.width + DISPLAY_X1 (canvas) - canvas->zoom_xofs;
	rect.y1 = event->area.y + event->area.height + DISPLAY_Y1 (canvas) - canvas->zoom_yofs;

	uta = art_uta_from_irect (&rect);
	gtk_canvas_request_redraw_uta (canvas, uta);
	gtk_canvas_update_now (canvas);

	return FALSE;
}

/* Key event handler for the canvas */
static gint
gtk_canvas_key (GtkWidget *widget, GdkEventKey *event)
{
	GtkCanvas *canvas;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_CANVAS_IS_CANVAS (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	canvas = GTK_CANVAS (widget);
	return emit_event (canvas, (GdkEvent *) event);
}

/* Crossing event handler for the canvas */
static gint
gtk_canvas_crossing (GtkWidget *widget, GdkEventCrossing *event)
{
	GtkCanvas *canvas;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_CANVAS_IS_CANVAS (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	canvas = GTK_CANVAS (widget);

	if (event->window != canvas->layout.bin_window)
		return FALSE;

	canvas->state = event->state;
	return pick_current_item (canvas, (GdkEvent *) event);
}

/* Focus in handler for the canvas */
static gint
gtk_canvas_focus_in (GtkWidget *widget, GdkEventFocus *event)
{
	GtkCanvas *canvas;

	canvas = GTK_CANVAS (widget);

#if 0
	if (canvas->ic)
		gdk_im_begin (canvas->ic, canvas->layout.bin_window);
#endif
	
	if (canvas->focused_item)
		return emit_event (canvas, (GdkEvent *) event);
	else
		return FALSE;
}

/* Focus out handler for the canvas */
static gint
gtk_canvas_focus_out (GtkWidget *widget, GdkEventFocus *event)
{
	GtkCanvas *canvas;

	canvas = GTK_CANVAS (widget);

	gdk_im_end ();

	if (canvas->focused_item)
		return emit_event (canvas, (GdkEvent *) event);
	else
		return FALSE;
}

#define IMAGE_WIDTH 512
#define IMAGE_HEIGHT 512

#define IMAGE_WIDTH_AA 256
#define IMAGE_HEIGHT_AA 64

/* Repaints the areas in the canvas that need it */
static void
paint (GtkCanvas *canvas)
{
	GtkWidget *widget;
	int draw_x1, draw_y1;
	int draw_x2, draw_y2;
	int width, height;
	GdkPixmap *pixmap;
	ArtIRect *rects;
	gint n_rects, i;

	if (canvas->need_update) {
		double affine[6];

		art_affine_identity (affine);
		gtk_canvas_item_invoke_update (canvas->root, affine, NULL, 0);
		canvas->need_update = FALSE;
	}

	if (!canvas->need_redraw)
		return;

	if (canvas->aa)
		rects = art_rect_list_from_uta (canvas->redraw_area, IMAGE_WIDTH_AA, IMAGE_HEIGHT_AA,
						&n_rects);
	else
		rects = art_rect_list_from_uta (canvas->redraw_area, IMAGE_WIDTH, IMAGE_HEIGHT,
						&n_rects);

	art_uta_free (canvas->redraw_area);

	canvas->redraw_area = NULL;

	widget = GTK_WIDGET (canvas);

	for (i = 0; i < n_rects; i++) {
		canvas->redraw_x1 = rects[i].x0;
		canvas->redraw_y1 = rects[i].y0;
		canvas->redraw_x2 = rects[i].x1;
		canvas->redraw_y2 = rects[i].y1;

		draw_x1 = DISPLAY_X1 (canvas) - canvas->zoom_xofs;
		draw_y1 = DISPLAY_Y1 (canvas) - canvas->zoom_yofs;
		draw_x2 = draw_x1 + GTK_WIDGET (canvas)->allocation.width;
		draw_y2 = draw_y1 + GTK_WIDGET (canvas)->allocation.height;

		if (canvas->redraw_x1 > draw_x1)
			draw_x1 = canvas->redraw_x1;

		if (canvas->redraw_y1 > draw_y1)
			draw_y1 = canvas->redraw_y1;

		if (canvas->redraw_x2 < draw_x2)
			draw_x2 = canvas->redraw_x2;

		if (canvas->redraw_y2 < draw_y2)
			draw_y2 = canvas->redraw_y2;

		if ((draw_x1 < draw_x2) && (draw_y1 < draw_y2)) {
			canvas->draw_xofs = draw_x1;
			canvas->draw_yofs = draw_y1;

			width = draw_x2 - draw_x1;
			height = draw_y2 - draw_y1;

			if (canvas->aa) {
				GtkCanvasBuf buf;
				GdkColor *color;

				buf.buf = g_new (guchar, IMAGE_WIDTH_AA * IMAGE_HEIGHT_AA * 3);
				buf.buf_rowstride = IMAGE_WIDTH_AA * 3;
				buf.rect.x0 = draw_x1;
				buf.rect.y0 = draw_y1;
				buf.rect.x1 = draw_x2;
				buf.rect.y1 = draw_y2;
				color = &widget->style->bg[GTK_STATE_NORMAL];
				buf.bg_color = (((color->red & 0xff00) << 8)
						| (color->green & 0xff00)
						| (color->blue >> 8));
				buf.is_bg = 1;
				buf.is_buf = 0;

				if (canvas->root->object.flags & GTK_CANVAS_ITEM_VISIBLE)
					(* GTK_CANVAS_ITEM_CLASS (
						canvas->root->object.klass)->render) (
							canvas->root, &buf);

				if (buf.is_bg) {
					gdk_rgb_gc_set_foreground (canvas->pixmap_gc, buf.bg_color);
					gdk_draw_rectangle (canvas->layout.bin_window,
							    canvas->pixmap_gc,
							    TRUE,
							    (draw_x1 - DISPLAY_X1 (canvas)
							     + canvas->zoom_xofs),
							    (draw_y1 - DISPLAY_Y1 (canvas)
							     + canvas->zoom_yofs),
							    width, height);
				} else {
					gdk_draw_rgb_image (canvas->layout.bin_window,
							    canvas->pixmap_gc,
							    (draw_x1 - DISPLAY_X1 (canvas)
							     + canvas->zoom_xofs),
							    (draw_y1 - DISPLAY_Y1 (canvas)
							     + canvas->zoom_yofs),
							    width, height,
							    GDK_RGB_DITHER_NONE,
							    buf.buf,
							    IMAGE_WIDTH_AA * 3);
				}
				canvas->draw_xofs = draw_x1;
				canvas->draw_yofs = draw_y1;
				g_free (buf.buf);

			} else {
				pixmap = gdk_pixmap_new (canvas->layout.bin_window, width, height,
							 gtk_widget_get_visual (widget)->depth);

				gdk_gc_set_foreground (canvas->pixmap_gc,
						       &widget->style->bg[GTK_STATE_NORMAL]);
				gdk_draw_rectangle (pixmap,
						    canvas->pixmap_gc,
						    TRUE,
						    0, 0,
						    width, height);

				/* Draw the items that intersect the area */

				if (canvas->root->object.flags & GTK_CANVAS_ITEM_VISIBLE)
					(* GTK_CANVAS_ITEM_CLASS (
						canvas->root->object.klass)->draw) (
							canvas->root, pixmap,
							draw_x1, draw_y1,
							width, height);
#if 0
				gdk_draw_line (pixmap,
					       widget->style->black_gc,
					       0, 0,
					       width - 1, height - 1);
				gdk_draw_line (pixmap,
					       widget->style->black_gc,
					       width - 1, 0,
					       0, height - 1);
#endif
				/* Copy the pixmap to the window and clean up */

				gdk_draw_pixmap (canvas->layout.bin_window,
						 canvas->pixmap_gc,
						 pixmap,
						 0, 0,
						 draw_x1 - DISPLAY_X1 (canvas) + canvas->zoom_xofs,
						 draw_y1 - DISPLAY_Y1 (canvas) + canvas->zoom_yofs,
						 width, height);

				gdk_pixmap_unref (pixmap);
			}
	  	}
	}

	art_free (rects);

	canvas->need_redraw = FALSE;
	canvas->redraw_x1 = 0;
	canvas->redraw_y1 = 0;
	canvas->redraw_x2 = 0;
	canvas->redraw_y2 = 0;
}

static void
do_update (GtkCanvas *canvas)
{
	/* Cause the update if necessary */

	if (canvas->need_update) {
		double affine[6];

		art_affine_identity (affine);
		gtk_canvas_item_invoke_update (canvas->root, affine, NULL, 0);
		canvas->need_update = FALSE;
	}

	/* Pick new current item */

	while (canvas->need_repick) {
		canvas->need_repick = FALSE;
		pick_current_item (canvas, &canvas->pick_event);
	}

	/* Paint if able to */

	if (GTK_WIDGET_DRAWABLE (canvas))
		paint (canvas);
}

/* Idle handler for the canvas.  It deals with pending updates and redraws. */
static gint
idle_handler (gpointer data)
{
	GtkCanvas *canvas;

	GDK_THREADS_ENTER ();

	canvas = GTK_CANVAS (data);
	do_update (canvas);

	/* Reset idle id */
	canvas->idle_id = 0;

	GDK_THREADS_LEAVE ();

	return FALSE;
}

/* Convenience function to add an idle handler to a canvas */
static void
add_idle (GtkCanvas *canvas)
{
	if (canvas->idle_id != 0)
		return;

	canvas->idle_id = gtk_idle_add (idle_handler, canvas);
}

/**
 * gtk_canvas_root:
 * @canvas: A canvas.
 *
 * Queries the root group of a canvas.
 *
 * Return value: The root group of the specified canvas.
 **/
GtkCanvasGroup *
gtk_canvas_root (GtkCanvas *canvas)
{
	g_return_val_if_fail (canvas != NULL, NULL);
	g_return_val_if_fail (GTK_CANVAS_IS_CANVAS (canvas), NULL);

	return GTK_CANVAS_GROUP (canvas->root);
}


/**
 * gtk_canvas_set_scroll_region:
 * @canvas: A canvas.
 * @x1: Leftmost limit of the scrolling region.
 * @y1: Upper limit of the scrolling region.
 * @x2: Rightmost limit of the scrolling region.
 * @y2: Lower limit of the scrolling region.
 *
 * Sets the scrolling region of a canvas to the specified rectangle.  The canvas
 * will then be able to scroll only within this region.  The view of the canvas
 * is adjusted as appropriate to display as much of the new region as possible.
 **/
void
gtk_canvas_set_scroll_region (GtkCanvas *canvas, double x1, double y1, double x2, double y2)
{
	double wxofs, wyofs;
	int xofs, yofs;

	g_return_if_fail (canvas != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (canvas));

	/*
	 * Set the new scrolling region.  If possible, do not move the visible contents of the
	 * canvas.
	 */

	gtk_canvas_c2w (canvas,
			  DISPLAY_X1 (canvas) - canvas->zoom_xofs,
			  DISPLAY_Y1 (canvas) - canvas->zoom_yofs,
			  &wxofs, &wyofs);

	canvas->scroll_x1 = x1;
	canvas->scroll_y1 = y1;
	canvas->scroll_x2 = x2;
	canvas->scroll_y2 = y2;

	gtk_canvas_w2c (canvas, wxofs, wyofs, &xofs, &yofs);

	gtk_layout_freeze (GTK_LAYOUT (canvas));

	scroll_to (canvas, xofs, yofs);

	canvas->need_repick = TRUE;
#if 0
	/* todo: should be requesting update */
	(* GTK_CANVAS_ITEM_CLASS (canvas->root->object.klass)->update) (
		canvas->root, NULL, NULL, 0);
#endif

	gtk_layout_thaw (GTK_LAYOUT (canvas));
}


/**
 * gtk_canvas_get_scroll_region:
 * @canvas: A canvas.
 * @x1: Leftmost limit of the scrolling region (return value).
 * @y1: Upper limit of the scrolling region (return value).
 * @x2: Rightmost limit of the scrolling region (return value).
 * @y2: Lower limit of the scrolling region (return value).
 *
 * Queries the scrolling region of a canvas.
 **/
void
gtk_canvas_get_scroll_region (GtkCanvas *canvas, double *x1, double *y1, double *x2, double *y2)
{
	g_return_if_fail (canvas != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (canvas));

	if (x1)
		*x1 = canvas->scroll_x1;

	if (y1)
		*y1 = canvas->scroll_y1;

	if (x2)
		*x2 = canvas->scroll_x2;

	if (y2)
		*y2 = canvas->scroll_y2;
}

/**
 * gtk_canvas_set_pixels_per_unit:
 * @canvas: A canvas.
 * @n: The number of pixels that correspond to one canvas unit.
 *
 * Sets the zooming factor of a canvas by specifying the number of pixels that
 * correspond to one canvas unit.
 **/
void
gtk_canvas_set_pixels_per_unit (GtkCanvas *canvas, double n)
{
	double cx, cy;
	int x1, y1;
	int canvas_width, canvas_height;

	g_return_if_fail (canvas != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (canvas));
	g_return_if_fail (n > GTK_CANVAS_EPSILON);

	canvas_width = GTK_WIDGET (canvas)->allocation.width;
	canvas_height = GTK_WIDGET (canvas)->allocation.height;

	/* Re-center view */

	gtk_canvas_c2w (canvas,
			  DISPLAY_X1 (canvas) - canvas->zoom_xofs + canvas_width / 2,
			  DISPLAY_Y1 (canvas) - canvas->zoom_yofs + canvas_height / 2,
			  &cx,
			  &cy);

	canvas->pixels_per_unit = n;

	if (!(canvas->root->object.flags & GTK_CANVAS_ITEM_NEED_AFFINE)) {
		canvas->root->object.flags |= GTK_CANVAS_ITEM_NEED_AFFINE;
		gtk_canvas_request_update (canvas);
	}

	gtk_canvas_w2c (canvas,
			  cx - (canvas_width / (2.0 * n)),
			  cy - (canvas_height / (2.0 * n)),
			  &x1, &y1);

	gtk_layout_freeze (GTK_LAYOUT (canvas));

	scroll_to (canvas, x1, y1);

	canvas->need_repick = TRUE;
#ifdef OLD_XFORM
	(* GTK_CANVAS_ITEM_CLASS (canvas->root->object.klass)->update) (
		canvas->root, NULL, NULL, 0);
#else

#endif

	gtk_layout_thaw (GTK_LAYOUT (canvas));
}

/**
 * gtk_canvas_scroll_to:
 * @canvas: A canvas.
 * @cx: Horizontal scrolling offset in canvas pixel units.
 * @cy: Vertical scrolling offset in canvas pixel units.
 *
 * Makes a canvas scroll to the specified offsets, given in canvas pixel units.
 * The canvas will adjust the view so that it is not outside the scrolling
 * region.  This function is typically not used, as it is better to hook
 * scrollbars to the canvas layout's scrolling adjusments.
 **/
void
gtk_canvas_scroll_to (GtkCanvas *canvas, int cx, int cy)
{
	g_return_if_fail (canvas != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (canvas));

	scroll_to (canvas, cx, cy);
}

/**
 * gtk_canvas_get_scroll_offsets:
 * @canvas: A canvas.
 * @cx: Horizontal scrolling offset (return value).
 * @cy: Vertical scrolling offset (return value).
 *
 * Queries the scrolling offsets of a canvas.  The values are returned in canvas
 * pixel units.
 **/
void
gtk_canvas_get_scroll_offsets (GtkCanvas *canvas, int *cx, int *cy)
{
	g_return_if_fail (canvas != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (canvas));

	if (cx)
		*cx = canvas->layout.hadjustment->value;

	if (cy)
		*cy = canvas->layout.vadjustment->value;
}

/**
 * gtk_canvas_update_now:
 * @canvas: A canvas.
 *
 * Forces an immediate update and redraw of a canvas.  If the canvas does not
 * have any pending update or redraw requests, then no action is taken.  This is
 * typically only used by applications that need explicit control of when the
 * display is updated, like games.  It is not needed by normal applications.
 */
void
gtk_canvas_update_now (GtkCanvas *canvas)
{
	g_return_if_fail (canvas != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (canvas));

	if (!(canvas->need_update || canvas->need_redraw))
		return;

	remove_idle (canvas);
	do_update (canvas);
}

/**
 * gtk_canvas_get_item_at:
 * @canvas: A canvas.
 * @x: X position in world coordinates.
 * @y: Y position in world coordinates.
 *
 * Looks for the item that is under the specified position, which must be
 * specified in world coordinates.
 *
 * Return value: The sought item, or NULL if no item is at the specified
 * coordinates.
 **/
GtkCanvasItem *
gtk_canvas_get_item_at (GtkCanvas *canvas, double x, double y)
{
	GtkCanvasItem *item;
	double dist;
	int cx, cy;

	g_return_val_if_fail (canvas != NULL, NULL);
	g_return_val_if_fail (GTK_CANVAS_IS_CANVAS (canvas), NULL);

	gtk_canvas_w2c (canvas, x, y, &cx, &cy);

	dist = gtk_canvas_item_invoke_point (canvas->root, x, y, cx, cy, &item);
	if ((int) (dist * canvas->pixels_per_unit + 0.5) <= canvas->close_enough)
		return item;
	else
		return NULL;
}

/* Queues an update of the canvas */
static void
gtk_canvas_request_update (GtkCanvas *canvas)
{
	canvas->need_update = TRUE;
	add_idle (canvas);
}

/* Computes the union of two microtile arrays while clipping the result to the
 * specified rectangle.  Any of the specified utas can be NULL, in which case it
 * is taken to be an empty region.
 */
static ArtUta *
uta_union_clip (ArtUta *uta1, ArtUta *uta2, ArtIRect *clip)
{
	ArtUta *uta;
	ArtUtaBbox *utiles;
	int clip_x1, clip_y1, clip_x2, clip_y2;
	int union_x1, union_y1, union_x2, union_y2;
	int new_x1, new_y1, new_x2, new_y2;
	int x, y;
	int ofs, ofs1, ofs2;

	g_assert (clip != NULL);

	/* Compute the tile indices for the clipping rectangle */

	clip_x1 = clip->x0 >> ART_UTILE_SHIFT;
	clip_y1 = clip->y0 >> ART_UTILE_SHIFT;
	clip_x2 = (clip->x1 >> ART_UTILE_SHIFT) + 1;
	clip_y2 = (clip->y1 >> ART_UTILE_SHIFT) + 1;

	/* Get the union of the bounds of both utas */

	if (!uta1) {
		if (!uta2)
			return art_uta_new (clip_x1, clip_y1, clip_x1 + 1, clip_y1 + 1);

		union_x1 = uta2->x0;
		union_y1 = uta2->y0;
		union_x2 = uta2->x0 + uta2->width;
		union_y2 = uta2->y0 + uta2->height;
	} else {
		if (!uta2) {
			union_x1 = uta1->x0;
			union_y1 = uta1->y0;
			union_x2 = uta1->x0 + uta1->width;
			union_y2 = uta1->y0 + uta1->height;
		} else {
			union_x1 = MIN (uta1->x0, uta2->x0);
			union_y1 = MIN (uta1->y0, uta2->y0);
			union_x2 = MAX (uta1->x0 + uta1->width, uta2->x0 + uta2->width);
			union_y2 = MAX (uta1->y0 + uta1->height, uta2->y0 + uta2->height);
		}
	}

	/* Clip the union of the bounds */

	new_x1 = MAX (clip_x1, union_x1);
	new_y1 = MAX (clip_y1, union_y1);
	new_x2 = MIN (clip_x2, union_x2);
	new_y2 = MIN (clip_y2, union_y2);

	if (new_x1 >= new_x2 || new_y1 >= new_y2)
		return art_uta_new (clip_x1, clip_y1, clip_x1 + 1, clip_y1 + 1);

	/* Make the new clipped union */

	uta = art_new (ArtUta, 1);
	uta->x0 = new_x1;
	uta->y0 = new_y1;
	uta->width = new_x2 - new_x1;
	uta->height = new_y2 - new_y1;
	uta->utiles = utiles = art_new (ArtUtaBbox, uta->width * uta->height);

	ofs = 0;
	ofs1 = ofs2 = 0;

	for (y = new_y1; y < new_y2; y++) {
		if (uta1)
			ofs1 = (y - uta1->y0) * uta1->width + new_x1 - uta1->x0;

		if (uta2)
			ofs2 = (y - uta2->y0) * uta2->width + new_x1 - uta2->x0;

		for (x = new_x1; x < new_x2; x++) {
			ArtUtaBbox bb1, bb2, bb;

			if (!uta1
			    || x < uta1->x0 || y < uta1->y0
			    || x >= uta1->x0 + uta1->width || y >= uta1->y0 + uta1->height)
				bb1 = 0;
			else
				bb1 = uta1->utiles[ofs1];

			if (!uta2
			    || x < uta2->x0 || y < uta2->y0
			    || x >= uta2->x0 + uta2->width || y >= uta2->y0 + uta2->height)
				bb2 = 0;
			else
				bb2 = uta2->utiles[ofs2];

			if (bb1 == 0)
				bb = bb2;
			else if (bb2 == 0)
				bb = bb1;
			else
				bb = ART_UTA_BBOX_CONS (MIN (ART_UTA_BBOX_X0 (bb1),
							     ART_UTA_BBOX_X0 (bb2)),
							MIN (ART_UTA_BBOX_Y0 (bb1),
							     ART_UTA_BBOX_Y0 (bb2)),
							MAX (ART_UTA_BBOX_X1 (bb1),
							     ART_UTA_BBOX_X1 (bb2)),
							MAX (ART_UTA_BBOX_Y1 (bb1),
							     ART_UTA_BBOX_Y1 (bb2)));

			utiles[ofs] = bb;

			ofs++;
			ofs1++;
			ofs2++;
		}
	}

	return uta;
}

/**
 * gtk_canvas_request_redraw_uta:
 * @canvas: A canvas.
 * @uta: Microtile array that specifies the area to be redrawn.
 *
 * Informs a canvas that the specified area, given as a microtile array, needs
 * to be repainted.  To be used only by item implementations.
 **/
void
gtk_canvas_request_redraw_uta (GtkCanvas *canvas,
                                 ArtUta *uta)
{
	ArtIRect visible;

	g_return_if_fail (canvas != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (canvas));
	g_return_if_fail (uta != NULL);

	visible.x0 = DISPLAY_X1 (canvas) - canvas->zoom_xofs;
	visible.y0 = DISPLAY_Y1 (canvas) - canvas->zoom_yofs;
	visible.x1 = visible.x0 + GTK_WIDGET (canvas)->allocation.width;
	visible.y1 = visible.y0 + GTK_WIDGET (canvas)->allocation.height;

	if (canvas->need_redraw) {
		ArtUta *new_uta;

		g_assert (canvas->redraw_area != NULL);

		new_uta = uta_union_clip (canvas->redraw_area, uta, &visible);
		art_uta_free (canvas->redraw_area);
		art_uta_free (uta);
		canvas->redraw_area = new_uta;
	} else {
		ArtUta *new_uta;

		g_assert (canvas->redraw_area == NULL);

		new_uta = uta_union_clip (uta, NULL, &visible);
		art_uta_free (uta);
		canvas->redraw_area = new_uta;
		canvas->need_redraw = TRUE;
	}

	add_idle (canvas);
}


/**
 * gtk_canvas_request_redraw:
 * @canvas: A canvas.
 * @x1: Leftmost coordinate of the rectangle to be redrawn.
 * @y1: Upper coordinate of the rectangle to be redrawn.
 * @x2: Rightmost coordinate of the rectangle to be redrawn, plus 1.
 * @y2: Lower coordinate of the rectangle to be redrawn, plus 1.
 *
 * Convenience function that informs a canvas that the specified rectangle needs
 * to be repainted.  This function converts the rectangle to a microtile array
 * and feeds it to gtk_canvas_request_redraw_uta().  The rectangle includes
 * @x1 and @y1, but not @x2 and @y2.  To be used only by item implementations.
 **/
void
gtk_canvas_request_redraw (GtkCanvas *canvas, int x1, int y1, int x2, int y2)
{
	ArtUta *uta;
	ArtIRect bbox;
	ArtIRect visible;
	ArtIRect clip;

	g_return_if_fail (canvas != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (canvas));

	if (!GTK_WIDGET_DRAWABLE (canvas) || (x1 == x2) || (y1 == y2))
		return;

	bbox.x0 = x1;
	bbox.y0 = y1;
	bbox.x1 = x2;
	bbox.y1 = y2;

	visible.x0 = DISPLAY_X1 (canvas) - canvas->zoom_xofs;
	visible.y0 = DISPLAY_Y1 (canvas) - canvas->zoom_yofs;
	visible.x1 = visible.x0 + GTK_WIDGET (canvas)->allocation.width;
	visible.y1 = visible.y0 + GTK_WIDGET (canvas)->allocation.height;

	art_irect_intersect (&clip, &bbox, &visible);

	if (!art_irect_empty (&clip)) {
		uta = art_uta_from_irect (&clip);
		gtk_canvas_request_redraw_uta (canvas, uta);
	}
}


/**
 * gtk_canvas_w2c_affine:
 * @canvas: A canvas.
 * @affine: An affine transformation matrix (return value).
 *
 * Gets the affine transform that converts from world coordinates to canvas
 * pixel coordinates.
 **/
void
gtk_canvas_w2c_affine (GtkCanvas *canvas, double affine[6])
{
	double x, y;
	double zooom;

	g_return_if_fail (canvas != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (canvas));
	g_return_if_fail (affine != NULL);

	zooom = canvas->pixels_per_unit;

	affine[0] = zooom;
	affine[1] = 0;
	affine[2] = 0;
	affine[3] = zooom;
	affine[4] = -canvas->scroll_x1 * zooom;
	affine[5] = -canvas->scroll_y1 * zooom;
}

/**
 * gtk_canvas_w2c:
 * @canvas: A canvas.
 * @wx: World X coordinate.
 * @wy: World Y coordinate.
 * @cx: X pixel coordinate (return value).
 * @cy: Y pixel coordinate (return value).
 *
 * Converts world coordinates into canvas pixel coordinates.
 **/
void
gtk_canvas_w2c (GtkCanvas *canvas, double wx, double wy, int *cx, int *cy)
{
	double affine[6];
	ArtPoint w, c;

	g_return_if_fail (canvas != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (canvas));

	gtk_canvas_w2c_affine (canvas, affine);
	w.x = wx;
	w.y = wy;
	art_affine_point (&c, &w, affine);
	if (cx)
		*cx = floor (c.x + 0.5);
	if (cy)
		*cy = floor (c.y + 0.5);
}

/**
 * gtk_canvas_w2c_d:
 * @canvas: A canvas.
 * @wx: World X coordinate.
 * @wy: World Y coordinate.
 * @cx: X pixel coordinate (return value).
 * @cy: Y pixel coordinate (return value).
 *
 * Converts world coordinates into canvas pixel coordinates.  This version
 * returns coordinates in floating point coordinates, for greater precision.
 **/
void
gtk_canvas_w2c_d (GtkCanvas *canvas, double wx, double wy, double *cx, double *cy)
{
	double affine[6];
	ArtPoint w, c;

	g_return_if_fail (canvas != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (canvas));

	gtk_canvas_w2c_affine (canvas, affine);
	w.x = wx;
	w.y = wy;
	art_affine_point (&c, &w, affine);
	if (cx)
		*cx = c.x;
	if (cy)
		*cy = c.y;
}


/**
 * gtk_canvas_c2w:
 * @canvas: A canvas.
 * @cx: Canvas pixel X coordinate.
 * @cy: Canvas pixel Y coordinate.
 * @wx: X world coordinate (return value).
 * @wy: Y world coordinate (return value).
 *
 * Converts canvas pixel coordinates to world coordinates.
 **/
void
gtk_canvas_c2w (GtkCanvas *canvas, int cx, int cy, double *wx, double *wy)
{
	double affine[6], inv[6];
	ArtPoint w, c;

	g_return_if_fail (canvas != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (canvas));

	gtk_canvas_w2c_affine (canvas, affine);
	art_affine_invert (inv, affine);
	c.x = cx;
	c.y = cy;
	art_affine_point (&w, &c, inv);
	if (wx)
		*wx = w.x;
	if (wy)
		*wy = w.y;
}


/**
 * gtk_canvas_window_to_world:
 * @canvas: A canvas.
 * @winx: Window-relative X coordinate.
 * @winy: Window-relative Y coordinate.
 * @worldx: X world coordinate (return value).
 * @worldy: Y world coordinate (return value).
 *
 * Converts window-relative coordinates into world coordinates.  You can use
 * this when you need to convert mouse coordinates into world coordinates, for
 * example.
 **/
void
gtk_canvas_window_to_world (GtkCanvas *canvas, double winx, double winy,
			      double *worldx, double *worldy)
{
	g_return_if_fail (canvas != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (canvas));

	if (worldx)
		*worldx = canvas->scroll_x1 + ((winx + DISPLAY_X1 (canvas) - canvas->zoom_xofs)
					       / canvas->pixels_per_unit);

	if (worldy)
		*worldy = canvas->scroll_y1 + ((winy + DISPLAY_Y1 (canvas) - canvas->zoom_yofs)
					       / canvas->pixels_per_unit);
}


/**
 * gtk_canvas_world_to_window:
 * @canvas: A canvas.
 * @worldx: World X coordinate.
 * @worldy: World Y coordinate.
 * @winx: X window-relative coordinate.
 * @winy: Y window-relative coordinate.
 *
 * Converts world coordinates into window-relative coordinates.
 **/
void
gtk_canvas_world_to_window (GtkCanvas *canvas, double worldx, double worldy,
			      double *winx, double *winy)
{
	g_return_if_fail (canvas != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (canvas));

	if (winx)
		*winx = (canvas->pixels_per_unit)*(worldx - canvas->scroll_x1) -
		  DISPLAY_X1(canvas) + canvas->zoom_xofs;

	if (winy)
		*winy = (canvas->pixels_per_unit)*(worldy - canvas->scroll_y1) -
		  DISPLAY_Y1(canvas) + canvas->zoom_yofs;
}



/**
 * gtk_canvas_get_color:
 * @canvas: A canvas.
 * @spec: X color specification, or NULL for "transparent".
 * @color: Returns the allocated color.
 *
 * Allocates a color based on the specified X color specification.  As a
 * convenience to item implementations, it returns TRUE if the color was
 * allocated, or FALSE if the specification was NULL.  A NULL color
 * specification is considered as "transparent" by the canvas.
 *
 * Return value: TRUE if @spec is non-NULL and the color is allocated.  If @spec
 * is NULL, then returns FALSE.
 **/
int
gtk_canvas_get_color (GtkCanvas *canvas, const char *spec, GdkColor *color)
{
	gint n;

	g_return_val_if_fail (canvas != NULL, FALSE);
	g_return_val_if_fail (GTK_CANVAS_IS_CANVAS (canvas), FALSE);
	g_return_val_if_fail (color != NULL, FALSE);

	if (!spec) {
		color->pixel = 0;
		color->red = 0;
		color->green = 0;
		color->blue = 0;
		return FALSE;
	}

	gdk_color_parse (spec, color);

	color->pixel = 0;
	n = 0;
	gdk_color_context_get_pixels (canvas->cc,
				      &color->red,
				      &color->green,
				      &color->blue,
				      1,
				      &color->pixel,
				      &n);

	return TRUE;
}

/**
 * gtk_canvas_get_color_pixel:
 * @canvas: A canvas.
 * @rgba: RGBA color specification.
 *
 * Allocates a color from the RGBA value passed into this function.  The alpha
 * opacity value is discarded, since normal X colors do not support it.
 *
 * Return value: Allocated pixel value corresponding to the specified color.
 **/
gulong
gtk_canvas_get_color_pixel (GtkCanvas *canvas,
			      guint        rgba)
{
	GdkColor color;
	gint n;

	g_return_val_if_fail (GTK_CANVAS_IS_CANVAS (canvas), 0);

	color.pixel = 0;
	color.red = ((rgba & 0xff000000) >> 16) + ((rgba & 0xff000000) >> 24);
	color.green = ((rgba & 0x00ff0000) >> 8) + ((rgba & 0x00ff0000) >> 16);
	color.blue = (rgba & 0x0000ff00) + ((rgba & 0x0000ff00) >> 8);
	n = 0;
	gdk_color_context_get_pixels (canvas->cc,
				      &color.red,
				      &color.green,
				      &color.blue,
				      1,
				      &color.pixel,
				      &n);

	return color.pixel;
}


/**
 * gtk_canvas_set_stipple_origin:
 * @canvas: A canvas.
 * @gc: GC on which to set the stipple origin.
 *
 * Sets the stipple origin of the specified GC as is appropriate for the canvas,
 * so that it will be aligned with other stipple patterns used by canvas items.
 * This is typically only needed by item implementations.
 **/
void
gtk_canvas_set_stipple_origin (GtkCanvas *canvas, GdkGC *gc)
{
	g_return_if_fail (canvas != NULL);
	g_return_if_fail (GTK_CANVAS_IS_CANVAS (canvas));
	g_return_if_fail (gc != NULL);

	gdk_gc_set_ts_origin (gc, -canvas->draw_xofs, -canvas->draw_yofs);
}
