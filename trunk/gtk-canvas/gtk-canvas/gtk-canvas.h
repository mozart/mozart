/* GtkCanvas widget - Tk-like canvas widget for Gnome
 *
 * GtkCanvas is basically a port of the Tk toolkit's most excellent canvas
 * widget.  Tk is copyrighted by the Regents of the University of California,
 * Sun Microsystems, and other parties.
 *
 * Copyright (C) 1998 The Free Software Foundation
 *
 * Authors: Federico Mena <federico@nuclecu.unam.mx>
 *          Raph Levien <raph@gimp.org>
 */

#ifndef GTK_CANVAS_H_INTERNAL
#define GTK_CANVAS_H_INTERNAL

#include <gtk-canvas/gtk-canvas-defs.h>
#include <gtk/gtklayout.h>
#include <stdarg.h>
#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_rect.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_uta.h>
#include <libart_lgpl/art_affine.h>

BEGIN_GTK_CANVAS_DECLS


/* "Small" value used by canvas stuff */
#define GTK_CANVAS_EPSILON 1e-10


/* Macros for building colors that fit in a 32-bit integer.  The values are in
 * [0, 255].
 */

#define GTK_CANVAS_COLOR(r, g, b) ((((int) (r) & 0xff) << 24)	\
				     | (((int) (g) & 0xff) << 16)	\
				     | (((int) (b) & 0xff) << 8)	\
				     | 0xff)

#define GTK_CANVAS_COLOR_A(r, g, b, a) ((((int) (r) & 0xff) << 24)	\
					  | (((int) (g) & 0xff) << 16)	\
					  | (((int) (b) & 0xff) << 8)	\
					  | ((int) (a) & 0xff))


typedef struct _GtkCanvas           GtkCanvas;
typedef struct _GtkCanvasClass      GtkCanvasClass;
typedef struct _GtkCanvasItem       GtkCanvasItem;
typedef struct _GtkCanvasItemClass  GtkCanvasItemClass;
typedef struct _GtkCanvasGroup      GtkCanvasGroup;
typedef struct _GtkCanvasGroupClass GtkCanvasGroupClass;


/* GtkCanvasItem - base item class for canvas items
 *
 * All canvas items are derived from GtkCanvasItem.  The only information a
 * GtkCanvasItem contains is its parent canvas, its parent canvas item group,
 * its bounding box in world coordinates, and its current affine transformation.
 *
 * Items inside a canvas are organized in a tree of GtkCanvasItemGroup nodes
 * and GtkCanvasItem leaves.  Each canvas has a single root group, which can
 * be obtained with the gtk_canvas_get_root() function.
 *
 * The abstract GtkCanvasItem class does not have any configurable or
 * queryable attributes.
 */

/* Object flags for items */
enum {
	GTK_CANVAS_ITEM_REALIZED      = 1 << 4,
	GTK_CANVAS_ITEM_MAPPED        = 1 << 5,
	GTK_CANVAS_ITEM_ALWAYS_REDRAW = 1 << 6,
	GTK_CANVAS_ITEM_VISIBLE       = 1 << 7,
	GTK_CANVAS_ITEM_NEED_UPDATE	= 1 << 8,
	GTK_CANVAS_ITEM_NEED_AFFINE	= 1 << 9,
	GTK_CANVAS_ITEM_NEED_CLIP	= 1 << 10,
	GTK_CANVAS_ITEM_NEED_VIS	= 1 << 11,
	GTK_CANVAS_ITEM_AFFINE_FULL	= 1 << 12
};

/* Update flags for items */
enum {
	GTK_CANVAS_UPDATE_REQUESTED  = 1 << 0,
	GTK_CANVAS_UPDATE_AFFINE     = 1 << 1,
	GTK_CANVAS_UPDATE_CLIP       = 1 << 2,
	GTK_CANVAS_UPDATE_VISIBILITY = 1 << 3,
	GTK_CANVAS_UPDATE_IS_VISIBLE = 1 << 4		/* Deprecated.  FIXME: remove this */
};

