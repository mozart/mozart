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

local
   %% Import generic item constructors
   NewItem        = GtkCanvasNative.itemNew
   NewPolygon     = GtkCanvasNative.polygonNew
   NewImage       = GtkCanvasNative.imageNew
   %% Import fast item constructors
   NewGroup       = GtkCanvasNative.groupNew
   NewText        = GtkCanvasNative.textNew
   NewWidget      = GtkCanvasNative.widgetNew
   NewRectangle   = GtkCanvasNative.rectangleNew
   NewLine        = GtkCanvasNative.lineNew
   NewFastPolygon = GtkCanvasNative.fastPolygonNew
   %% Import item configuration
   ItemSet        = GtkCanvasNative.itemSet
   WrapItem       = GOZCore.wrapItem
   UnwrapItem     = GOZCore.unwrapItem
   NewColor       = GtkCanvasNative.colorNew
   %% Item position configuration
   ItemPosition   = GtkCanvasNative.itemPosition
   ItemMoveTo     = GtkCanvasNative.itemMoveTo
   ItemRaise      = GtkCanvasNative.itemRaise
   ItemLower      = GtkCanvasNative.itemLower
   %% Item destruction
   ItemDestroy    = GtkCanvasNative.itemDestroy
   %% Inspector Tools Import
   InspectorNewText  = GtkCanvasNative.inspectorTextNew
   InspectorNewImage = GtkCanvasNative.inspectorImageNew
   %% Item construction
   fun {Construct Item Arity Args}
      case Arity
      of parent|Ar then {Construct Item Ar Args}
      [] points|Ar then {Construct Item Ar Args}
      [] Feat|Ar then
	 {ItemSet Item Feat Args.Feat}
	 {Construct Item Ar Args}
      [] nil then Item
      end
   end

   %% Item configuration
   proc {Configure Item Arity Args}
      case Arity
      of Feat|Ar then {ItemSet Item Feat Args.Feat} {Configure Item Ar Args}
      [] nil then skip
      end
   end

   %% Item Color Handling
   local
      ColorDict = {Dictionary.new}
   in
      fun {MakeColor Str}
	 case {Dictionary.condGet ColorDict Str nil}
	 of nil then
	    Color = {NewColor Str}
	 in
	    {Dictionary.put ColorDict Str Color} Color
	 [] Color then Color
	 end
      end
   end
in
   class OzCanvasBase from GTK.layout
      %% Visual Stuff 
      meth pushVisual
	 {GtkCanvasNative.pushVisual}
      end
      meth popVisual
	 {GtkCanvasNative.popVisual}
      end
      %% Item creation/maintenance
      meth rootItem($)
	 {UnwrapItem {self root($)}}
      end
      meth newItem(Desc $)
	 case Desc
	 of group(parent: G) then
	    {NewGroup G}
	 [] group(parent: G ...) then
	    {Construct {NewGroup G} {Arity Desc} Desc}
	 [] text(parent: G text: S x: X y: Y anchor: A
		 font_gdk: F fill_color_gdk: C) then
	    {NewText G S X Y A F C}
	 [] text(parent: G ...) then
	    {Construct {NewItem G 0} {Arity Desc} Desc}
	 [] widget(parent: G widget: I x: X y: Y
		   width: W height: H anchor: A) then
	    {NewWidget G I X Y W H A}
	 [] widget(parent: G ...) then
	    {Construct {NewItem G 2} {Arity Desc} Desc}
	 [] rectangle(parent: G
		      x1: X1 y1:Y1 x2: X2 y2: Y2
		      fill_color_gdk: FC outline_color_gdk: OC
		      width_pixels: W) then
	    {NewRectangle G X1 Y1 X2 Y2 FC OC W 1}
	 [] rectangle(parent: G ...) then
	    {Construct {NewItem G 3} {Arity Desc} Desc}
	 [] ellipse(parent: G points: P
		    x1: X1 y1:Y1 x2: X2 y2: Y2
		    fill_color_gdk: FC outline_color_gdk_: OC
		    width_pixels: W) then
	    {NewRectangle G P X1 Y1 X2 Y2 FC OC W 0}
	 [] ellipse(parent: G ...) then
	    {Construct {NewItem G 5} {Arity Desc} Desc}
	 [] line(parent: G points: P fill_color_gdk: C
		 line_style: S width_pixels: W) then
	    {NewLine G P C S W}
	 [] line(parent: G points: P ...) then
	    {Construct {NewPolygon G 4 P} {Arity Desc}  Desc}
	 [] polygon(parent: G points: P
		    fill_color_gdk: FC outline_color_gdk: OC
		    width_pixels: W) then
	    {NewFastPolygon G P FC OC W}
	 [] polygon(parent: G points: P ...) then
	    {Construct {NewPolygon G 6 P} {Arity Desc} Desc}
	 [] image(parent: P image: I x: X y: Y width: W height: H anchor:A) then
	    {NewImage P {UnwrapItem I} X Y W H A}
	 else raise 'OzCanvasBase::newItem: illegal item description'#Desc end
	 end
      end
      meth newWrappedItem(Args $)
	 Item = {self newItem(Args $)}
      in
	 Item#{WrapItem Item}
      end
      meth unwrapItem(Item $)
	 {UnwrapItem Item}
      end
      meth configureItem(Item Args)
	 {Configure Item {Arity Args} Args}
      end
      meth itemGetPosition(Item X Y)
	 {ItemPosition Item X Y}
      end
      meth moveItemTo(Item X Y)
	 {ItemMoveTo Item X Y}
      end
      meth raiseItem(Item Level)
	 {ItemRaise Item Level} %% 0 == raiseToTop
      end
      meth lowerItem(Item Level)
	 {ItemLower Item Level} %% 0 == lowerToBottom
      end
      meth destroyItem(Item)
	 {ItemDestroy Item}
      end
      meth itemColor(Color $)
	 {MakeColor Color}
      end
      meth inspectorTools($)
	 tools(text   : InspectorNewText
	       image  : InspectorNewImage
	       unwrap : UnwrapItem)
      end
   end
end
