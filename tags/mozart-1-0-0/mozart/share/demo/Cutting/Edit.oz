%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
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
   Tk
   TkTools

   Configure(colors: Colors
	     fonts:  Fonts)
   

export
   page: EditPage
   
prepare
   
   NoPlates   = 5
   Quantity   = 60
   PlateWidth = 5
   Pad        = 3

define
   
   EntryColor   = Colors.entry
   
   BadColor     = Colors.bad
   GoodColor    = Colors.good
   NeutralColor = Colors.neutral
   
   GlassColor   = Colors.glass
   
   class PlateBar
      from Tk.canvas
      prop locking
	 
      feat d
      attr Pos:0
	 
      meth init(parent:P size:S)
	 lock
	    D  = S * PlateWidth
	    N  = Quantity div (S + 1)
	    XH = ~ Quantity * PlateWidth
	 in
	    Tk.canvas,tkInit(parent: P
			     width:  (Quantity - 1) * PlateWidth
			     height: D
			     bg:     ivory)
	    {For 0 N-1 1
	     proc {$ I}
		X0 = XH + I*(D+PlateWidth) + 1
		X1 = X0 + D - 1 
		Y0 = 1
		Y1 = Y0 + D - 1
	     in
		{self tk(create rectangle X0 Y0 X1 Y1
			 fill:    GlassColor
			 outline: black)}
	     end}
	    self.d = D
	 end
      end
      
      meth set(N)
	 lock
	    M = N - @Pos
	 in
	    Pos <- N
	    Tk.canvas,tk(move all M*(self.d + PlateWidth) 0)
	 end
      end
      
   end
      
   class EditPage
      from TkTools.note
      feat
	 squares
	 plateBars
	 status
	 compute
      attr
	 x:10 y:10
	 
      meth init(parent:P compute:C)
	 TkTools.note,tkInit(parent:P text:'Edit')
	 PlateFrame  = {New TkTools.textframe
			tkInit(parent: self
			       font:   Fonts.normal
			       text:   'Glass Plates')}
	 self.squares   = {Dictionary.new}
	 self.plateBars = {MakeTuple bars NoPlates}
	 
	 TicklePackPlates =
	 {ForThread 1 NoPlates 1
	  fun {$ Tcls D}
	     L = {New Tk.label
		  tkInit(parent: PlateFrame.inner
			 font:   Fonts.normal
			 text:   D#'x'#D)}
	     E = {New TkTools.numberentry
		  tkInit(parent: PlateFrame.inner
			 font:   Fonts.bold
			 min:    0
			 max:    Quantity div (D + 1)
			 width:  2
			 action: self # plate(D))}
	     {E.entry tk(configure bg:EntryColor)}
	     G = {New PlateBar init(parent:PlateFrame.inner size:D)}
	  in
	     self.plateBars.D = G
	     (grid(row:D column:1 L padx:Pad pady:Pad sticky:n) |
	      grid(row:D column:2 E padx:Pad pady:Pad sticky:n) |
	      grid(row:D column:3 G padx:Pad pady:Pad sticky:n) | Tcls)
	  end nil}
	 
	 TargetFrame = {New TkTools.textframe
			tkInit(parent: self
			       font:   Fonts.normal
			       text:  'Target Plate')}
	 
	 XL = {New Tk.label
	       tkInit(parent: TargetFrame.inner
		      font:   Fonts.normal
		      text:   'X')}
	 XE = {New TkTools.numberentry
	       tkInit(parent: TargetFrame.inner
		      min:    1
		      val:    10
		      max:    50
		      width:  2
		      font:   Fonts.bold
		      action: self # set(x))}
	 {XE.entry tk(configure bg:EntryColor)}
	 YL = {New Tk.label tkInit(parent: TargetFrame.inner
				   font:   Fonts.normal
				   text:   'Y')}
	 YE = {New TkTools.numberentry
	       tkInit(parent: TargetFrame.inner
		      min:    1
		      val:    10
		      max:    50
		      width:  2
		      font:   Fonts.bold
		      action: self # set(y))}
	 {YE.entry tk(configure bg:EntryColor)}
	 CL = {New Tk.label tkInit(parent: TargetFrame.inner
				   font:   Fonts.normal
				   text:   ''
				   width:  26
				   bg:     NeutralColor
				   relief: sunken
				   bd:     1)}
      in
	 self.status  = CL
	 self.compute = C
	 {Tk.batch {Append TicklePackPlates
		    [grid(row:1 column:1 XL padx:Pad pady:Pad sticky:n)
		     grid(row:1 column:2 XE padx:Pad pady:Pad sticky:n)
		     grid(row:1 column:3 {New Tk.canvas
					  tkInit(parent:TargetFrame.inner
						 width:10
						 height:1)} sticky:w)
		     grid(row:1 column:4 YL padx:Pad pady:Pad sticky:w)
		     grid(row:1 column:5 YE padx:Pad pady:Pad sticky:w)
		     grid(row:1 column:6 {New Tk.canvas
					  tkInit(parent:TargetFrame.inner
						 width:10
						 height:1)} sticky:w)
		     grid(row:1 column:7 CL padx:Pad pady:Pad sticky:w)
		     grid(row:1 column:1 padx:Pad pady:Pad PlateFrame)
		     grid(row:2 column:1 padx:Pad pady:Pad TargetFrame
			  sticky:ew)]}}
      end
      
      meth UpdateStatus
	 Cap     = @x * @y
	 Use     = {FoldL {Dictionary.entries self.squares}
		    fun {$ Use D#N}
		       D*D*N + Use
		    end 0}
	 Col#Txt = if Cap < Use then
		      BadColor  # 'Plate too small.'
		   else
		      GoodColor # 'Plate possibly large enough.'
		   end
      in
	 {self.status tk(configure
			 text: Txt
			 bg:   Col)}
      end
      
      meth plate(D N)
	 {self.plateBars.D set(N)}
	 {Dictionary.put   self.squares D N}
	 EditPage,UpdateStatus
      end
      
      meth set(Attr X)
	 Attr <- X
	 EditPage,UpdateStatus
      end
      
      meth getSpec($)
	 spec(x:@x y:@y squares:{Dictionary.toRecord spec self.squares})
      end

   end

end
