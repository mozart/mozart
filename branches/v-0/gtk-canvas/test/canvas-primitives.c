#include <config.h>
#include <math.h>
#include "test-gtkcanvas.h"
#include <gdk/gdkkeysyms.h>


static void
zoom_changed (GtkAdjustment *adj, gpointer data)
{
	gtk_canvas_set_pixels_per_unit (data, adj->value);
}

static gint
item_event (GtkCanvasItem *item, GdkEvent *event, gpointer data)
{
	static double x, y;
	double new_x, new_y;
	GdkCursor *fleur;
	static int dragging;
	double item_x, item_y;

	/* set item_[xy] to the event x,y position in the parent's item-relative coordinates */
	item_x = event->button.x;
	item_y = event->button.y;
	gtk_canvas_item_w2i (item->parent, &item_x, &item_y);

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			if (event->button.state & GDK_SHIFT_MASK)
				gtk_object_destroy (GTK_OBJECT (item));
			else {
				x = item_x;
				y = item_y;

				fleur = gdk_cursor_new (GDK_FLEUR);
				gtk_canvas_item_grab (item,
							GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK,
							fleur,
							event->button.time);
				gdk_cursor_destroy (fleur);
				dragging = TRUE;
			}
			break;

		case 2:
			if (event->button.state & GDK_SHIFT_MASK)
				gtk_canvas_item_lower_to_bottom (item);
			else
				gtk_canvas_item_lower (item, 1);
			break;

		case 3:
			if (event->button.state & GDK_SHIFT_MASK)
				gtk_canvas_item_raise_to_top (item);
			else
				gtk_canvas_item_raise (item, 1);
			break;

		default:
			break;
		}

		break;

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			new_x = item_x;
			new_y = item_y;

			gtk_canvas_item_move (item, new_x - x, new_y - y);
			x = new_x;
			y = new_y;
		}
		break;

	case GDK_BUTTON_RELEASE:
		gtk_canvas_item_ungrab (item, event->button.time);
		dragging = FALSE;
		break;

	default:
		break;
	}

	return FALSE;
}

static void
setup_item (GtkCanvasItem *item)
{
	gtk_signal_connect (GTK_OBJECT (item), "event",
			    (GtkSignalFunc) item_event,
			    NULL);
}

static void
setup_heading (GtkCanvasGroup *root, char *text, int pos)
{
	gtk_canvas_item_new (root,
			       gtk_canvas_text_get_type (),
			       "text", text,
			       "x", (double) ((pos % 3) * 200 + 100),
			       "y", (double) ((pos / 3) * 150 + 5),
			       "font", "-adobe-helvetica-medium-r-normal--12-*-72-72-p-*-iso8859-1",
			       "anchor", GTK_ANCHOR_N,
			       "fill_color", "black",
			       NULL);
}

