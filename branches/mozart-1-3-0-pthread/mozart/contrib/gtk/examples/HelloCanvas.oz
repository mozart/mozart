%%%
%%% Author:
%%%   Thorsten Brunklaus <bruni@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Thorsten Brunklaus, 2001
%%%
%%% Last Change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor $
import
   Application
   GDK    at 'x-oz://system/gtk/GDK.ozf'
   GTK    at 'x-oz://system/gtk/GTK.ozf'
   Canvas at 'x-oz://system/gtk/GTKCANVAS.ozf'
define
   %% Create Toplevel window class
   class CanvasToplevel from GTK.window
      meth new
	 GTK.window, new(GTK.'WINDOW_TOPLEVEL')
	 GTK.window, setBorderWidth(10)
	 GTK.window, setTitle("Hello Canvas")
	 {self signalConnect('delete_event' deleteEvent nil _)}
      end
      meth deleteEvent(Args)
	 %% Caution: At this time, the underlying GTK object
	 %% Caution: has been destroyed already
	 %% Caution: Destruction also includes all attached child objects.
	 %% Caution: This event is solely intended to do OZ side
	 %% Caution: cleanup via calling close
	 {self close}
	 {Application.exit 0}
      end
   end

   Toplevel = {New CanvasToplevel new}
 
   %% Set up the Colors
   %% 1. Obtain the system colormap
   %% 2. Allocate the color structure with R, G, B preset
   %% 3. Try to alloc appropriate system colors,
   %%    non-writeable and with best-match
   %% 4. Use color black
   Colormap = {New GDK.colormap getSystem}
   Black    = {New GDK.color new(0 0 0)}
   {Colormap allocColor(Black 0 1 _)}

   %% This will be our canvasItem
   TextItem = ["text"#"Hello, Canvas World!"
	       "x"#100.0
	       "y"#100.0
	       "font"#
	       "-adobe-helvetica-medium-r-normal--18-*-72-72-p-*-iso8859-1"
	       "fill_color_gdk"#Black]

   %% Setup canvas
   MyCanvas = {New Canvas.canvas new}
   {MyCanvas setUsize(400 400)}
   {MyCanvas setScrollRegion(0.0 0.0 400.0 400.0)}
   %% Make Canvas child of toplevel
   {Toplevel add(MyCanvas)}
   %% Create our item (member of root group); ignore item object
   _ = {MyCanvas newItem({MyCanvas root($)} {MyCanvas textGetType($)}
			 TextItem $)}

   %% Make it all visible
   {Toplevel showAll}
end