/* Data for rendering in antialiased mode */
typedef struct {
	/* 24-bit RGB buffer for rendering */
	guchar *buf;

	/* Rowstride for the buffer */
	int buf_rowstride;

	/* Rectangle describing the rendering area */
	ArtIRect rect;

	/* Background color, given as 0xrrggbb */
	guint32 bg_color;

	/* Invariant: at least one of the following flags is true. */

	/* Set when the render rectangle area is the solid color bg_color */
	unsigned int is_bg : 1;

	/* Set when the render rectangle area is represented by the buf */
	unsigned int is_buf : 1;
} GtkCanvasBuf;


#define GTK_CANVAS_TYPE_CANVAS_ITEM            (gtk_canvas_item_get_type ())
#define GTK_CANVAS_ITEM(obj)            (GTK_CHECK_CAST ((obj), GTK_CANVAS_TYPE_CANVAS_ITEM, GtkCanvasItem))
#define GTK_CANVAS_ITEM_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GTK_CANVAS_TYPE_CANVAS_ITEM, GtkCanvasItemClass))
#define GTK_CANVAS_IS_CANVAS_ITEM(obj)         (GTK_CHECK_TYPE ((obj), GTK_CANVAS_TYPE_CANVAS_ITEM))
#define GTK_CANVAS_IS_CANVAS_ITEM_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_CANVAS_TYPE_CANVAS_ITEM))


struct _GtkCanvasItem {
	GtkObject object;

	/* Parent canvas for this item */
	GtkCanvas *canvas;

	/* Parent canvas group for this item (a GtkCanvasGroup) */
	GtkCanvasItem *parent;

	/* Bounding box for this item (in world coordinates) */
	double x1, y1, x2, y2;

	/* If NULL, assumed to be the identity tranform.  If flags does not have
	 * AFFINE_FULL, then a two-element array containing a translation.  If
	 * flags contains AFFINE_FULL, a six-element array containing an affine
	 * transformation.
	 */
	double *xform;
};

struct _GtkCanvasItemClass {
	GtkObjectClass parent_class;

	/* Tell the item to update itself.  The flags are from the update flags
	 * defined above.  The item should update its internal state from its
	 * queued state, and recompute and request its repaint area.  The
	 * affine, if used, is a pointer to a 6-element array of doubles.  The
	 * update method also recomputes the bounding box of the item.
	 */
	void (* update) (GtkCanvasItem *item, double *affine, ArtSVP *clip_path, int flags);

	/* Realize an item -- create GCs, etc. */
	void (* realize) (GtkCanvasItem *item);

	/* Unrealize an item */
	void (* unrealize) (GtkCanvasItem *item);

	/* Map an item - normally only need by items with their own GdkWindows */
	void (* map) (GtkCanvasItem *item);

	/* Unmap an item */
	void (* unmap) (GtkCanvasItem *item);

	/* Return the microtile coverage of the item */
	ArtUta *(* coverage) (GtkCanvasItem *item);

	/* Draw an item of this type.  (x, y) are the upper-left canvas pixel
	 * coordinates of the drawable, a temporary pixmap, where things get
	 * drawn.  (width, height) are the dimensions of the drawable.
	 */
	void (* draw) (GtkCanvasItem *item, GdkDrawable *drawable,
		       int x, int y, int width, int height);

	/* Render the item over the buffer given.  The buf data structure
	 * contains both a pointer to a packed 24-bit RGB array, and the
	 * coordinates.  This method is only used for libart-based canvases.
	 *
	 * TODO: figure out where clip paths fit into the rendering framework.
	 */
	void (* render) (GtkCanvasItem *item, GtkCanvasBuf *buf);

	/* Calculate the distance from an item to the specified point.  It also
         * returns a canvas item which is the item itself in the case of the
         * object being an actual leaf item, or a child in case of the object
         * being a canvas group.  (cx, cy) are the canvas pixel coordinates that
         * correspond to the item-relative coordinates (x, y).
	 */
	double (* point) (GtkCanvasItem *item, double x, double y, int cx, int cy,
			  GtkCanvasItem **actual_item);