static void
setup_divisions (GtkCanvasGroup *root)
{
	GtkCanvasGroup *group;
	GtkCanvasPoints *points;

	group = GTK_CANVAS_GROUP (gtk_canvas_item_new (root,
							   gtk_canvas_group_get_type (),
							   "x", 0.0,
							   "y", 0.0,
							   NULL));
	setup_item (GTK_CANVAS_ITEM (group));

	points = gtk_canvas_points_new (2);

	gtk_canvas_item_new (group,
			       gtk_canvas_rect_get_type (),
			       "x1", 0.0,
			       "y1", 0.0,
			       "x2", 600.0,
			       "y2", 450.0,
			       "outline_color", "black",
			       "width_units", 4.0,
			       NULL);

	points->coords[0] = 0.0;
	points->coords[1] = 150.0;
	points->coords[2] = 600.0;
	points->coords[3] = 150.0;
	gtk_canvas_item_new (group,
			       gtk_canvas_line_get_type (),
			       "points", points,
			       "fill_color", "black",
			       "width_units", 4.0,
			       NULL);

	points->coords[0] = 0.0;
	points->coords[1] = 300.0;
	points->coords[2] = 600.0;
	points->coords[3] = 300.0;
	gtk_canvas_item_new (group,
			       gtk_canvas_line_get_type (),
			       "points", points,
			       "fill_color", "black",
			       "width_units", 4.0,
			       NULL);

	points->coords[0] = 200.0;
	points->coords[1] = 0.0;
	points->coords[2] = 200.0;
	points->coords[3] = 450.0;
	gtk_canvas_item_new (group,
			       gtk_canvas_line_get_type (),
			       "points", points,
			       "fill_color", "black",
			       "width_units", 4.0,
			       NULL);

	points->coords[0] = 400.0;
	points->coords[1] = 0.0;
	points->coords[2] = 400.0;
	points->coords[3] = 450.0;
	gtk_canvas_item_new (group,
			       gtk_canvas_line_get_type (),
			       "points", points,
			       "fill_color", "black",
			       "width_units", 4.0,
			       NULL);

	setup_heading (group, "Rectangles", 0);
	setup_heading (group, "Ellipses", 1);
	setup_heading (group, "Texts", 2);
	setup_heading (group, "Images", 3);
	setup_heading (group, "Lines", 4);
	setup_heading (group, "Curves", 5);
	setup_heading (group, "Arcs", 6);
	setup_heading (group, "Polygons", 7);
	setup_heading (group, "Widgets", 8);
}

#define gray50_width 2
#define gray50_height 2
static char gray50_bits[] = {
  0x02, 0x01, };

static void
setup_rectangles (GtkCanvasGroup *root)
{
	GdkBitmap *stipple;

	setup_item (gtk_canvas_item_new (root,
					   gtk_canvas_rect_get_type (),
					   "x1", 20.0,
					   "y1", 30.0,
					   "x2", 70.0,
					   "y2", 60.0,
					   "outline_color", "red",
					   "width_pixels", 8,
					   NULL));

	if (root->item.canvas->aa) {
		setup_item (gtk_canvas_item_new (root,
						   gtk_canvas_rect_get_type (),
						   "x1", 90.0,
						   "y1", 40.0,
						   "x2", 180.0,
						   "y2", 100.0,
						   "fill_color_rgba", 0x3cb37180,
						   "outline_color", "black",
						   "width_units", 4.0,
						   NULL));
	} else {
		stipple = gdk_bitmap_create_from_data (NULL, gray50_bits, gray50_width, gray50_height);
		setup_item (gtk_canvas_item_new (root,
						   gtk_canvas_rect_get_type (),
						   "x1", 90.0,
						   "y1", 40.0,
						   "x2", 180.0,
						   "y2", 100.0,
						   "fill_color", "mediumseagreen",
						   "fill_stipple", stipple,
						   "outline_color", "black",
						   "width_units", 4.0,
						   NULL));
		gdk_bitmap_unref (stipple);
	}

	setup_item (gtk_canvas_item_new (root,
					   gtk_canvas_rect_get_type (),
					   "x1", 10.0,
					   "y1", 80.0,
					   "x2", 80.0,
					   "y2", 140.0,
					   "fill_color", "steelblue",
					   NULL));
}

