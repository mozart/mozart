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
   GDK GTK GTKCANVAS
define
   %% Create Toplevel window class
   class CanvasToplevel from GTK.window
      meth new
	 GTK.window, new(GTK.'WINDOW_TOPLEVEL')
	 GTK.window, setBorderWidth(10)
	 GTK.window, setTitle("Canvas Events")
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
   %% Setup canvas dimension
   {Canvas setUsize(400 400)}
   {Canvas setScrollRegion(0.0 0.0 400.0 400.0)}
   %% Make Canvas child of toplevel
   {Toplevel add(Canvas)}
   
   %% Setup Canvas Items
   %% Create a Rectangle item (member of root group)
   RectDesc = rectangle(parent               : {Canvas rootItem($)}
			x1                   : 10
			y1                   : 20
			x2                   : 380
			y2                   : 380
			fill_color_gdk       : {Canvas itemColor('#FFFFFF' $)}
			outline_color_gdk    : {Canvas itemColor('#000000' $)}
			width_pixels         : 1)
   _#RectObj = {Canvas newWrappedItem(RectDesc $)}

   %% Assign Events to Rectangle Item
   proc {ItemEvent [Event]}
      case {Label Event}
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
      [] 'GDK_SELECTION_REQUEST' then
	 {System.show 'Got SelectionRequest Event'}
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
   {RectObj signalConnect('event' ItemEvent _)}
   %% Allow RectItem to receive key events
   {RectObj grabFocus}
   
   %% Create a Line item(member of root group); ignore Item Object
   PolyDesc = line(parent          : {Canvas rootItem($)}
		   points          : [20 20 380 200 20 380]
		   fill_color_gdk  : {Canvas itemColor('#000000' $)}
		   line_style      : GDK.'LINE_SOLID'
		   width_pixels    : 2)
   _ = {Canvas newItem(PolyDesc $)}
   
   %% Make it all visible
   {Toplevel showAll}
end