	/* Deprecated.  FIXME: remove this */
	void (* translate) (GtkCanvasItem *item, double dx, double dy);

	/* Fetch the item's bounding box (need not be exactly tight).  This
	 * should be in item-relative coordinates.
	 */
	void (* bounds) (GtkCanvasItem *item, double *x1, double *y1, double *x2, double *y2);

	/* Signal: an event ocurred for an item of this type.  The (x, y)
	 * coordinates are in the canvas world coordinate system.
	 */
	gint (* event) (GtkCanvasItem *item, GdkEvent *event);
};


/* Standard Gtk function */
GtkType gtk_canvas_item_get_type (void);

/* Create a canvas item using the standard Gtk argument mechanism.  The item is
 * automatically inserted at the top of the specified canvas group.  The last
 * argument must be a NULL pointer.
 */
GtkCanvasItem *gtk_canvas_item_new (GtkCanvasGroup *parent, GtkType type,
					const gchar *first_arg_name, ...);

/* Same as above, with parsed args */
GtkCanvasItem *gtk_canvas_item_newv (GtkCanvasGroup *parent, GtkType type,
					 guint nargs, GtkArg *args);

/* Constructors for use in derived classes and language wrappers */
void gtk_canvas_item_construct (GtkCanvasItem *item, GtkCanvasGroup *parent,
				  const gchar *first_arg_name, va_list args);

void gtk_canvas_item_constructv (GtkCanvasItem *item, GtkCanvasGroup *parent,
				   guint nargs, GtkArg *args);

/* Configure an item using the standard Gtk argument mechanism.  The last
 * argument must be a NULL pointer.
 */
void gtk_canvas_item_set (GtkCanvasItem *item, const gchar *first_arg_name, ...);

/* Same as above, with parsed args */
void gtk_canvas_item_setv (GtkCanvasItem *item, guint nargs, GtkArg *args);

/* Used only for language wrappers and the like */
void gtk_canvas_item_set_valist (GtkCanvasItem *item,
				   const gchar *first_arg_name, va_list args);

/* Move an item by the specified amount */
void gtk_canvas_item_move (GtkCanvasItem *item, double dx, double dy);

/* Apply a relative affine transformation to the item. */
void gtk_canvas_item_affine_relative (GtkCanvasItem *item, const double affine[6]);

/* Apply an absolute affine transformation to the item. */
void gtk_canvas_item_affine_absolute (GtkCanvasItem *item, const double affine[6]);

/* Scale an item about a point by the specified factors */
void gtk_canvas_item_scale (GtkCanvasItem *item,
			      double x, double y,
			      double scale_x, double scale_y);

/* Rotate an item about a point by the specified number of degrees */
void gtk_canvas_item_rotate (GtkCanvasItem *item, double x, double y, double angle);

/* Raise an item in the z-order of its parent group by the specified number of
 * positions.
 */
void gtk_canvas_item_raise (GtkCanvasItem *item, int positions);

/* Lower an item in the z-order of its parent group by the specified number of
 * positions.
 */
void gtk_canvas_item_lower (GtkCanvasItem *item, int positions);

/* Raise an item to the top of its parent group's z-order. */
void gtk_canvas_item_raise_to_top (GtkCanvasItem *item);

/* Lower an item to the bottom of its parent group's z-order */
void gtk_canvas_item_lower_to_bottom (GtkCanvasItem *item);

/* Show an item (make it visible).  If the item is already shown, it has no
 * effect.
 */
void gtk_canvas_item_show (GtkCanvasItem *item);

/* Hide an item (make it invisible).  If the item is already invisible, it has
 * no effect.
 */
void gtk_canvas_item_hide (GtkCanvasItem *item);

/* Grab the mouse for the specified item.  Only the events in event_mask will be
 * reported.  If cursor is non-NULL, it will be used during the duration of the
 * grab.  Time is a proper X event time parameter.  Returns the same values as
 * XGrabPointer().
 */
int gtk_canvas_item_grab (GtkCanvasItem *item, unsigned int event_mask,
			    GdkCursor *cursor, guint32 etime);