static void
setup_ellipses (GtkCanvasGroup *root)
{
	GdkBitmap *stipple;

	setup_item (gtk_canvas_item_new (root,
					   gtk_canvas_ellipse_get_type (),
					   "x1", 220.0,
					   "y1", 30.0,
					   "x2", 270.0,
					   "y2", 60.0,
					   "outline_color", "goldenrod",
					   "width_pixels", 8,
					   NULL));

	setup_item (gtk_canvas_item_new (root,
					   gtk_canvas_ellipse_get_type (),
					   "x1", 290.0,
					   "y1", 40.0,
					   "x2", 380.0,
					   "y2", 100.0,
					   "fill_color", "wheat",
					   "outline_color", "midnightblue",
					   "width_units", 4.0,
					   NULL));

	if (root->item.canvas->aa) {
		setup_item (gtk_canvas_item_new (root,
						   gtk_canvas_ellipse_get_type (),
						   "x1", 210.0,
						   "y1", 80.0,
						   "x2", 280.0,
						   "y2", 140.0,
						   "fill_color_rgba", 0x5f9ea080,
						   "outline_color", "black",
						   "width_pixels", 0,
						   NULL));
	} else {
		stipple = gdk_bitmap_create_from_data (NULL, gray50_bits, gray50_width, gray50_height);
		setup_item (gtk_canvas_item_new (root,
						   gtk_canvas_ellipse_get_type (),
						   "x1", 210.0,
						   "y1", 80.0,
						   "x2", 280.0,
						   "y2", 140.0,
						   "fill_color", "cadetblue",
						   "fill_stipple", stipple,
						   "outline_color", "black",
						   "width_pixels", 0,
						   NULL));
		gdk_bitmap_unref (stipple);
	}
}

static GtkCanvasGroup *
make_anchor (GtkCanvasGroup *root, double x, double y)
{
	GtkCanvasGroup *group;

	group = GTK_CANVAS_GROUP (gtk_canvas_item_new (root,
							   gtk_canvas_group_get_type (),
							   "x", x,
							   "y", y,
							   NULL));
	setup_item (GTK_CANVAS_ITEM (group));

	gtk_canvas_item_new (group,
			       gtk_canvas_rect_get_type (),
			       "x1", -2.0,
			       "y1", -2.0,
			       "x2", 2.0,
			       "y2", 2.0,
			       "outline_color", "black",
			       "width_pixels", 0,
			       NULL);

	return group;
}

static void
setup_texts (GtkCanvasGroup *root)
{
	GdkBitmap *stipple;

	if (root->item.canvas->aa) {
		gtk_canvas_item_new (make_anchor (root, 420.0, 20.0),
				       gtk_canvas_text_get_type (),
				       "text", "Anchor NW",
				       "x", 0.0,
				       "y", 0.0,
				       "font", "-adobe-helvetica-bold-r-normal--24-240-75-75-p-138-iso8859-1",
				       "anchor", GTK_ANCHOR_NW,
				       "fill_color_rgba", 0x0000ff80,
				       NULL);
	} else {
		stipple = gdk_bitmap_create_from_data (NULL, gray50_bits, gray50_width, gray50_height);
		gtk_canvas_item_new (make_anchor (root, 420.0, 20.0),
				       gtk_canvas_text_get_type (),
				       "text", "Anchor NW",
				       "x", 0.0,
				       "y", 0.0,
				       "font", "-adobe-helvetica-bold-r-normal--24-240-75-75-p-138-iso8859-1",
				       "anchor", GTK_ANCHOR_NW,
				       "fill_color", "blue",
				       "fill_stipple", stipple,
				       NULL);
		gdk_bitmap_unref (stipple);
	}

	gtk_canvas_item_new (make_anchor (root, 470.0, 75.0),
			       gtk_canvas_text_get_type (),
			       "text", "Anchor center\nJustify center\nMultiline text",
			       "x", 0.0,
			       "y", 0.0,
			       "font", "-b&h-lucida-bold-r-normal-*-14-*-*-*-p-*-iso8859-1",
			       "anchor", GTK_ANCHOR_CENTER,
			       "justification", GTK_JUSTIFY_CENTER,
			       "fill_color", "firebrick",
			       NULL);

	gtk_canvas_item_new (make_anchor (root, 590.0, 140.0),
			       gtk_canvas_text_get_type (),
			       "text", "Clipped text\nClipped text\nClipped text\nClipped text\nClipped text\nClipped text",
			       "x", 0.0,
			       "y", 0.0,
			       "font", "-*-clean-medium-r-*-*-12-*-*-*-*-*-*-*",
			       "anchor", GTK_ANCHOR_SE,
			       "clip", TRUE,
			       "clip_width", 50.0,
			       "clip_height", 55.0,
			       "x_offset", 10.0,
			       "fill_color", "darkgreen",
			       NULL);
}

