%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   fun {TclGetConf T Opt}
      l(lindex(l(T conf '-'#Opt) 4))
   end

   ActiveColor     = white
   Width           = 240
   LeftWidth       = 60
   HalfWidth       = Width div 2
   Border          = 5
   Height          = 80

   FontFamily      = '-*-helvetica-medium-r-normal--*-'
   FontMatch       = '-*-*-*-*-*-*'
   TickFont        = FontFamily#100#FontMatch

   local
      Log10 = {Log 10.0}

      fun {Grow N L}
	 case N=<L then L else {Grow N 10.0*L} end
      end

      fun {Shrink N L}
	 L2 = L  / 2.0   L4 = L2 / 2.0
      in
	 case N=<L4 then L4
	 elsecase N=<L2 then L2
	 else L
	 end
      end
   in
      fun {GetLimit N}
	 case N=<5.0 then 5.0
	 else
	    {Floor
	     {Shrink N
	     {Grow N
	      {IntToFloat
	       {Pow 10 {FloatToInt {Floor {Log N} / Log10}}}}}}}
	 end
      end
   end

in

   class Load
      from Tk.canvas
      feat
	 BothTag
	 TextTag
	 CoverTag
	 Dimension
	 Colors
      attr
	 LeftTag:   Unit
	 RightTag:  Unit
	 LeftMaxY:  0.0
	 RightMaxY: 0.0
	 PrevYs:    nil
	 CurX:      0
	 CurScale:  1.0
	 CurLimit:  0.0
      
      meth init(parent:P maxy:Y dim:Dim colors:Cs)
	 Limit = {GetLimit Y}
      in
	 <<Tk.canvas tkInit(parent:             P
			    width:              Width+LeftWidth+1
			    height:             Height+1+2*Border
			    highlightthickness: 0
			    xscrollincrement:   1
			    yscrollincrement:   1)>>
	 <<Tk.canvas tk(yview scroll ~Height-Border units)>>
	 <<Tk.canvas tk(xview scroll ~LeftWidth     units)>>
	 LeftTag      <- {New Tk.canvasTag tkInit(parent: self)}
	 RightTag     <- {New Tk.canvasTag tkInit(parent: self)}
	 self.BothTag   =  {New Tk.canvasTag tkInit(parent: self)} 
	 self.CoverTag  =  {New Tk.canvasTag tkInit(parent: self)} 
	 self.TextTag   =  {New Tk.canvasTag tkInit(parent: self)}
	 self.Dimension = Dim # ' '
	 self.Colors    = Cs
	 CurScale  <- {IntToFloat Height} / Limit
	 CurLimit  <- Limit
	 LeftMaxY  <- Y
	 RightMaxY <- 0.0
	 <<Tk.canvas tk(crea rectangle 0 0 Width ~Height
			fill:ActiveColor)>>
	 <<Load DrawTicks(5 ~ Height div 5)>>
	 <<Tk.canvas tk(crea rectangle
			~LeftWidth-4 Border ~1 ~Height - Border
			outline: {TclGetConf self bg}
			fill:    {TclGetConf self bg}
			tags:    self.CoverTag)>>
	 <<Tk.canvas tk(crea rectangle 0 0 Width ~Height)>>
	 <<Load DrawLabel(5 ~ Height div 5 Limit / 5.0)>>
      end

      meth DrawTicks(N D)
	 <<Load tk(crea line 0 D*N Width D*N)>>
	 case N>0 then <<Load DrawTicks(N-1 D)>> else true end
      end
      
      meth DrawLabel(N D Y)
	 <<Load tk(crea text 0 D*N
		   font: TickFont
		   text: N*{FloatToInt Y}#' '#self.Dimension 
		   anchor: e
		   tags:   self.TextTag)>>
	 <<Load tk(raise self.TextTag)>>
	 case N>0 then <<Load DrawLabel(N-1 D Y)>> else true end
      end
      
      meth DisplayLoads(Y1s Y2s X1 X2 Cs T)
	 case Y1s of nil then true
	 [] Y1|Y1r then !Y2s=Y2|Y2r !Cs=C|Cr S=@CurScale in
	    <<Tk.canvas tk(crea polygon
			   X1 0 X1 ~S*Y1 X2 ~S*Y2 X2 0
			   fill: C
			   tags: q(T self.BothTag))>>
	    <<Load tk(lower self.BothTag self.CoverTag)>>
	    <<Load DisplayLoads(Y1r Y2r X1 X2 Cr T)>>
	 end
      end

      meth ReScale(NewLimit)
	 NewScale = {IntToFloat Height} / NewLimit
      in
	 <<Tk.canvas tk(scale self.BothTag 0 0 1 NewScale / @CurScale)>>
	 CurScale <- NewScale
	 {self.TextTag tk(delete)}
	 <<Load DrawLabel(5 ~Height div 5 NewLimit / 5.0)>>
      end
      
      meth display(Ys S)
	 Y          = Ys.1
	 IsLeft     = (@CurX < HalfWidth)
	 NeedsScale = (Y > @CurLimit)
      in
	 %% Check whether display needs to be scrolled
	 case @CurX+S>=Width then
	    TmpTag = @LeftTag
	 in
	    {TmpTag    tk(delete)}
	    {@RightTag tk(move ~HalfWidth 0)}
	    CurX     <- @CurX - HalfWidth
	    LeftTag  <- @RightTag
	    RightTag <- TmpTag
	    case NeedsScale then true
	    else RightLimit = {GetLimit @RightMaxY} in
	       case RightLimit < @CurLimit then
		  <<Load ReScale(RightLimit)>>
		  CurLimit  <- RightLimit
	       else true
	       end
	    end
	    LeftMaxY  <- @RightMaxY
	    RightMaxY <- Y
	 else true end
	 %% Check whether display needs to be rescaled
	 case NeedsScale then
	    NewLimit = {GetLimit Y}
	 in
	    <<Load ReScale(NewLimit)>>
	    CurLimit <- NewLimit
	 else true end
	 case IsLeft then
	    LeftMaxY  <- {Max @LeftMaxY Y}
	 else
	    RightMaxY <- {Max @RightMaxY Y}
	 end 
	 <<Load DisplayLoads(@PrevYs Ys @CurX @CurX+S self.Colors
			     case @CurX+S>HalfWidth then @RightTag
			     else @LeftTag
			     end)>>
	 PrevYs <- Ys
	 CurX   <- @CurX + S
      end

   end

end