/* Ungrabs the mouse -- the specified item must be the same that was passed to
 * gtk_canvas_item_grab().  Time is a proper X event time parameter.
 */
void gtk_canvas_item_ungrab (GtkCanvasItem *item, guint32 etime);

/* These functions convert from a coordinate system to another.  "w" is world
 * coordinates and "i" is item coordinates.
 */
void gtk_canvas_item_w2i (GtkCanvasItem *item, double *x, double *y);
void gtk_canvas_item_i2w (GtkCanvasItem *item, double *x, double *y);

/* Gets the affine transform that converts from item-relative coordinates to
 * world coordinates.
 */
void gtk_canvas_item_i2w_affine (GtkCanvasItem *item, double affine[6]);

/* Gets the affine transform that converts from item-relative coordinates to
 * canvas pixel coordinates.
 */
void gtk_canvas_item_i2c_affine (GtkCanvasItem *item, double affine[6]);

/* Remove the item from its parent group and make the new group its parent.  The
 * item will be put on top of all the items in the new group.  The item's
 * coordinates relative to its new parent to *not* change -- this means that the
 * item could potentially move on the screen.
 * 
 * The item and the group must be in the same canvas.  An item cannot be
 * reparented to a group that is the item itself or that is an inferior of the
 * item.
 */
void gtk_canvas_item_reparent (GtkCanvasItem *item, GtkCanvasGroup *new_group);

/* Used to send all of the keystroke events to a specific item as well as
 * GDK_FOCUS_CHANGE events.
 */
void gtk_canvas_item_grab_focus (GtkCanvasItem *item);

/* Fetch the bounding box of the item.  The bounding box may not be exactly
 * tight, but the canvas items will do the best they can.  The returned bounding
 * box is in the coordinate system of the item's parent.
 */
void gtk_canvas_item_get_bounds (GtkCanvasItem *item,
				   double *x1, double *y1, double *x2, double *y2);

/* Request that the update method eventually get called.  This should be used
 * only by item implementations.
 */
void gtk_canvas_item_request_update (GtkCanvasItem *item);


/* GtkCanvasGroup - a group of canvas items
 *
 * A group is a node in the hierarchical tree of groups/items inside a canvas.
 * Groups serve to give a logical structure to the items.
 *
 * Consider a circuit editor application that uses the canvas for its schematic
 * display.  Hierarchically, there would be canvas groups that contain all the
 * components needed for an "adder", for example -- this includes some logic
 * gates as well as wires.  You can move stuff around in a convenient way by
 * doing a gtk_canvas_item_move() of the hierarchical groups -- to move an
 * adder, simply move the group that represents the adder.
 *
 * The following arguments are available:
 *
 * name		type		read/write	description
 * --------------------------------------------------------------------------------
 * x		double		RW		X coordinate of group's origin
 * y		double		RW		Y coordinate of group's origin
 */


#define GTK_CANVAS_TYPE_CANVAS_GROUP            (gtk_canvas_group_get_type ())
#define GTK_CANVAS_GROUP(obj)            (GTK_CHECK_CAST ((obj), GTK_CANVAS_TYPE_CANVAS_GROUP, GtkCanvasGroup))
#define GTK_CANVAS_GROUP_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GTK_CANVAS_TYPE_CANVAS_GROUP, GtkCanvasGroupClass))
#define GTK_CANVAS_IS_CANVAS_GROUP(obj)         (GTK_CHECK_TYPE ((obj), GTK_CANVAS_TYPE_CANVAS_GROUP))
#define GTK_CANVAS_IS_CANVAS_GROUP_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_CANVAS_TYPE_CANVAS_GROUP))


struct _GtkCanvasGroup {
	GtkCanvasItem item;

	/* Children of the group */
	GList *item_list;
	GList *item_list_end;

	/* The position of the group has been subsumed into the xform of all items */
#ifdef OLD_XFORM
	double xpos, ypos;	/* Point that defines the group's origin */
#endif
};

struct _GtkCanvasGroupClass {
	GtkCanvasItemClass parent_class;
};


