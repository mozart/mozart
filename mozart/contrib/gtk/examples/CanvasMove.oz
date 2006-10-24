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
	 GTK.window, setTitle("Canvas Move")
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
   %% Set canvas dimensions
   {Canvas setUsize(400 400)}
   {Canvas setScrollRegion(0.0 0.0 400.0 400.0)}
   %% Make Canvas child of toplevel
   {Toplevel add(Canvas)}

   %% Setup the Item Colors
   Black = {Canvas itemColor('#000000' $)}
   White = {Canvas itemColor('#FFFFFF' $)}
  
   %% Create a text item (member of root group)
   RootItem = {Canvas rootItem($)}
   local
      Font = '-adobe-helvetica-medium-r-normal--12-*-72-72-p-*-iso8859-1'
   in
      TextDesc = text(parent         : RootItem
		      text           : "Press Button to move canvas item below"
		      x              : 10.0
		      y              : 10.0
		      font           : Font
		      fill_color_gdk : Black
		      anchor         : GTK.'ANCHOR_NORTH_WEST')
   end
   _ = {Canvas newItem(TextDesc $)}

   %% Create a rectangle item (member of root group)
   RectDesc = rectangle(parent            : RootItem
			x1                : 200.0
			y1                : 60.0
			x2                : 400.0
			y2                : 180.0
			fill_color_gdk    : Black
			outline_color_gdk : White)

   %% Canvas items are unwrapped by default.
   %% However, event connection requires an Oz object.
   %% The resulting object must be freed explicitly
   %% (i.e. {RectObject gtkClose}).
   RectItem#RectObject = {Canvas newWrappedItem(RectDesc $)}

   %% Create Rectangle Event Handler
   local
      proc {ToggleColor Item Fill Outline}
	 {Canvas configureItem(Item options(fill_color_gdk : Fill))}
	 {Canvas configureItem(Item options(outline_color_gdk : Outline))}
      end
   in
      fun {MakeRectangleEvent Item}
	 ButtonX = {Cell.new 0.0}
	 ButtonY = {Cell.new 0.0}
	 Pressed = {Cell.new false}
      in
	 proc {$ [Event]}
	    case Event
	    of 'GDK_BUTTON_PRESS'(button:Button x:X y:Y ...) then
	       case Button
	       of 1 then
		  {ToggleColor Item White Black}
		  {Cell.assign Pressed true}
		  {Cell.assign ButtonX X}
		  {Cell.assign ButtonY Y}
	       [] _ then skip
	       end
	    [] 'GDK_BUTTON_RELEASE'(...) then
	       {ToggleColor Item Black White}
	       {Cell.assign Pressed false}
	    [] 'GDK_MOTION_NOTIFY'(x:X y:Y ...) then
	       if {Cell.access Pressed}
	       then
		  {Canvas moveItemTo(Item {Float.toInt X} {Float.toInt Y})}
		  {Cell.assign ButtonX X}
		  {Cell.assign ButtonY Y}
	       end
	    [] _ then skip
	    end
	 end
      end
   end
   %% Items only have the generic 'event' signal which contains
   %% all appropriate events
   {RectObject signalConnect('event' {MakeRectangleEvent RectItem} _)}

   %% Make it all visible
   {Toplevel showAll}
end
