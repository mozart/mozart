/*
 *
 * GtkCanvas initialization function
 *
 * Copyright 2000 Li-Cheng (Andy) Tai.  This is free software; you may 
 * distribute or modify it under the terms of the GNU Lesser General Public 
 * License, Versio 2.1 or (at your opinion) any later version.
 *
*/
  
#include "gtk-canvas-types.h"
#include "gtk-canvas-init.h"

static int gtk_canvas_initalized = 0;


void gtk_canvas_init()
{
   if (gtk_canvas_initalized)
      return;
   
   gdk_imlib_init();      
   gtk_canvas_type_init();
   gtk_canvas_initalized = 1;
}
