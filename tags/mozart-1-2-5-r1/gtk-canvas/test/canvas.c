#include <config.h>
#include <math.h>
#include "test-gtkcanvas.h"


GtkWidget *
create_canvas (void)
{
	GtkWidget *notebook;


	notebook = gtk_notebook_new ();
	gtk_widget_show (notebook);

	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), create_canvas_primitives (0), gtk_label_new ("Primitives"));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), create_canvas_primitives (1), gtk_label_new ("Antialias"));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), create_canvas_arrowhead (), gtk_label_new ("Arrowhead"));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), create_canvas_fifteen (), gtk_label_new ("Fifteen"));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), create_canvas_features (), gtk_label_new ("Features"));

	gtk_widget_show (notebook);
	return notebook;
}
