%%%
%%% Authors:
%%%   Christian Schulte <schulte@dfki.de>
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
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

declare

   local
      HelvFamily        = '-*-helvetica-medium-r-normal--*-'
      HelvBoldFamily    = '-*-helvetica-bold-r-normal--*-'
      CourierFamily     = '-*-courier-medium-r-normal--*-'
      CourierBoldFamily = '-*-courier-bold-r-normal--*-'
      FontMatch         = '-*-*-*-*-*-*'
      
      FontSize          = 120
      SmallSize         = 100
   in
      [HelvBold Helv Courier SmallCourierBold] =
      {Map [HelvBoldFamily    # FontSize  # FontMatch
	    HelvFamily        # FontSize  # FontMatch
	    CourierFamily     # FontSize  # FontMatch
	    CourierBoldFamily # SmallSize # FontMatch]
       VirtualString.toAtom}
   end

NoPlates   = 5
Quantity   = 60
PlateWidth = 5
GlassColor = 'SteelBlue1' % c(191 239 255) %c(142 188 143)
Pad = 2
   EntryColor = GlassColor
   BadColor  = c(238 44 44)
   OkayColor = c(255 127 0)
   NeutralColor = ivory
   GoodPercentage = 30.0
   GoodColor = c(180 238 180)
   
TextFont

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
      
class EditDialog
   from TkTools.note
   feat
      squares
      plateBars
      status
   attr
      x:10 y:10

   meth init(parent:P)
      TkTools.note,tkInit(parent:P text:'Edit')
      PlateFrame  = {New TkTools.textframe
		     tkInit(parent: self
			    font:   Helv
			    text:   'Glass Plates')}
      self.squares   = {Dictionary.new}
      self.plateBars = {MakeTuple bars NoPlates}
      
      TicklePackPlates =
      {ForThread 1 NoPlates 1
       fun {$ Tcls D}
	  L = {New Tk.label
	       tkInit(parent: PlateFrame.inner
		      font:   Helv
		      text:   D#'x'#D)}
	  E = {New TkTools.numberentry
	       tkInit(parent: PlateFrame.inner
		      font:   HelvBold
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
			    font:   Helv
			    text:  'Target Plate')}

      XL = {New Tk.label
	    tkInit(parent: TargetFrame.inner
		   font:   Helv
		   text:   'X')}
      XE = {New TkTools.numberentry
	    tkInit(parent: TargetFrame.inner
		   min:    1
		   val:    10
		   max:    50
		   width:  2
		   font:   HelvBold
		   action: self # set(x))}
      {XE.entry tk(configure bg:EntryColor)}
      YL = {New Tk.label tkInit(parent: TargetFrame.inner
				font:   Helv
				text:   'Y')}
      YE = {New TkTools.numberentry
	    tkInit(parent: TargetFrame.inner
		   min:    1
		   val:    10
		   max:    50
		   width:  2
		   font:   HelvBold
		   action: self # set(y))}
      {YE.entry tk(configure bg:EntryColor)}
      CL = {New Tk.label tkInit(parent: TargetFrame.inner
				font:   Helv
				text:   ''
				width:  26
				bg:     NeutralColor
				relief: sunken
				bd:     1)}
   in
      self.status = CL
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
      Cap     = {IntToFloat @x * @y}
      Use     = {IntToFloat {FoldL {Dictionary.entries self.squares}
			     fun {$ Use D#N}
				D*D*N + Use
			     end 0}}
      Rest    = (Cap - Use) / Cap
      Col#Txt = if Rest < 0.0 then
		   BadColor  # 'Plate too small.'
		elseif Rest < 0.3 then
		   OkayColor # 'Plate possibly to small.'
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
      EditDialog,UpdateStatus
   end

   meth set(Attr X)
      Attr <- X
      EditDialog,UpdateStatus
   end
   
   meth getSpec($)
      S=spec(x:@x y:@y squares:{Dictionary.toRecord spec self.squares})
   in
%      {Browse S}
      S
   end

end


local

   CanvasBg     = ivory
   CanvasBd     = 10
   PlateWidth   = 15
   PlateBd      = 3
   CanvasWidth  = 250
   CanvasHeight = 200
   SketchColor  = c(202 225 255)
   CutColor  = red
   CutDelay  = 400
   AnimDelay = 3 * CutDelay

   class PlateCanvas
      from Tk.canvas

      attr
	 Sol:       unit
	 BreakAnim: unit
	 
      meth init(parent:P)
      	Tk.canvas,tkInit(parent: P
			 bg:     CanvasBg
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
	 {For 1 {Width S.squares} 1
	  proc {$ I}
	     Sq = S.squares.I
	     D  = Sq.d
	     X0 = Sq.x * PlateWidth + 1 + PlateBd + CanvasBd
	     X1 = X0 + D*PlateWidth - 1 - PlateBd
	     Y0 = Sq.y * PlateWidth + 1 + PlateBd + CanvasBd
	     Y1 = Y0 + D*PlateWidth - 1 - PlateBd
	  in
	     {self tk(create rectangle X0 Y0 X1 Y1
		      fill:    GlassColor
		      width:   1
		      outline: black)}
	  end}
      end

      meth Animate(CutInfo Dir x:X y:Y)
	 if {IsFree @BreakAnim} then
	    X0#X1=X Y0#Y1=Y
	 in
	    case CutInfo
	    of nil then skip
	    [] info(cut:Cut L R) then
	       case Dir
	       of x then
		  Tk.canvas,tk(create line
			       Cut*PlateWidth+CanvasBd+2
			       Y0*PlateWidth+CanvasBd+2
			       Cut*PlateWidth+CanvasBd+2
			       Y1*PlateWidth+CanvasBd+2
			       tags:  animate
			       width: 1
			       fill:  CutColor)
		  {WaitOr @BreakAnim {Alarm CutDelay}}
		  PlateCanvas,Animate(L y x:X0#Cut y:Y0#Y1)
		  PlateCanvas,Animate(R y x:Cut#X1 y:Y0#Y1)
	       [] y then
		  Tk.canvas,tk(create line
			       X0*PlateWidth+CanvasBd+2
			       Cut*PlateWidth+CanvasBd+2
			       X1*PlateWidth+CanvasBd+2
			       Cut*PlateWidth+CanvasBd+2
			       tags:  animate
			       width: 1
			       fill:  CutColor)
		  {WaitOr @BreakAnim {Alarm CutDelay}}
		  PlateCanvas,Animate(L x x:X0#X1 y:Y0#Cut)
		  PlateCanvas,Animate(R x x:X0#X1 y:Cut#Y1)
	       end
	    end
	 end
      end
      
      meth animate
	 S = @Sol
      in
	 BreakAnim <- _
	 PlateCanvas,Animate(S.cuts x x:0#S.x y:0#S.y)
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

in
   
   class ComputeDialog
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
				       width:  8
				       action: self # start
				       font:   Helv)}
	 Stop  = {New Tk.button tkInit(parent: ButtonFrame
				       text:   'Stop'
				       width:  8
				       action: self # stop
				       state:  disabled
				       font:   Helv)}
	 Anim  = {New Tk.button tkInit(parent: ButtonFrame
				       text:   'Animate'
				       width:  8
				       action: self # animate
				       state:  disabled
				       font:   Helv)}
	 
	 GlassFrame = {New Tk.frame tkInit(parent:self)}
	 
	 PC = {New PlateCanvas init(parent: GlassFrame)}
	 H  = {New Tk.scrollbar      tkInit(parent:GlassFrame orient:horizontal
					    width:13)}
	 V  = {New Tk.scrollbar      tkInit(parent:GlassFrame orient:vertical
					    width:13)}
	 Status = {New Tk.label tkInit(parent: ButtonFrame
				       font:   Helv
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
					       width:0 height:30)})
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
	 case {Search.one.depth Script 4 ?Stopper}
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
      
      meth toTop
	 {self.plate stopAnim}
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

T  = {New Tk.toplevel tkInit(title:'Glass Plates' withdraw:true)}
B  = {New TkTools.notebook tkInit(parent:T)}
N1 = {New EditDialog       init(parent:B)}
N2 = {New ComputeDialog    init(parent:B edit:N1)}
{B add(N1)}
{B add(N2)}
{Tk.batch [pack(B)
	   update(idletasks)
	   wm(deiconify T)]}
%{Browse {G getSpec($)}}