static void
free_imlib_image (GtkObject *object, gpointer data)
{
	gdk_imlib_destroy_image (data);
}

static void
plant_flower (GtkCanvasGroup *root, double x, double y, GtkAnchorType anchor, int aa)
{
	GdkImlibImage *im;
	GtkCanvasItem *image;

	if (aa)
		im = gtk_canvas_load_alpha ("flower.png");
	else
		im = gdk_imlib_load_image ("flower.png");

	if (im){
		image = gtk_canvas_item_new (root,
					       gtk_canvas_image_get_type (),
					       "image", im,
					       "x", x,
					       "y", y,
					       "width", (double) im->rgb_width,
					       "height", (double) im->rgb_height,
					       "anchor", anchor,
					       NULL);
		setup_item (image);
		gtk_signal_connect (GTK_OBJECT (image), "destroy",
				    (GtkSignalFunc) free_imlib_image,
				    im);
	}
}

static void
setup_images (GtkCanvasGroup *root, int aa)
{
	GdkImlibImage *im;
	GtkCanvasItem *image;

	if (aa)
		im = gtk_canvas_load_alpha ("toroid.png");
	else
		im = gdk_imlib_load_image ("toroid.png");
	if (im){
		image = gtk_canvas_item_new (root,
					       gtk_canvas_image_get_type (),
					       "image", im,
					       "x", 100.0,
					       "y", 225.0,
					       "width", (double) im->rgb_width,
					       "height", (double) im->rgb_height,
					       "anchor", GTK_ANCHOR_CENTER,
					       NULL);
		setup_item (image);
		gtk_signal_connect (GTK_OBJECT (image), "destroy",
				    (GtkSignalFunc) free_imlib_image,
				    im);
	} else
		g_warning ("Could not find the toroid.png sample file");

	plant_flower (root,  20.0, 170.0, GTK_ANCHOR_NW, aa);
	plant_flower (root, 180.0, 170.0, GTK_ANCHOR_NE, aa);
	plant_flower (root,  20.0, 280.0, GTK_ANCHOR_SW, aa);
	plant_flower (root, 180.0, 280.0, GTK_ANCHOR_SE, aa);
}

#define VERTICES 10
#define RADIUS 60.0

static void
polish_diamond (GtkCanvasGroup *root)
{
	GtkCanvasGroup *group;
	int i, j;
	double a;
	GtkCanvasPoints *points;

	group = GTK_CANVAS_GROUP (gtk_canvas_item_new (root,
							   gtk_canvas_group_get_type (),
							   "x", 270.0,
							   "y", 230.0,
							   NULL));
	setup_item (GTK_CANVAS_ITEM (group));

	points = gtk_canvas_points_new (2);

	for (i = 0; i < VERTICES; i++) {
		a = 2.0 * M_PI * i / VERTICES;
		points->coords[0] = RADIUS * cos (a);
		points->coords[1] = RADIUS * sin (a);

		for (j = i + 1; j < VERTICES; j++) {
			a = 2.0 * M_PI * j / VERTICES;
			points->coords[2] = RADIUS * cos (a);
			points->coords[3] = RADIUS * sin (a);
			gtk_canvas_item_new (group,
					       gtk_canvas_line_get_type (),
					       "points", points,
					       "fill_color", "black",
					       "width_units", 1.0,
					       "cap_style", GDK_CAP_ROUND,
					       NULL);
		}
	}

	gtk_canvas_points_free (points);
}

#define SCALE 7.0

