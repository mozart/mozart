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
   System(show)
   GDK    at 'x-oz://system/gtk/GDK.ozf'
   GTK    at 'x-oz://system/gtk/GTK.ozf'
   Canvas at 'x-oz://system/gtk/GTKCANVAS.ozf'
define
   %% Create Toplevel window class
   class CanvasToplevel from GTK.window
      meth new
	 GTK.window, new(GTK.wINDOW_TOPLEVEL)
	 GTK.window, setBorderWidth(10)
	 GTK.window, setTitle("Canvas Events")
      end
      meth connectEvents
	 {self signalConnect('destroy' destroyEvent _)}
      end
      meth destroyEvent(Event)
	 %% This is to allow GC
	 %% Toplevel is a container which recursively frees all its child widgets
	 {self close}
	 {Application.exit 0}
      end
   end

   Toplevel = {New CanvasToplevel new}
 
   %% Setup the Colors
   %% 1. Obtain the system colormap
   %% 2. Allocate the color structure with R, G, B preset
   %% 3. Try to alloc appropriate system colors, non-writeable and with best-match
   %% 4. Use colors black and white
   Colormap = {New GDK.colormap getSystem}
   Black    = {New GDK.color new(0 0 0)}
   White    = {New GDK.color new(65535 65535 65535)}
   {Colormap allocColor(Black 0 1 _)}
   {Colormap allocColor(White 0 1 _)}

   %% Setup canvas
   MyCanvas = {New Canvas.canvas new}
   {MyCanvas setUsize(400 400)}
   {MyCanvas setScrollRegion(0.0 0.0 400.0 400.0)}
   %% Make Canvas child of toplevel
   {Toplevel add(MyCanvas)}
   
   %% Setup Canvas Items
   %% Create a Rectangle item (member of root group)
   RectItemPars = ["x1"#10.0 "y1"#20.0 "x2"#380.0 "y2"#380.0
		   "fill_color_gdk"#White "outline_color_gdk"#Black]
   RectItem = {MyCanvas itemNew({MyCanvas root($)} {MyCanvas rectGetType($)} RectItemPars $)}

   %% Assign Events to Rectangle Item
   proc {ItemEvent Event}
      case {Label {GDK.getEvent Event}}
      of 'GDK_EXPOSE'            then {System.show 'Got Expose Event'}
      [] 'GDK_MOTION_NOTIFY'     then {System.show 'Got Motion Event'}
      [] 'GDK_BUTTON_PRESS'      then {System.show 'Got ButtonPress Event'}
      [] 'GDK_2BUTTON_PRESS'     then {System.show 'Got Button2Press Event'}
      [] 'GDK_3BUTTON_PRESS'     then {System.show 'Got Button3Press Event'}
      [] 'GDK_BUTTON_RELEASE'    then {System.show 'Got ButtonRelease Event'}
      [] 'GDK_KEY_PRESS'         then {System.show 'Got KeyPress Event'}
      [] 'GDK_KEY_RELEASE'       then {System.show 'Got KeyRelease Event'}
      [] 'GDK_ENTER_NOTIFY'      then {System.show 'Got Enter Event'}
      [] 'GDK_LEAVE_NOTIFY'      then {System.show 'Got Leave Event'}
      [] 'GDK_FOCUS_CHANGE'      then {System.show 'Got Focus Event'}
      [] 'GDK_CONFIGURE'         then {System.show 'Got Configure Event'}
      [] 'GDK_NO_EXPOSE'         then {System.show 'Got NoExpose Event'}
      [] 'GDK_NOTHING'           then {System.show 'Got Nothing Event'}
      [] 'GDK_DELETE'            then {System.show 'Got Delete Event'}
      [] 'GDK_DESTROY'           then {System.show 'Got Destroy Event'}
      [] 'GDK_MAP'               then {System.show 'Got Map Event'}
      [] 'GDK_UNMAP'             then {System.show 'Got Unmap Event'}
      [] 'GDK_PROPERTY_NOTIFY'   then {System.show 'Got Property Event'}
      [] 'GDK_SELECTION_CLEAR'   then {System.show 'Got SelecitonClear Event'}
      [] 'GDK_SELECTION_REQUEST' then {System.show 'Got SelectionRequest Event'}
      [] 'GDK_SELECTION_NOTIFY'  then {System.show 'Got SelectionNotify Event'}
      [] 'GDK_PROXIMITY_IN'      then {System.show 'Got ProximityIn Event'}
      [] 'GDK_PROXIMITY_OUT'     then {System.show 'Got ProximityOut Event'}
      [] 'GDK_DRAG_ENTER'        then {System.show 'Got DragEnter Event'}
      [] 'GDK_DRAG_LEAVE'        then {System.show 'Got DragLeave Event'}
      [] 'GDK_DRAG_MOTION'       then {System.show 'Got DrawMotion Event'}
      [] 'GDK_DRAG_STATUS'       then {System.show 'Got DragStatus Event'}
      [] 'GDK_DROP_START'        then {System.show 'Got DropStart Event'}
      [] 'GDK_DROP_FINISHED'     then {System.show 'Got DropFinished Event'}
      [] 'GDK_CLIENT_EVENT'      then {System.show 'Got Client Event'}
      [] 'UNSUPPORTED'           then {System.show 'Got UNSUPPORTED Event'}
      [] _                       then {System.show 'Got Strange Event'}
      end
   end
   {RectItem signalConnect('event' ItemEvent _)}
   
   %% Create a Polygon item(member of root group); ignore Item Object
   PolyItemPars =["points"#[20#20 380#200 20#380]
		  "fill_color_gdk"#Black
		  "width_pixels"#2]
   _ = {MyCanvas itemNew({MyCanvas root($)} {MyCanvas lineGetType($)} PolyItemPars $)}
   
   %% Make it all visible
   {Toplevel showAll}
end
