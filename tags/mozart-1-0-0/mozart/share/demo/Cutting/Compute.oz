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
   Tk TkTools Search

   Script(compile: Compile)
   Configure(colors: Colors
	     delays: Delays
	     fonts:  Fonts)
   

export
   page: ComputePage
   
define
   
   BgColor      = Colors.bg
   
   BadColor     = Colors.bad
   OkayColor    = Colors.okay
   GoodColor    = Colors.good
   NeutralColor = Colors.neutral
   
   GlassColor   = Colors.glass
   CutColor     = Colors.cut
   SketchColor  = Colors.sketch

   CanvasBd     = 10
   PlateWidth   = 15
   ButtonWidth  = 10
   PlateBd      = 3
   Pad          = 3
   CanvasWidth  = 250
   CanvasHeight = 200

   AnimDelay = Delays.wait
   CutDelay  = Delays.cut
   
   class PlateCanvas
      from Tk.canvas

      attr
	 Sol:       unit
	 BreakAnim: unit
	 
      meth init(parent:P)
      	Tk.canvas,tkInit(parent: P
			 bg:     BgColor
			 width:  CanvasWidth
			 height: CanvasHeight)
      end

      meth sketch(Spec)
	 X0 = 1 + CanvasBd
	 X1 = X0 + Spec.x*PlateWidth + 1
	 Y0 = 1 + CanvasBd
	 Y1 = Y0 + Spec.y*PlateWidth + 1
      in
	 Tk.canvas,tk(create rectangle X0 Y0 X1 Y1
		      fill:    SketchColor
		      outline: SketchColor
		      width:   0)
	 Tk.canvas,tk(configure
		      scrollregion:q(0 0
				     {Max Spec.x * PlateWidth + 2 * CanvasBd
				      CanvasWidth}
				     {Max Spec.y * PlateWidth + 2 * CanvasBd
				      CanvasHeight}))
      end
      
      meth display(S)
	 Sol <- S
	 D = S.d
	 X = S.x
	 Y = S.y
      in
	 {For 1 {Width X} 1
	  proc {$ I}
	     X0 = X.I * PlateWidth + 1 + PlateBd + CanvasBd
	     X1 = X0 + D.I*PlateWidth - 1 - PlateBd
	     Y0 = Y.I * PlateWidth + 1 + PlateBd + CanvasBd
	     Y1 = Y0 + D.I*PlateWidth - 1 - PlateBd
	  in
	     {self tk(create rectangle X0 Y0 X1 Y1
		      fill:    GlassColor
		      width:   1
		      outline: black)}
	  end}
      end

      meth Animate(CutInfo x:X y:Y)
	 if {IsFree @BreakAnim} then
	    X0#X1=X Y0#Y1=Y
	 in
	    case CutInfo
	    of nil then skip
	    [] info(cut:Cut dir:Dir L R) then
	       case Dir
	       of y then
		  Tk.canvas,tk(create line
			       Cut*PlateWidth+CanvasBd+2
			       Y0*PlateWidth+CanvasBd+2
			       Cut*PlateWidth+CanvasBd+2
			       Y1*PlateWidth+CanvasBd+2
			       tags:  animate
			       width: 1
			       fill:  CutColor)
		  {WaitOr @BreakAnim {Alarm CutDelay}}
		  PlateCanvas,Animate(L x:X0#Cut y:Y0#Y1)
		  PlateCanvas,Animate(R x:Cut#X1 y:Y0#Y1)
	       [] x then
		  Tk.canvas,tk(create line
			       X0*PlateWidth+CanvasBd+2
			       Cut*PlateWidth+CanvasBd+2
			       X1*PlateWidth+CanvasBd+2
			       Cut*PlateWidth+CanvasBd+2
			       tags:  animate
			       width: 1
			       fill:  CutColor)
		  {WaitOr @BreakAnim {Alarm CutDelay}}
		  PlateCanvas,Animate(L x:X0#X1 y:Y0#Cut)
		  PlateCanvas,Animate(R x:X0#X1 y:Cut#Y1)
	       end
	    end
	 end
      end
      
      meth animate
	 S = @Sol
      in
	 BreakAnim <- _
	 PlateCanvas,Animate(S.cuts x:0#S.dx y:0#S.dy)
	 {WaitOr @BreakAnim {Alarm AnimDelay}}
	 Tk.canvas,tk(delete animate)
      end

      meth stopAnim
	 @BreakAnim = unit
      end

      meth clear
	 Sol <- unit
	 Tk.canvas,tk(delete all)
      end
   end


   class ComputePage
      from TkTools.note

      feat
	 start stop animate plate edit status
	 
      attr
	 script:  unit
	 stopper: false
	 
      meth init(parent:P edit:E)
	 TkTools.note,tkInit(parent:P text:'Compute')
	 
	 ButtonFrame = {New Tk.frame tkInit(parent:self)}
	 
	 Start = {New Tk.button tkInit(parent: ButtonFrame
				       text:   'Start'
				       width:  ButtonWidth
				       action: self # start
				       font:   Fonts.normal)}
	 Stop  = {New Tk.button tkInit(parent: ButtonFrame
				       text:   'Stop'
				       width:  ButtonWidth
				       action: self # stop
				       state:  disabled
				       font:   Fonts.normal)}
	 Anim  = {New Tk.button tkInit(parent: ButtonFrame
				       text:   'Animate'
				       width:  ButtonWidth
				       action: self # animate
				       state:  disabled
				       font:   Fonts.normal)}
	 
	 GlassFrame = {New Tk.frame tkInit(parent:self)}
	 
	 PC = {New PlateCanvas init(parent: GlassFrame)}
	 H  = {New Tk.scrollbar      tkInit(parent:GlassFrame orient:horizontal
					    width:13)}
	 V  = {New Tk.scrollbar      tkInit(parent:GlassFrame orient:vertical
					    width:13)}
	 Status = {New Tk.label tkInit(parent: ButtonFrame
				       font:   Fonts.normal
				       text:   ''
				       width:  13
				       bg:     NeutralColor
				       relief: sunken
				       bd:     1)}
	 
      in
	 {Tk.addXScrollbar PC H}
	 {Tk.addYScrollbar PC V}
	 {Tk.batch [grid(columnconfigure GlassFrame    0 weight:1)
		    grid(rowconfigure    GlassFrame    0 weight:1)
		    grid(PC row:0 column:0 sticky:nsew)
		    grid(H row:1 column:0 sticky:we)
		    grid(V row:0 column:1 sticky:ns)
		    grid(columnconfigure self    2 weight:1)
		    grid(rowconfigure    self    1 weight:1)
		    
		    grid(row:1 column:1 padx:Pad pady:Pad Start)
		    grid(row:2 column:1 padx:Pad pady:Pad Stop)
		    grid(row:3 column:1 padx:Pad pady:Pad
			 {New Tk.canvas tkInit(parent:ButtonFrame
					       width:0 height:20)})
		    grid(row:4 column:1 padx:Pad pady:Pad Status)
		    grid(row:5 column:1 padx:Pad pady:Pad
			 {New Tk.canvas tkInit(parent:ButtonFrame
					       width:0 height:20)})
		    grid(row:6 column:1 padx:Pad pady:Pad Anim)
		    
		    grid(row:1 column:1 ButtonFrame sticky:nw)
		    grid(row:1 column:2 GlassFrame sticky:ne)
		   ]}
	 self.start   = Start
	 self.stop    = Stop
	 self.animate = Anim
	 self.plate   = PC
	 self.edit    = E
	 self.status  = Status
      end

      meth start
	 Script
	 Stopper
      in
	 lock
	    Script = @script
	    stopper <- Stopper
	    {self.start  tk(configure state:disabled)}
	    {self.stop   tk(configure state:normal)}
	    {self.status tk(configure bg:NeutralColor text:'Computing.')}
	 end
	 case {Search.one.depth Script 1 ?Stopper}
	 of nil then
	    {self.stop   tk(configure state:disabled)}
	    if @stopper==true then
	       {self.status tk(configure bg:OkayColor text:'Stopped.')}
	       {self.start  tk(configure state:normal)}
	    else
	       {self.status tk(configure bg:BadColor text:'No solution.')}
	    end
	 [] [Sol] then
	    {self.stop    tk(configure state:disabled)}
	    {self.plate   display(Sol)}
	    {self.status tk(configure bg:GoodColor text:'Okay.')}
	    {self.animate tk(configure state:normal)}
	 end
      end

      meth stop
	 lock
	    S=@stopper
	 in
	    if {Procedure.is S} then
	       stopper <- true
	       {S}
	    end
	 end
      end
      
      meth animate
	 lock
	    {self.animate tk(configure state:disabled)}
	    {self.plate animate}
	    {self.animate tk(configure state:normal)}
	 end
      end

      meth toBottom
	 {self.plate stopAnim}
	 ComputePage,stop
      end
      
      meth toTop
	 lock
	    Spec = {self.edit getSpec($)}
	 in
	    {self.stop    tk(configure state:disabled)}
	    {self.animate tk(configure state:disabled)}
	    {self.plate   clear}
	    if {Record.foldL Spec.squares Number.'+' 0}>0 then
	       script  <- {Compile Spec}
	       stopper <- false
	       {self.start   tk(configure state:normal)}
	       {self.plate   sketch(Spec)}
	       {self.status  tk(configure bg:NeutralColor text:'')}
	    else
	       {self.start   tk(configure state:disabled)}
	       {self.status  tk(configure bg:BadColor text:'What?')}
	    end
	 end
      end

   end
   
end
