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
   LeftWidth       = 50
   HalfWidth       = LoadWidth div 2
   Border          = 5
   BlackTickSize   = 4 
   Height          = 70

   FontFamily      = '-*-helvetica-medium-r-normal--*-'
   FontMatch       = '-*-*-*-*-*-*'
   TickFont        = !FontFamily # 100 # !FontMatch

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
	 Stipple
      attr
	 LeftTag:      unit
	 RightTag:     unit
	 LeftMaxY:     0.0
	 RightMaxY:    0.0
	 PrevYs:       nil
	 CurX:         0
	 CurScale:     1.0
	 CurLimit:     0.0
	 Slice:        (LoadWidth * DefaultUpdateTime) div DefaultHistoryRange
      
      meth init(parent:P maxy:Y dim:Dim colors:Cs stipple:Ss)
	 Limit = {GetLimit Y}
      in
	 Tk.canvas,tkInit(parent:             P
			  width:              LoadWidth+LeftWidth+1
			  height:             Height+1+2*Border
			  highlightthickness: 0
			  xscrollincrement:   1
			  yscrollincrement:   1)
	          ,tk(yview scroll ~Height-Border units)
	          ,tk(xview scroll ~LeftWidth     units)
	 LeftTag      <- {New Tk.canvasTag tkInit(parent: self)}
	 RightTag     <- {New Tk.canvasTag tkInit(parent: self)}
	 self.BothTag   =  {New Tk.canvasTag tkInit(parent: self)} 
	 self.CoverTag  =  {New Tk.canvasTag tkInit(parent: self)} 
	 self.TextTag   =  {New Tk.canvasTag tkInit(parent: self)}
	 self.Dimension = Dim # ' '
	 self.Colors    = Cs
	 self.Stipple   = Ss
	 CurScale  <- {IntToFloat Height} / Limit
	 CurLimit  <- Limit
	 LeftMaxY  <- Y
	 RightMaxY <- 0.0
	 Tk.canvas,tk(crea rectangle 0 0 LoadWidth ~Height fill:ActiveColor)
	 Load,DrawTicks(5 ~ Height div 5)
	 Tk.canvas,tk(crea rectangle
		      ~LeftWidth-4 Border ~1 ~Height - Border
		      outline: {TclGetConf self bg}
		      fill:    {TclGetConf self bg}
		      tags:    self.CoverTag)
	          ,tk(crea rectangle 0 0 LoadWidth ~Height)
	 Load,DrawLabel(5 ~ Height div 5 Limit / 5.0)
      end

      meth DrawTicks(N D)
	 Load,tk(crea line 0 D*N LoadWidth D*N stipple:DashLine)
	 case N>0 then Load,DrawTicks(N-1 D) else skip end
      end
      
      meth DrawLabel(N D Y)
	 Load,tk(crea text 0 D*N
		 font: TickFont
		 text: N*{FloatToInt Y}#' '#self.Dimension
		 anchor: e
		 tags:   self.TextTag)
	     ,tk(raise self.TextTag)
	 case N>0 then Load,DrawLabel(N-1 D Y) else skip end
      end
      
      meth DisplayLoads(Y1s Y2s X1 X2 Cs Ss T)
	 case Y1s of nil then skip
	 [] Y1|Y1r then
	    Y2|Y2r = !Y2s
	    C|Cr   = !Cs 
	    S|Sr   = !Ss
	    CS     = ~@CurScale
	    Y3     = case Y1r of nil then 0.0 [] Y|_ then Y end
	    Y4     = case Y2r of nil then 0.0 [] Y|_ then Y end
	 in
	    Tk.canvas,tk(crea polygon
			 X1 CS*Y3 X1 CS*Y1 X2 CS*Y2 X2 CS*Y4
			 fill: C stipple: S
			 tags: q(T self.BothTag))
	             ,tk(crea line
			 X1 CS*Y1 X2 CS*Y2
			 tags: q(T self.BothTag))
	    Load,tk(lower self.BothTag self.CoverTag)
	        ,DisplayLoads(Y1r Y2r X1 X2 Cr Sr T)
	 end
      end

      meth ReScale(NewLimit)
	 NewScale = {IntToFloat Height} / NewLimit
      in
	 Tk.canvas,tk(scale self.BothTag 0 0 1 NewScale / @CurScale)
	 CurScale <- NewScale
	 {self.TextTag tk(delete)}
	 Load,DrawLabel(5 ~Height div 5 NewLimit / 5.0)
      end

      meth clear
	 NewLimit = {GetLimit {FoldL @PrevYs Max 0.0}}
      in
	 {@LeftTag  tk(delete)}
	 {@RightTag tk(delete)}
	 CurX <- 0
	 case NewLimit==@CurLimit then skip else
	    Load,ReScale(NewLimit)
	    CurLimit <- NewLimit
	 end
      end

      meth slice(S)
	 Slice <- S
	 Load,clear
      end
      
      meth display(Ys)
	 S          = @Slice
	 Y          = {FoldL Ys Max 0.0}
	 L          = @CurLimit
	 NeedsScale = (Y > L)
      in
	 %% Check whether display needs to be scrolled
	 case @CurX+S >= LoadWidth then
	    TmpTag = @LeftTag
	 in
	    {TmpTag    tk(delete)}
	    {@RightTag tk(move ~HalfWidth 0)}
	    CurX     <- @CurX - HalfWidth
	    LeftTag  <- @RightTag
	    RightTag <- TmpTag
	    case NeedsScale then skip
	    else RightLimit = {GetLimit @RightMaxY} in
	       case RightLimit < L then
		  Load,ReScale(RightLimit)
		  CurLimit  <- RightLimit
	       else skip
	       end
	    end
	    LeftMaxY  <- @RightMaxY
	    RightMaxY <- Y
	 else skip
	 end
	 %% Check whether display needs to be rescaled
	 case NeedsScale then NewLimit = {GetLimit Y} in
	    Load,ReScale(NewLimit)
	    CurLimit <- NewLimit
	 else skip
	 end
	 case @CurX+S < HalfWidth then LeftMaxY <- {Max @LeftMaxY Y}
	 else RightMaxY <- {Max @RightMaxY Y}
	 end 
	 Load,DisplayLoads(@PrevYs Ys @CurX @CurX+S self.Colors self.Stipple
			   case @CurX+S < HalfWidth then @LeftTag
			   else @RightTag
			   end)
	 PrevYs <- Ys
	 CurX   <- @CurX+S
      end

   end

end
