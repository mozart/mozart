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
   GTK GTKCANVAS
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

   %% Setup canvas without image support
   Canvas = {New GTKCANVAS.canvas new(false)}
   %% Set Canvas dimension
   {Canvas setUsize(400 400)}
   {Canvas setScrollRegion(0.0 0.0 400.0 400.0)}
   
   %% This will be our CanvasItem (member of root group)
   local
      Font = '-adobe-helvetica-medium-r-normal--18-*-72-72-p-*-iso8859-1'
   in
      TextDesc = text(parent         : {Canvas rootItem($)}
		      text           : "Hallo, schöne Canvas Welt!"
		      x              : 100.0
		      y              : 100.0
		      anchor         : GTK.'ANCHOR_NW'
		      font           : Font
		      fill_color_gdk : {Canvas itemColor('#FF0000' $)})
   end
   
   %% Make Canvas child of toplevel
   {Toplevel add(Canvas)}
   %% Create our item and ignore item object
   _ = {Canvas newItem(TextDesc $)}

   %% Make it all visible
   {Toplevel showAll}
end