/* Standard Gtk function */
GtkType gtk_canvas_group_get_type (void);

/* Deprecated.  FIXME: remove this */
void gtk_canvas_group_child_bounds (GtkCanvasGroup *group, GtkCanvasItem *item);


/*** GtkCanvas ***/


#define GTK_CANVAS_TYPE_CANVAS            (gtk_canvas_get_type ())
#define GTK_CANVAS(obj)            (GTK_CHECK_CAST ((obj), GTK_CANVAS_TYPE_CANVAS, GtkCanvas))
#define GTK_CANVAS_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GTK_CANVAS_TYPE_CANVAS, GtkCanvasClass))
#define GTK_CANVAS_IS_CANVAS(obj)         (GTK_CHECK_TYPE ((obj), GTK_CANVAS_TYPE_CANVAS))
#define GTK_CANVAS_IS_CANVAS_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_CANVAS_TYPE_CANVAS))


struct _GtkCanvas {
	GtkLayout layout;

	/* Idle handler ID */
	guint idle_id;

	/* Root canvas group */
	GtkCanvasItem *root;

	/* Signal handler ID for destruction of the root item */
	guint root_destroy_id;

	/* Scrolling region */
	double scroll_x1, scroll_y1;
	double scroll_x2, scroll_y2;

	/* Scaling factor to be used for display */
	double pixels_per_unit;

	/* Area that is being redrawn.  Contains (x1, y1) but not (x2, y2).
	 * Specified in canvas pixel coordinates.
	 */
	int redraw_x1, redraw_y1;
	int redraw_x2, redraw_y2;

	/* Area that needs redrawing, stored as a microtile array */
	ArtUta *redraw_area;

	/* Offsets of the temprary drawing pixmap */
	int draw_xofs, draw_yofs;

	/* Internal pixel offsets when zoomed out */
	int zoom_xofs, zoom_yofs;

	/* Last known modifier state, for deferred repick when a button is down */
	int state;

	/* The item containing the mouse pointer, or NULL if none */
	GtkCanvasItem *current_item;

	/* Item that is about to become current (used to track deletions and such) */
	GtkCanvasItem *new_current_item;

	/* Item that holds a pointer grab, or NULL if none */
	GtkCanvasItem *grabbed_item;

	/* Event mask specified when grabbing an item */
	guint grabbed_event_mask;

	/* If non-NULL, the currently focused item */
	GtkCanvasItem *focused_item;

	/* Event on which selection of current item is based */
	GdkEvent pick_event;

	/* Tolerance distance for picking items */
	int close_enough;

	/* Color context used for color allocation */
	GdkColorContext *cc;

	/* GC for temporary draw pixmap */
	GdkGC *pixmap_gc;

	/* Whether items need update at next idle loop iteration */
	unsigned int need_update : 1;

	/* Whether the canvas needs redrawing at the next idle loop iteration */
	unsigned int need_redraw : 1;

	/* Whether current item will be repicked at next idle loop iteration */
	unsigned int need_repick : 1;

	/* For use by internal pick_current_item() function */
	unsigned int left_grabbed_item : 1;

	/* For use by internal pick_current_item() function */
	unsigned int in_repick : 1;

	/* Whether the canvas is in antialiased mode or not */
	unsigned int aa : 1;
};

struct _GtkCanvasClass {
	GtkLayoutClass parent_class;
};


/* Standard Gtk function */
GtkType gtk_canvas_get_type (void);

/* Creates a new canvas.  You should check that the canvas is created with the
 * proper visual and colormap.  Any visual will do unless you intend to insert
 * gdk_imlib images into it, in which case you should use the gdk_imlib visual.
 *
 * You should call gtk_canvas_set_scroll_region() soon after calling this
 * function to set the desired scrolling limits for the canvas.
 */
GtkWidget *gtk_canvas_new (void);

/* Creates a new antialiased empty canvas.  You should push the GdkRgb colormap
 * and visual for this.
 */
#ifndef GTK_CANVAS_EXCLUDE_EXPERIMENTAL
GtkWidget *gtk_canvas_new_aa (void);
#endif