static void
make_hilbert (GtkCanvasGroup *root)
{
	char hilbert[] = "urdrrulurulldluuruluurdrurddldrrruluurdrurddldrddlulldrdldrrurd";
	char *c;
	double *pp, *p;
	GtkCanvasPoints *points;
	GdkBitmap *stipple;

	points = gtk_canvas_points_new (strlen (hilbert) + 1);
	points->coords[0] = 340.0;
	points->coords[1] = 290.0;

	pp = points->coords;
	for (c = hilbert, p = points->coords + 2; *c; c++, p += 2, pp += 2)
		switch (*c) {
		case 'u':
			p[0] = pp[0];
			p[1] = pp[1] - SCALE;
			break;

		case 'd':
			p[0] = pp[0];
			p[1] = pp[1] + SCALE;
			break;

		case 'l':
			p[0] = pp[0] - SCALE;
			p[1] = pp[1];
			break;

		case 'r':
			p[0] = pp[0] + SCALE;
			p[1] = pp[1];
			break;
		}

	if (root->item.canvas->aa) {
		setup_item (gtk_canvas_item_new (root,
						   gtk_canvas_line_get_type (),
						   "points", points,
						   "fill_color_rgba", 0xff000080,
						   "width_units", 4.0,
						   "cap_style", GDK_CAP_PROJECTING,
						   "join_style", GDK_JOIN_MITER,
						   NULL));
	} else {
		stipple = gdk_bitmap_create_from_data (NULL, gray50_bits, gray50_width, gray50_height);
		setup_item (gtk_canvas_item_new (root,
						   gtk_canvas_line_get_type (),
						   "points", points,
						   "fill_color", "red",
						   "fill_stipple", stipple,
						   "width_units", 4.0,
						   "cap_style", GDK_CAP_PROJECTING,
						   "join_style", GDK_JOIN_MITER,
						   NULL));
		gdk_bitmap_unref (stipple);
	}

	gtk_canvas_points_free (points);
}

static void
setup_lines (GtkCanvasGroup *root)
{
	GtkCanvasPoints *points;

	polish_diamond (root);
	make_hilbert (root);

	/* Arrow tests */

	points = gtk_canvas_points_new (4);
	points->coords[0] = 340.0;
	points->coords[1] = 170.0;
	points->coords[2] = 340.0;
	points->coords[3] = 230.0;
	points->coords[4] = 390.0;
	points->coords[5] = 230.0;
	points->coords[6] = 390.0;
	points->coords[7] = 170.0;
	setup_item (gtk_canvas_item_new (root,
					   gtk_canvas_line_get_type (),
					   "points", points,
					   "fill_color", "midnightblue",
					   "width_units", 3.0,
					   "first_arrowhead", TRUE,
					   "last_arrowhead", TRUE,
					   "arrow_shape_a", 8.0,
					   "arrow_shape_b", 12.0,
					   "arrow_shape_c", 4.0,
					   NULL));
	gtk_canvas_points_free (points);

	points = gtk_canvas_points_new (2);
	points->coords[0] = 356.0;
	points->coords[1] = 180.0;
	points->coords[2] = 374.0;
	points->coords[3] = 220.0;
	setup_item (gtk_canvas_item_new (root,
					   gtk_canvas_line_get_type (),
					   "points", points,
					   "fill_color", "blue",
					   "width_pixels", 0,
					   "first_arrowhead", TRUE,
					   "last_arrowhead", TRUE,
					   "arrow_shape_a", 6.0,
					   "arrow_shape_b", 6.0,
					   "arrow_shape_c", 4.0,
					   NULL));

	points->coords[0] = 356.0;
	points->coords[1] = 220.0;
	points->coords[2] = 374.0;
	points->coords[3] = 180.0;
	setup_item (gtk_canvas_item_new (root,
					   gtk_canvas_line_get_type (),
					   "points", points,
					   "fill_color", "blue",
					   "width_pixels", 0,
					   "first_arrowhead", TRUE,
					   "last_arrowhead", TRUE,
					   "arrow_shape_a", 6.0,
					   "arrow_shape_b", 6.0,
					   "arrow_shape_c", 4.0,
					   NULL));
	gtk_canvas_points_free (points);
}

