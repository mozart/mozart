%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 2001
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   GDK
   GTK
   GBuilder(create: Create)
export
   Is
   FromOz
   ToOz
define
   fun {Enum Type Values Module}
      FromOzDict = {NewDictionary}
      ToOzDict = {NewDictionary}
      for A#F in Values do I in
	 I = Module.F
	 FromOzDict.A := I
	 ToOzDict.I := A
      end
      FromOz = {Dictionary.toRecord fromOz FromOzDict}
      ToOz = {Dictionary.toRecord toOz ToOzDict}
   in
      Type(is: fun {$ X} {HasFeature FromOz X} end
	   fromOz: fun {$ A} FromOz.A end
	   toOz: fun {$ X} ToOz.X end)
   end

   fun {GdkEnum Type Values}
      {Enum Type Values GDK}
   end

   fun {GtkEnum Type Values}
      {Enum Type Values GTK}
   end

   fun {Flags Type Values Module}
      Values2 = {Sort {Map Values fun {$ A#F} A#Module.F end}
		 fun {$ _#A _#B} A < B end}
   in
      Type(is:
	      fun {$ X}
		 {IsList X} andthen
		 {All X fun {$ Y} {Some Values2 fun {$ A#_} A == Y end} end}
		 %--** require all distinct (else not handled correctly)
	      end
	   fromOz:
	      fun {$ As}
		 {FoldR As
		  fun {$ A In}
		     case {Filter Values2 fun {$ A2#_} A2 == A end} of [_#I]
		     then In + I
		     end
		  end 0}
	      end
	   toOz:
	      fun {$ X} C in
		 C = {NewCell X}
		 {FoldR Values2
		  fun {$ A#I In}
		     if {Access C} >= I then {Assign C {Access C} - I} A|In
		     else In
		     end
		  end nil}
	      end)
   end

   fun {GdkFlags Type Values}
      {Flags Type Values GDK}
   end

   fun {GtkFlags Type Values}
      {Flags Type Values GTK}
   end

   fun {Process Specs}
      {List.toRecord types {Map Specs fun {$ Spec} {Label Spec}#Spec end}}
   end

   Types =
   {Process
    [int(is: IsInt
	 fromOz: fun {$ I} I end
	 toOz: fun {$ X} X end)
     float(is: IsFloat
	   fromOz: fun {$ F} F end
	   toOz: fun {$ X} X end)
     bool(is: IsBool
	  fromOz: fun {$ B} B end
	  toOz: fun {$ X} X end)
     boolean(is: IsBool
	     fromOz: fun {$ B} B end
	     toOz: fun {$ X} X end)
     string(is: IsVirtualString
	    fromOz: fun {$ V} V end
	    toOz: fun {$ X} X end)
     stringArray(is: fun {$ X}
			{IsList X} andthen {All X IsVirtualString}
		     end
		 fromOz: GTK.makeStrArr
		 toOz: GTK.getStrArr
		 destroy: GTK.freeStrArr)
     colormap(is: fun {$ _} true end   %--** GdkColormap
	      fromOz: fun {$ X} X end
	      toOz: fun {$ X} X end)
     accelGroup(is: fun {$ _} true end   %--** GTK binding does not provide access
		fromOz: fun {$ X} X end
		toOz: fun {$ X} X end)
     {GdkFlags modifierType [shift#'SHIFT_MASK'
			     'lock'#'LOCK_MASK'
			     control#'CONTROL_MASK'
			     mod1#'MOD1_MASK'
			     mod2#'MOD2_MASK'
			     mod3#'MOD3_MASK'
			     mod4#'MOD4_MASK'
			     mod5#'MOD5_MASK'
			     button1#'BUTTON1_MASK'
			     button2#'BUTTON2_MASK'
			     button3#'BUTTON3_MASK'
			     button4#'BUTTON4_MASK'
			     button5#'BUTTON5_MASK'
			     release#'RELEASE_MASK'
			     modifier#'MODIFIER_MASK']}
     {GdkEnum rgbDither [none#'RGB_DITHER_NONE'
			 normal#'RGB_DITHER_NORMAL'
			 max#'RGB_DITHER_MAX']}
     {GtkFlags attachOptions [expand#'EXPAND'
			      shrink#'SHRINK'
			      fill#'FILL']}
     {GtkFlags calendarDisplayOptions
      [showHeading#'CALENDAR_SHOW_HEADING'
       showDayNames#'CALENDAR_SHOW_DAY_NAMES'
       noMonthChange#'CALENDAR_NO_MONTH_CHANGE'
       showWeekNumbers#'CALENDAR_SHOW_WEEK_NUMBERS'
       weekStartMonday#'CALENDAR_WEEK_START_MONDAY']}
     {GtkFlags packerOptions [expand#'PACK_EXPAND'
			      fillX#'FILL_X'
			      fillY#'FILL_Y']}
     {GtkEnum anchorType [center#'ANCHOR_CENTER'
			  north#'ANCHOR_NORTH'
			  northWest#'ANCHOR_NORTH_WEST'
			  northEast#'ANCHOR_NORTH_EAST'
			  south#'ANCHOR_SOUTH'
			  southWest#'ANCHOR_SOUTH_WEST'
			  southEast#'ANCHOR_SOUTH_EAST'
			  west#'ANCHOR_WEST'
			  east#'ANCHOR_EAST'
			  n#'ANCHOR_N'
			  sw#'ANCHOR_NW'
			  se#'ANCHOR_NE'
			  s#'ANCHOR_S'
			  sw#'ANCHOR_SW'
			  se#'ANCHOR_SE'
			  w#'ANCHOR_W'
			  e#'ANCHOR_E']}
     {GtkEnum arrowType [up#'ARROW_UP'
			 down#'ARROW_DOWN'
			 left#'ARROW_LEFT'
			 right#'ARROW_RIGHT']}
     {GtkEnum buttonBoxStyle [defaultStyle#'BUTTONBOX_DEFAULT_STYLE'
			      spread#'BUTTONBOX_SPREAD'
			      edge#'BUTTONBOX_EDGE'
			      start#'BUTTONBOX_START'
			      'end'#'BUTTONBOX_END']}
     {GtkEnum cornerType [topLeft#'CORNER_TOP_LEFT'
			  bottomLeft#'CORNER_BOTTOM_LEFT'
			  topRight#'CORNER_TOP_RIGHT'
			  bottomRight#'CORNER_BOTTOM_RIGHT']}
     {GtkEnum curveType [linear#'CURVE_TYPE_LINEAR'
			 spline#'CURVE_TYPE_SPLINE'
			 free#'CURVE_TYPE_FREE']}
     {GtkEnum justification [left#'JUSTIFY_LEFT'
			     right#'JUSTIFY_RIGHT'
			     center#'JUSTIFY_CENTER'
			     fill#'JUSTIFY_FILL']}
     {GtkEnum metricType [pixels#'PIXELS'
			  inches#'INCHES'
			  centimeters#'CENTIMETERS']}
     {GtkEnum orientation [horizontal#'ORIENTATION_HORIZONTAL'
			   vertical#'ORIENTATION_VERTICAL']}
     {GtkEnum policyType [always#'POLICY_ALWAYS'
			  automatic#'POLICY_AUTOMATIC'
			  never#'POLICY_NEVER']}
     {GtkEnum positionType [left#'POS_LEFT'
			    right#'POS_RIGHT'
			    top#'POS_TOP'
			    bottom#'POS_BOTTOM']}
     {GtkEnum previewType [color#'PREVIEW_COLOR'
			   grayscale#'PREVIEW_GRAYSCALE']}
     {GtkEnum progressBarStyle [continuous#'PROGRESS_CONTINUOUS'
				discrete#'PROGRESS_DISCRETE']}
     {GtkEnum progressBarOrientation [leftToRight#'PROGRESS_LEFT_TO_RIGHT'
				      rightToLeft#'PROGRESS_RIGHT_TO_LEFT'
				      bottomToTop#'PROGRESS_BOTTOM_TO_TOP'
				      topToBottom#'PROGRESS_TOP_TO_BOTTOM']}
     {GtkEnum reliefStyle [normal#'RELIEF_NORMAL'
			   half#'RELIEF_HALF'
			   none#'RELIEF_NONE']}
     {GtkEnum resizeMode [parent#'RESIZE_PARENT'
			  queue#'RESIZE_QUEUE'
			  immediate#'RESIZE_IMMEDIATE']}
     {GtkEnum selectionMode [single#'SELECTION_SINGLE'
			     browse#'SELECTION_BROWSE'
			     multiple#'SELECTION_MULTIPLE'
			     extended#'SELECTION_EXTENDED']}
     {GtkEnum shadowType [none#'SHADOW_NONE'
			  'in'#'SHADOW_IN'
			  out#'SHADOW_OUT'
			  etchedIn#'SHADOW_ETCHED_IN'
			  etchedOut#'SHADOW_ETCHED_OUT']}
     {GtkEnum sideType [top#'SIDE_TOP'
			bottom#'SIDE_BOTTOM'
			left#'SIDE_LEFT'
			right#'SIDE_RIGHT']}
     {GtkEnum spinButtonUpdatePolicy [always#'UPDATE_ALWAYS'
				      ifValid#'UPDATE_IF_VALID']}
     {GtkEnum submenuPlacement [topBottom#'TOP_BOTTOM'
				leftRight#'LEFT_RIGHT']}
     {GtkEnum toolbarSpaceStyle [empty#'TOOLBAR_SPACE_EMPTY'
				 line#'TOOLBAR_SPACE_LINE']}
     {GtkEnum toolbarStyle [icons#'TOOLBAR_ICONS'
			    text#'TOOLBAR_TEXT'
			    both#'TOOLBAR_BOTH']}
     {GtkEnum treeViewMode [line#'TREE_VIEW_LINE'
			    item#'TREE_VIEW_ITEM']}
     {GtkEnum updateType [continuous#'UPDATE_CONTINUOUS'
			  discontinuous#'UPDATE_DISCONTINUOUS'
			  delayed#'UPDATE_DELAYED']}
     {GtkEnum windowPosition [none#'WIN_POS_NONE'
			      center#'WIN_POS_CENTER'
			      mouse#'WIN_POS_MOUSE']}
     {GtkEnum windowType [toplevel#'WINDOW_TOPLEVEL'
			  dialog#'WINDOW_DIALOG'
			  popup#'WINDOW_POPUP']}]}

   fun {Is Type X}
      case Type of option(ElementType) then
	 X == unit orelse {Is ElementType X}
      [] list(ElementType) then
	 {IsList X} andthen {All X fun {$ Y} {Is ElementType Y} end}
      [] object(_) then true   %--** GTK binding does not support type tests
      [] group(ArgumentType) then {Is object(ArgumentType) X}
      else {Types.Type.is X}
      end
   end

   fun {FromOz Type X}
      case Type of option(ElementType) then
	 case X of unit then unit
	 else {FromOz ElementType X}
	 end
      [] list(ElementType) then
	 {Map X fun {$ Y} {FromOz ElementType Y} end}
      [] object(_) then
	 if {IsObject X} then X else {Create X} end
      [] group(_) then {X group($)}
      else {Types.Type.fromOz X}
      end
   end

   fun {ToOz Type X}
      case Type of option(ElementType) then
	 case X of unit then unix
	 else {ToOz ElementType X}
	 end
      [] list(ElementType) then
	 {Map X fun {$ Y} {ToOz ElementType Y} end}
      [] object(_) then X
      [] group(_) then {Exception.raiseError gBuilder(toOz Type X)} unit
      else {Types.Type.toOz X}
      end
   end
end
