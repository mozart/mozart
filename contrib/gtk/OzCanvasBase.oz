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
   proc {AssignArgs Item Args}
      case Args
      of (Par#ParVal)|Ar then {Item set(Par ParVal)} {AssignArgs Item Ar}
      [] nil             then skip
      end
   end

   proc {ConfigureArgs Item Args}
      case Args
      of (Par#RawVal)|Ar then
         Val = if Par == 'points'
               then {GtkCanvasNative.allocateCanvasPoints RawVal}
               else RawVal
               end
      in
         {Item set(Par Val)}
         {ConfigureArgs Item Ar}
      [] nil then skip
      end
   end

   LineType    = {GtkCanvasNative.gtkCanvasLineGetType}
   PolygonType = {GtkCanvasNative.gtkCanvasPolygonGetType}
   ImageType   = {GtkCanvasNative.gtkCanvasImageGetType}

   NewPolygon   = GtkCanvasNative.newPolygonItem
   NewText      = GtkCanvasNative.newTextItem
   NewImage     = GtkCanvasNative.newImageItem
   NewTag       = GtkCanvasNative.newTagItem
   NewSimpleTag = GtkCanvasNative.newSimpleTagItem

   ImageError = 'Canvas: image args must match [image x y width height anchor]'
   LineError  = 'Canvas: line args must match [points ...]'
   PolyError  = 'Canvas: polygon args must match [points ...]'
in
   class OzCanvasBase from GTK.layout
      meth newItem(Group Type Args $)
         case Type
         of !LineType then
            case Args
            of ('points'#Val)|Ar then
               Item = {P2O CanvasItem {NewPolygon {O2P Group} LineType Val}}
            in
               {AssignArgs Item Ar} Item
            else raise LineError end
            end
         [] !PolygonType then
            case Args
            of ('points'#Val)|Ar then
               Item = {P2O CanvasItem {NewPolygon {O2P Group} PolygonType Val}}
            in
               {AssignArgs Item Ar} Item
            else raise PolyError end
            end
         [] !ImageType then
            case Args
            of ['image'#Image
                'x'#X 'y'#Y 'width'#Width 'height'#Height 'anchor'#Anchor] then
               {P2O CanvasItem
                {NewImage {O2P Group} {O2P Image} X Y Width Height Anchor}}
            else raise ImageError end
            end
         else
            Item = {New CanvasItem new(Group Type unit {GOZCore.null})}
         in
            {AssignArgs Item Args} Item
         end
      end
      meth newTextItem(Group Text X Y Anchor Font Color $)
         {P2O CanvasItem
          {NewText {O2P Group} Text X Y Anchor {O2P Font} {O2P Color}}}
      end
      meth newImageItem(Group Image X Y Width Height Anchor $)
         {P2O CanvasItem
          {NewImage {O2P Group} {O2P Image} X Y Width Height Anchor}}
      end
      meth newTagItem(Group X Y $)
         {P2O CanvasItem {NewTag {O2P Group} X Y}}
      end
      meth newSimpleTagItem(Group $)
         {P2O CanvasItem {NewSimpleTag {O2P Group}}}
      end
      meth configureItem(Item Args)
         {ConfigureArgs Item Args}
      end
      meth pushVisual
         {GtkCanvasNative.pushVisual}
      end
      meth popVisual
         {GtkCanvasNative.popVisual}
      end
   end
end