static void
setup_polygons (GtkCanvasGroup *root)
{
	GtkCanvasPoints *points;
	GdkBitmap *stipple;

	points = gtk_canvas_points_new (3);
	points->coords[0] = 210.0;
	points->coords[1] = 320.0;
	points->coords[2] = 210.0;
	points->coords[3] = 380.0;
	points->coords[4] = 260.0;
	points->coords[5] = 350.0;
	if (root->item.canvas->aa) {
		setup_item (gtk_canvas_item_new (root,
						   gtk_canvas_polygon_get_type (),
						   "points", points,
						   "fill_color_rgba", 0x0000ff80,
						   "outline_color", "black",
						   NULL));
	} else {
		stipple = gdk_bitmap_create_from_data (NULL, gray50_bits, gray50_width, gray50_height);
		setup_item (gtk_canvas_item_new (root,
						   gtk_canvas_polygon_get_type (),
						   "points", points,
						   "fill_color", "blue",
						   "fill_stipple", stipple,
						   "outline_color", "black",
						   NULL));
		gdk_bitmap_unref (stipple);
	}
	gtk_canvas_points_free (points);

	points = gtk_canvas_points_new (14);
	points->coords[0] = 270.0;
	points->coords[1] = 330.0;
	points->coords[2] = 270.0;
	points->coords[3] = 430.0;
	points->coords[4] = 390.0;
	points->coords[5] = 430.0;
	points->coords[6] = 390.0;
	points->coords[7] = 330.0;
	points->coords[8] = 310.0;
	points->coords[9] = 330.0;
	points->coords[10] = 310.0;
	points->coords[11] = 390.0;
	points->coords[12] = 350.0;
	points->coords[13] = 390.0;
	points->coords[14] = 350.0;
	points->coords[15] = 370.0;
	points->coords[16] = 330.0;
	points->coords[17] = 370.0;
	points->coords[18] = 330.0;
	points->coords[19] = 350.0;
	points->coords[20] = 370.0;
	points->coords[21] = 350.0;
	points->coords[22] = 370.0;
	points->coords[23] = 410.0;
	points->coords[24] = 290.0;
	points->coords[25] = 410.0;
	points->coords[26] = 290.0;
	points->coords[27] = 330.0;
	setup_item (gtk_canvas_item_new (root,
					   gtk_canvas_polygon_get_type (),
					   "points", points,
					   "fill_color", "tan",
					   "outline_color", "black",
					   "width_units", 3.0,
					   NULL));
	gtk_canvas_points_free (points);
}

static void
setup_widgets (GtkCanvasGroup *root)
{
	GtkWidget *w;

	w = gtk_button_new_with_label ("Hello world!");
	setup_item (gtk_canvas_item_new (root,
					   gtk_canvas_widget_get_type (),
					   "widget", w,
					   "x", 420.0,
					   "y", 330.0,
					   "width", 100.0,
					   "height", 40.0,
					   "anchor", GTK_ANCHOR_NW,
					   "size_pixels", FALSE,
					   NULL));
	gtk_widget_show (w);
}

static gint
key_press (GtkCanvas *canvas, GdkEventKey *event, gpointer data)
{
	int x, y;

	gtk_canvas_get_scroll_offsets (canvas, &x, &y);

	if (event->keyval == GDK_Up)
		gtk_canvas_scroll_to (canvas, x, y - 20);
	else if (event->keyval == GDK_Down)
		gtk_canvas_scroll_to (canvas, x, y + 20);
	else if (event->keyval == GDK_Left)
		gtk_canvas_scroll_to (canvas, x - 10, y);
	else if (event->keyval == GDK_Right)
		gtk_canvas_scroll_to (canvas, x + 10, y);

	return FALSE;
}

