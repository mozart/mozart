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
   Application(exit)
   System(show)
   GTK at 'x-oz://system/gtk/GTK.ozf'
   GTKCANVAS at 'x-oz://system/gtk/GTKCANVAS.ozf'
define
   %% Create Toplevel window class
   class CanvasToplevel from GTK.window
      meth new
	 GTK.window, new(GTK.'WINDOW_TOPLEVEL')
	 GTK.window, setBorderWidth(10)
	 GTK.window, setTitle("Hello Canvas")
	 {self signalConnect('delete-event' deleteEvent _)}
      end
      meth deleteEvent(Args)
	 {self gtkClose}
	 {Application.exit 0}
      end
   end

   Toplevel = {New CanvasToplevel new}

   %% Setup canvas
   Canvas = {New GTKCANVAS.canvas new(false)}
   {Canvas setUsize(400 400)}
   {Canvas setScrollRegion(0.0 0.0 400.0 400.0)}
   %% Make Canvas child of toplevel
   {Toplevel add(Canvas)}

   %% GtkArgs are working as follows
   %% 1. Allocate Structure with name and argument
   %% 2. Use it (ex. query)
   %% 3. Extract value from Arg
   %% 4. Release it by calling freeArg
   %% getArg Hint: if first argument is not unit, it must be a valid
   %% Gtk(Canvas) class. This is used to appropriately wrap the
   %% result pointer. Due to c side casting this cannot be done automaticly.

   %% Now query the width/height of the cancas
   ArgW = {GTK.makeArg "width" 0}
   ArgH = {GTK.makeArg "height" 0}
   %% Limited to one arg!
   {Canvas getv(1 ArgW)}
   {Canvas getv(1 ArgH)}
   TextStr = {VirtualString.toString
	      "Hi, I am a "#
	      {GTK.getArg unit ArgW}#
	      " x "#
	      {GTK.getArg unit ArgH}#
	      " Canvas!"}
   {GTK.freeArg ArgW}
   {GTK.freeArg ArgH}
   
   ItemFont = '-adobe-helvetica-medium-r-normal--18-*-72-72-p-*-iso8859-1'
   TextDesc = text(parent         : {Canvas rootItem($)}
		   text           : TextStr
		   x              : 150.0
		   y              : 100.0
		   font           : ItemFont
		   fill_color_gdk : {Canvas itemColor('#000000' $)})
	      
   %% Create our item (member of root group); ignore item object
   _ = {Canvas newItem(TextDesc $)}

   %% Make it all visible
   {Toplevel showAll}
end
