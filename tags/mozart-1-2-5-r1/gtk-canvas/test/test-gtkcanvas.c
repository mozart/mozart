/* testgtk-canvas - program similar to testgtk which shows gtk-canvas lib functions.
 *
 * Author : Richard Hestilow <hestgray@ionet.net>
 *
 * Copyright (C) 1998 Free Software Foundation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <config.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <gdk_imlib.h>

#include "test-gtkcanvas.h"

static const gchar *authors[] = {
	"Richard Hestilow",
	"Federico Mena",
	"Eckehard Berns",
	"Havoc Pennington",
	"Miguel de Icaza",
	"Jonathan Blandford",
	NULL
};


static void
create_about (void)
{
   printf("GNOME Test Program, Copyright 1998 the Free Software Foundation\nAuthors: Richard Hestilow,  Federico Mena,  Eckehard Berns, Havoc Pennington, Miguel de Icaza, Jonathan Blandford\n");
        
}

static void
quit_test (void)
{
        gtk_main_quit ();
}



int
main (int argc, char *argv[])
{
	GtkWidget *app;
	GtkWidget *notebook;
	int i;

        gtk_init (&argc, &argv);        
	
	gtk_canvas_init();
	
	app = gtk_widget_new (gtk_window_get_type (),
			   "GtkObject::user_data", NULL,
			   "GtkWindow::type", GTK_WINDOW_TOPLEVEL,
			   "GtkWindow::title", "hello world",
			   "GtkWindow::allow_grow", TRUE,
			   "GtkWindow::allow_shrink", TRUE,
			   "GtkContainer::border_width", 10,
			   NULL);

        gtk_signal_connect(GTK_OBJECT(app), "delete_event",
				   GTK_SIGNAL_FUNC(quit_test), NULL);
				   
	/*gtk_widget_set_usize (app, 800, 600);*/
	
	notebook = create_canvas();
	
	gtk_container_add(GTK_CONTAINER(app), notebook);
	gtk_widget_show(notebook);

	gtk_widget_show (app);
	gtk_main();

	gtk_widget_destroy(app);

	return 0;
}