GtkWidget *
create_canvas_primitives (gint aa)
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *table;
	GtkWidget *w;
	GtkWidget *frame;
	GtkWidget *canvas;
	GtkAdjustment *adj;
	GtkCanvasGroup *root;

	vbox = gtk_vbox_new (FALSE, 4);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);
	gtk_widget_show (vbox);

	w = gtk_label_new ("Drag an item with button 1.  Click button 2 on an item to lower it,\n"
			   "or button 3 to raise it.  Shift+click with buttons 2 or 3 to send\n"
			   "an item to the bottom or top, respectively.");
	gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 0);
	gtk_widget_show (w);

	hbox = gtk_hbox_new (FALSE, 4);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show (hbox);

	/* Create the canvas */

	if (aa) {
		gtk_widget_push_visual (gdk_rgb_get_visual ());
		gtk_widget_push_colormap (gdk_rgb_get_cmap ());
		canvas = gtk_canvas_new_aa ();
	} else {
		gtk_widget_push_visual (gdk_imlib_get_visual ());
		gtk_widget_push_colormap (gdk_imlib_get_colormap ());
		canvas = gtk_canvas_new ();
	}

	/* Setup canvas items */

	root = gtk_canvas_root (GTK_CANVAS (canvas));

	setup_divisions (root);
	setup_rectangles (root);
	setup_ellipses (root);
	setup_texts (root);
	setup_images (root, aa);
	setup_lines (root);
	setup_polygons (root);
	setup_widgets (root);

#if 0
	{
		double affine[6];

#if 1
		art_affine_rotate (affine, 15);
#else
		art_affine_scale (affine, 1.5, 0.7);
#endif
		gtk_canvas_item_affine_relative (root, affine);
	}
#endif

	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();

	/* Zoom */

	w = gtk_label_new ("Zoom:");
	gtk_box_pack_start (GTK_BOX (hbox), w, FALSE, FALSE, 0);
	gtk_widget_show (w);

	adj = GTK_ADJUSTMENT (gtk_adjustment_new (1.00, 0.05, 5.00, 0.05, 0.50, 0.50));
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
			    (GtkSignalFunc) zoom_changed,
			    canvas);
	w = gtk_spin_button_new (adj, 0.0, 2);
	gtk_widget_set_usize (w, 50, 0);
	gtk_box_pack_start (GTK_BOX (hbox), w, FALSE, FALSE, 0);
	gtk_widget_show (w);

	/* Layout the stuff */

	table = gtk_table_new (2, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);
	gtk_table_set_col_spacings (GTK_TABLE (table), 4);
	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);
	gtk_widget_show (table);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_table_attach (GTK_TABLE (table), frame,
			  0, 1, 0, 1,
			  GTK_EXPAND | GTK_FILL | GTK_SHRINK,
			  GTK_EXPAND | GTK_FILL | GTK_SHRINK,
			  0, 0);
	gtk_widget_show (frame);

	gtk_widget_set_usize (canvas, 600, 450);
	gtk_canvas_set_scroll_region (GTK_CANVAS (canvas), 0, 0, 600, 450);
	gtk_container_add (GTK_CONTAINER (frame), canvas);
	gtk_widget_show (canvas);

	gtk_signal_connect_after (GTK_OBJECT (canvas), "key_press_event",
				  (GtkSignalFunc) key_press,
				  NULL);

	w = gtk_hscrollbar_new (GTK_LAYOUT (canvas)->hadjustment);
	gtk_table_attach (GTK_TABLE (table), w,
			  0, 1, 1, 2,
			  GTK_EXPAND | GTK_FILL | GTK_SHRINK,
			  GTK_FILL,
			  0, 0);
	gtk_widget_show (w);

	w = gtk_vscrollbar_new (GTK_LAYOUT (canvas)->vadjustment);
	gtk_table_attach (GTK_TABLE (table), w,
			  1, 2, 0, 1,
			  GTK_FILL,
			  GTK_EXPAND | GTK_FILL | GTK_SHRINK,
			  0, 0);
	gtk_widget_show (w);

	GTK_WIDGET_SET_FLAGS (canvas, GTK_CAN_FOCUS);
	gtk_widget_grab_focus (canvas);

	return vbox;
}