/* Returns the root canvas item group of the canvas */
GtkCanvasGroup *gtk_canvas_root (GtkCanvas *canvas);

/* Sets the limits of the scrolling region, in world coordinates */
void gtk_canvas_set_scroll_region (GtkCanvas *canvas,
				     double x1, double y1, double x2, double y2);

/* Gets the limits of the scrolling region, in world coordinates */
void gtk_canvas_get_scroll_region (GtkCanvas *canvas,
				     double *x1, double *y1, double *x2, double *y2);

/* Sets the number of pixels that correspond to one unit in world coordinates */
void gtk_canvas_set_pixels_per_unit (GtkCanvas *canvas, double n);

/* Scrolls the canvas to the specified offsets, given in canvas pixel coordinates */
void gtk_canvas_scroll_to (GtkCanvas *canvas, int cx, int cy);

/* Returns the scroll offsets of the canvas in canvas pixel coordinates.  You
 * can specify NULL for any of the values, in which case that value will not be
 * queried.
 */
void gtk_canvas_get_scroll_offsets (GtkCanvas *canvas, int *cx, int *cy);

/* Requests that the canvas be repainted immediately instead of in the idle
 * loop.
 */
void gtk_canvas_update_now (GtkCanvas *canvas);

/* Returns the item that is at the specified position in world coordinates, or
 * NULL if no item is there.
 */
GtkCanvasItem *gtk_canvas_get_item_at (GtkCanvas *canvas, double x, double y);

/* For use only by item type implementations. Request that the canvas eventually
 * redraw the specified region. The region is specified as a microtile
 * array. This function takes over responsibility for freeing the uta argument.
 */
void gtk_canvas_request_redraw_uta (GtkCanvas *canvas, ArtUta *uta);

/* For use only by item type implementations.  Request that the canvas
 * eventually redraw the specified region, specified in canvas pixel
 * coordinates.  The region contains (x1, y1) but not (x2, y2).
 */
void gtk_canvas_request_redraw (GtkCanvas *canvas, int x1, int y1, int x2, int y2);

/* Gets the affine transform that converts world coordinates into canvas pixel
 * coordinates.
 */
void gtk_canvas_w2c_affine (GtkCanvas *canvas, double affine[6]);

/* These functions convert from a coordinate system to another.  "w" is world
 * coordinates, "c" is canvas pixel coordinates (pixel coordinates that are
 * (0,0) for the upper-left scrolling limit and something else for the
 * lower-left scrolling limit).
 */
void gtk_canvas_w2c (GtkCanvas *canvas, double wx, double wy, int *cx, int *cy);
void gtk_canvas_w2c_d (GtkCanvas *canvas, double wx, double wy, double *cx, double *cy);
void gtk_canvas_c2w (GtkCanvas *canvas, int cx, int cy, double *wx, double *wy);

/* This function takes in coordinates relative to the GTK_LAYOUT
 * (canvas)->bin_window and converts them to world coordinates.
 */
void gtk_canvas_window_to_world (GtkCanvas *canvas,
				   double winx, double winy, double *worldx, double *worldy);

/* This is the inverse of gtk_canvas_window_to_world() */
void gtk_canvas_world_to_window (GtkCanvas *canvas,
				   double worldx, double worldy, double *winx, double *winy);

/* Takes a string specification for a color and allocates it into the specified
 * GdkColor.  If the string is null, then it returns FALSE. Otherwise, it
 * returns TRUE.
 */
int gtk_canvas_get_color (GtkCanvas *canvas, const char *spec, GdkColor *color);

/* Allocates a color from the RGB value passed into this function. */
gulong gtk_canvas_get_color_pixel (GtkCanvas *canvas,
				     guint        rgba);
     

/* Sets the stipple origin of the specified gc so that it will be aligned with
 * all the stipples used in the specified canvas.  This is intended for use only
 * by canvas item implementations.
 */
void gtk_canvas_set_stipple_origin (GtkCanvas *canvas, GdkGC *gc);


END_GTK_CANVAS_DECLS

#endif
