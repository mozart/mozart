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
   OS
   System(show)
   GTK at 'x-oz://system/gtk/GTK.ozf'
   GTKCANVAS at 'x-oz://system/gtk/GTKCANVAS.ozf'
define
   %% Some Global Variables
   PieceSize = 50
   Font      = "-adobe-helvetica-bold-r-normal--24-240-75-75-p-138-iso8859-1"
      
   fun {IsEmpty ItemArr I}
      case {Dictionary.get ItemArr I}
      of none then true
      [] _    then false
      end
   end

   fun {ToHex C}
      case C
      of 0 then "0"
      [] 1 then "1"
      [] 2 then "2"
      [] 3 then "3"
      [] 4 then "4"
      [] 5 then "5"
      [] 6 then "6"
      [] 7 then "7"
      [] 8 then "8"
      [] 9 then "9"
      [] 10 then "A"
      [] 11 then "B"
      [] 12 then "C"
      [] 13 then "D"
      [] 14 then "E"
      [] 15 then "F"
      [] _ then raise error end
      end
   end
   
   fun {MakeHex C}
      {ToHex (C div 16)}#{ToHex (C mod 16)}
   end
   
   class MyCanvas from GTKCANVAS.canvas
      attr
	 itemArr %% Item Dictionary
	 posArr  %% Position Dictionary
	 black   %% Black Color
	 white   %% White Color
      meth new
	 XDim = (PieceSize * 4 + 1)
	 YDim = (PieceSize * 4 + 1)
      in
	 GTKCANVAS.canvas, new(false)
	 GTKCANVAS.canvas, setUsize(XDim YDim)
	 GTKCANVAS.canvas,
	 setScrollRegion(0.0 0.0
			 {Int.toFloat XDim} {Int.toFloat YDim})
	 @black = {self itemColor('#000000' $)}
	 @white = {self itemColor('#FFFFFF' $)}
      end
      meth getPieceColor(Piece $)
	 Y      = Piece div 4
	 X      = Piece mod 4
	 R      = ((4 - X) * 255) div 4
	 G      = ((4 - Y) * 255) div 4
	 B      = 128
	 ColStr = {VirtualString.toAtom
		   "#"#{MakeHex R}#{MakeHex G}#{MakeHex B}}
      in
	 {self itemColor(ColStr $)}
      end
      meth checkVictory(I $)
	 if I < 15
	 then {Dictionary.get @posArr I} == I andthen
	    MyCanvas, checkVictory((I + 1) $)
	 else {System.show 'You win!'} true
	 end
      end
      meth checkMove(X Y $)
	 ItemArr = @itemArr
	 Pos1    = ((Y - 1) * 4 + X)
	 Pos2    = ((Y + 1) * 4 + X)
	 Pos3    = (Y * 4 + X - 1)
	 Pos4    = (Y * 4 + X + 1)
      in
	 if (Y > 0)     andthen {IsEmpty ItemArr Pos1}
	 then 0.0#(~1.0)#X#(Y-1)#true
	 elseif (Y < 3) andthen {IsEmpty ItemArr Pos2}
	 then 0.0#1.0#X#(Y+1)#true
	 elseif (X > 0) andthen {IsEmpty ItemArr Pos3}
	 then (~1.0)#0.0#(X-1)#Y#true
	 elseif (X < 3) andthen {IsEmpty ItemArr Pos4}
	 then 1.0#0.0#(X+1)#Y#true
	 else false
	 end
      end
      meth fillBoard(I Root)
	 ItemArr = @itemArr
	 PosArr  = @posArr
      in
	 if I < 15
	 then
	    X              = I mod 4
	    Y              = I div 4
	    X1             = {Int.toFloat (X * PieceSize)}
	    Y1             = {Int.toFloat (Y * PieceSize)}
	    Group#GroupObj = {self newWrappedItem(group(parent : Root
							x      : X1
							y      : Y1) $)}
	    _     = {self
		     newItem(
			rectangle(parent             : Group
				  x1                 : 0
				  y1                 : 0
				  x2                 : PieceSize
				  y2                 : PieceSize
				  fill_color_gdk     : {self getPieceColor(I $)}
				  outline_color_gdk  : @black
				  width_pixels       : 0) $)}
	    Text = {self newItem(text(parent         : Group
				      text           : {Int.toString (I + 1)}
				      x              : 25.0
				      y              : 25.0
				      anchor         : GTK.'ANCHOR_CENTER'
				      font           : Font
				      fill_color_gdk : @black) $)}
	    proc {PieceEvent [Event]}
	       case {Label Event}
	       of 'GDK_ENTER_NOTIFY' then
		  {self configureItem(Text options(fill_color_gdk : @white))}
	       [] 'GDK_LEAVE_NOTIFY' then
		  {self configureItem(Text options(fill_color_gdk : @black))}
	       [] 'GDK_BUTTON_PRESS' then
		  Pos = {Dictionary.get PosArr I}
		  X   = Pos mod 4
		  Y   = Pos div 4
	       in
		  case MyCanvas, checkMove(X Y $)
		  of DX#DY#NX#NY#true then
		     NewPos = NY * 4 + NX
		     MX     = DX * {Int.toFloat PieceSize}
		     MY     = DY * {Int.toFloat PieceSize}
		  in
		     {Dictionary.put PosArr I NewPos}
		     {Dictionary.put ItemArr Pos none}
		     {Dictionary.put ItemArr NewPos some(Group)}
		     {GroupObj move(MX MY)}
		     MyCanvas, checkVictory(0 _)
		  [] _ then skip
		  end
	       [] _ then skip
	       end
	    end
	 in
	    {Dictionary.put ItemArr I some(GroupObj)}
	    {Dictionary.put PosArr I I}
	    {GroupObj signalConnect('event' PieceEvent _)}
	    MyCanvas, fillBoard((I + 1) Root)
	 else
	    {Dictionary.put ItemArr I none}
	    {Dictionary.put PosArr I I}
	 end
      end
      meth createBoard($)
	 @itemArr = {Dictionary.new}
	 @posArr  = {Dictionary.new}
	 MyCanvas, fillBoard(0 {self rootItem($)})
	 @itemArr#@posArr
      end
   end

   local
      InitMoves = 256

      fun {MakeMove Pos}
	 Dir = {OS.rand} mod 4
      in
	 if     (Dir == 0) andthen (Pos > 3)          then 0#(~1)
	 elseif (Dir == 1) andthen (Pos < 12)         then 0#1
	 elseif (Dir == 2) andthen ((Pos mod 4) \= 0) then (~1)#0
	 elseif (Dir == 3) andthen ((Pos mod 4) \= 3) then 1#0
	 else {MakeMove Pos}
	 end
      end
      
      fun {TranslateIndex PosArr Pos I}
	 if I =< 15
	 then
	    if {Dictionary.get PosArr I} == Pos
	    then I else {TranslateIndex PosArr Pos (I + 1)} end
	 else raise translation_error end
	 end
      end

      fun {ValOf Item}
	 case Item
	 of some(V) then V
	 end
      end

      fun {FindSpot ItemArr I}
	 case {Dictionary.get ItemArr I}
	 of none      then I
	 [] some(...) then {FindSpot ItemArr (I + 1)}
	 end
      end
      
      proc {MoveSpot Canvas Board I Pos}
	 case Board
	 of ItemArr#PosArr then
	    if I < InitMoves
	    then
	       X#Y         = {MakeMove Pos}
	       DX          = {Int.toFloat (PieceSize * ~X)}
	       DY          = {Int.toFloat (PieceSize * ~Y)}
	       PosIndex    = {TranslateIndex PosArr Pos 0}
	       OldPos      = (Pos + Y * 4 + X)
	       OldPosIndex = {TranslateIndex PosArr OldPos 0}
	       Item        = {Dictionary.get ItemArr OldPos}
	    in
	       {Dictionary.put ItemArr Pos Item}
	       {Dictionary.put ItemArr OldPos none}
	       {Dictionary.put PosArr PosIndex OldPos}
	       {Dictionary.put PosArr OldPosIndex Pos}
	       {{ValOf Item} move(DX DY)}
	       {Canvas updateNow}
	       {MoveSpot Canvas Board (I + 1) OldPos} 
	    end
	 end
      end
   in
      fun {ScrambleEvent Canvas Board}
	 proc {$ _}
	    case Board
	    of ItemArr#_ then
	       {OS.srand 0}
	       {MoveSpot Canvas Board 0 {FindSpot ItemArr 0}}
	    end
	 end
      end
   end
   
   class Fifteen from GTK.vBox
      meth new
	 Alignment = {New GTK.alignment new(0.5 0.5 0.0 0.0)}
	 Frame     = {New GTK.frame new(unit)}
	 Canvas    = {New MyCanvas new}
	 Button    = {New GTK.button newWithLabel("Scramble")}
	 Board     = {Canvas createBoard($)}
      in
	 GTK.vBox, new(false 4)
	 {self setBorderWidth(4)}
	 {self show}
	 {self packStart(Alignment true true 0)}
	 {Alignment show}
	 {Frame setShadowType(GTK.'SHADOW_IN')}
	 {Alignment add(Frame)}
	 {Frame show}
	 {Frame add(Canvas)}
	 {Canvas show}
	 {self packStart(Button false false 0)}
	 {Button signalConnect('clicked' {ScrambleEvent Canvas Board} _)}
      end
   end
   
   class NoteToplevel from GTK.window
      meth new(Scramble)
	 Notebook = {New GTK.notebook new}
      in
	 GTK.window, new(GTK.'WINDOW_TOPLEVEL')
	 GTK.window, setTitle("Canvas Demo")
	 {self signalConnect('delete-event' deleteEvent _)}
	 GTK.window, add(Notebook)
	 {Notebook setShowTabs(true)}
	 {Notebook appendPage(Scramble {New GTK.label new("Fifteen")})}
      end
      meth deleteEvent(Args)
	 {self gtkClose}
	 {Application.exit 0}
      end
   end

   %% Create Demo
   Toplevel = {New NoteToplevel new({New Fifteen new})}
   %% Make it all visible
   {Toplevel showAll}
end
