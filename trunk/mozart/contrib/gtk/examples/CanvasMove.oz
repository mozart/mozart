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
   GDK    at 'x-oz://system/gtk/GDK.ozf'
   GTK    at 'x-oz://system/gtk/GTK.ozf'
   Canvas at 'x-oz://system/gtk/GTKCANVAS.ozf'
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
	 %% CAUTION: At this time, the underlying objects has been destroyed.
	 %% CAUTION: This event is solely intended for oz side cleanup code.
	 %% CAUTION: If you want eager finalisation of object wrappers then
	 %% CAUTION: connect the delete event handler using a procedure
	 %% CAUTION: rather than a object method.
	 {Application.exit 0}
      end
   end

   Toplevel = {New CanvasToplevel new}
 
   %% Setup the Colors
   %% 1. Obtain the system colormap
   %% 2. Allocate the color structure with R, G, B preset
   %% 3. Try to alloc appropriate system colors,
   %%    non-writeable and with best-match
   %% 4. Use colors black and white
   Colormap = {New GDK.colormap getSystem}
   Black    = {New GDK.color new(0 0 0)}
   White    = {New GDK.color new(65535 65535 65535)}
   {Colormap allocColor(Black 0 1 _)}
   {Colormap allocColor(White 0 1 _)}

   %% Setup canvas without item support
   MyCanvas = {New Canvas.canvas new(false)}
   {MyCanvas setUsize(400 400)}
   {MyCanvas setScrollRegion(0.0 0.0 400.0 400.0)}
   %% Make Canvas child of toplevel
   {Toplevel add(MyCanvas)}
   
   %% Setup Canvas Items
   %% Create a text item (member of root group) and ignore item obj
   TextItemPars = ["x"#10.0 "y"#10.0
		   "text"#"Press Button to move canvas item below"
		   "font"#
		   "-adobe-helvetica-medium-r-normal--12-*-72-72-p-*-iso8859-1"
		   "fill_color_gdk"#Black
		   "anchor"#GTK.'ANCHOR_NORTH_WEST']
   _ = {MyCanvas newItem({MyCanvas root($)} {MyCanvas textGetType($)}
			 TextItemPars $)}

   %% Create a rectangle item
   RectItemPars = ["x1"#200.0 "y1"#60.0 "x2"#400.0 "y2"#180.0
		   "fill_color_gdk"#Black "outline_color_gdk"#White]
   RectItem = {MyCanvas newItem({MyCanvas root($)} {MyCanvas rectGetType($)}
				RectItemPars $)}

   %% Create Rectangle Item Event Handler
   local
      proc {ToggleColor Item Fill Outline}
	 {Item set("fill_color_gdk" Fill)}
	 {Item set("outline_color_gdk" Outline)}
      end
   in
      fun {MakeRectEvent Item}
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
		  NewX = X - {Cell.access ButtonX}
		  NewY = Y - {Cell.access ButtonY}
	       in
		  {Item move(NewX NewY)}
		  {Cell.assign ButtonX X}
		  {Cell.assign ButtonY Y}
	       end
	    [] _ then skip
	    end
	 end
      end
   end
   {RectItem signalConnect('event' {MakeRectEvent RectItem} _)}
      
   %% Make it all visible
   {Toplevel showAll}
end
