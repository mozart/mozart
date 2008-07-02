%%%
%%% Authors:
%%%   Nils Franzen <nilsf@sics.se>
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
export
   graph:Load
define
   Load
   LoadWidth = 300
   DefaultUpdateTime = 1
   DefaultHistoryRange = 50
   LineColor = black
   
   fun {TclGetConf T Opt}
      l(lindex(l(T conf '-'#Opt) 4))
   end

   ActiveColor     = white
   LeftWidth       = 50
   HalfWidth       = LoadWidth div 2
   Border          = 5
   Height          = 100

   FontFamily      = '-*-helvetica-medium-r-normal--*-'
   FontMatch       = '-*-*-*-*-*-*'
   TickFont        = FontFamily # 100 # FontMatch

   local
      Log10 = {Log 10.0}

      fun {Grow N L}
	 if N=<L then L else {Grow N 10.0*L} end
      end

      fun {Shrink N L}
	 L2 = L  / 2.0   L4 = L2 / 2.0
      in
	 if N=<L4 then L4
	 elseif N=<L2 then L2
	 else L
	 end
      end
   in
      fun {GetLimit N}
	 if N=<0.25 then 0.25
	 elseif N=<1.0 then 1.0
	 elseif N=<5.0 then 5.0
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
   class Load from Tk.canvas
      prop final
      feat
	 MinY
	 BothTag
	 TextTag
	 CoverTag
	 DivideTag
	 Dimension
	 Colors
	 Stipple
	 Fill
	 GraphDict
	 
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

      meth init(parent:P maxy:Y miny:MY dim:Dim fill:Fi)
	 Limit = {GetLimit {Max MY Y}}
      in
	 Tk.canvas,tkInit(parent:             P
			  width:              LoadWidth+LeftWidth+1
			  height:             Height+1+2*Border
			  highlightthickness: 0
			  xscrollincrement:   1
			  yscrollincrement:   1)
	 Tk.canvas,tk(yview scroll ~Height-Border units)
	 Tk.canvas,tk(xview scroll ~LeftWidth     units)
	 LeftTag      <- {New Tk.canvasTag tkInit(parent: self)}
	 RightTag     <- {New Tk.canvasTag tkInit(parent: self)}
	 self.BothTag   =  {New Tk.canvasTag tkInit(parent: self)}
	 self.CoverTag  =  {New Tk.canvasTag tkInit(parent: self)}
	 self.TextTag   =  {New Tk.canvasTag tkInit(parent: self)}
	 self.DivideTag   =  {New Tk.canvasTag tkInit(parent: self)}
	 self.Dimension = Dim # ' '
	 self.Fill      = Fi
	 self.GraphDict = {NewDictionary}
	 CurScale  <- {IntToFloat Height} / Limit
	 CurLimit  <- Limit
	 LeftMaxY  <- Y
	 RightMaxY <- 0.0
	 self.MinY = MY
	 Tk.canvas,tk(crea rectangle 0 0 LoadWidth ~Height fill:ActiveColor)
	 Load,DrawTicks(5 ~ Height div 5)
	 Tk.canvas,tk(crea rectangle
		      ~LeftWidth-4 Border ~1 ~Height - Border
		      outline: {TclGetConf self bg}
		      fill:    {TclGetConf self bg}
		      tags:    self.CoverTag)
	 Tk.canvas,tk(crea rectangle 0 0 LoadWidth ~Height)
	 Load,DrawLabel(5 ~ Height div 5 Limit / 5.0 Limit=<1.0)
      end

      meth DrawTicks(N D)
	 Load,tk(crea line 0 D*N LoadWidth D*N fill:LineColor)
	 if N>0 then Load,DrawTicks(N-1 D) else skip end
      end

      meth DrawLabel(N D Y IsFrac)
	 Load,tk(crea text 0 D*N
		 font: TickFont
		 text: if IsFrac then
			  case N*{FloatToInt Y*10.0}
			  of 10 then '1.0'
			  elseof M then '0.'#M
			  end
		       else N*{FloatToInt Y}
		       end #' '#self.Dimension
		 anchor: e
		 tags:   self.TextTag)
	 Load,tk('raise' self.TextTag)
	 if N>0 then Load,DrawLabel(N-1 D Y IsFrac) else skip end
      end

      meth addGraph(key:K col:C stp:S val:V<=0.0)
	 VC = {NewCell V}
	 Tag = {New Tk.canvasTag tkInit(parent:self)}
      in
	 {Dictionary.put self.GraphDict K r(col:C stp:S val:VC tag:Tag)}
      end

      meth rmGraph(key:K)
	 {Dictionary.remove self.GraphDict K}
      end

      meth GetOldVal(key:K A)
	 {Dictionary.get self.GraphDict K A}
      end

      meth SetNewVal(key:K val:V)
	 {Assign {Dictionary.get self.GraphDict K $}.val V}
      end

      meth divider(col:Col)
	 T = if @CurX < HalfWidth then @LeftTag else @RightTag end
      in
	 Tk.canvas,tk(crea line
		      @CurX 0 @CurX ~Height
		      width:1
		      fill: Col
		      tags: q(T self.DivideTag))
	% Load,tk(lower self.DivideTag self.CoverTag)
	 %Load,tk('raise' self.DivideTag)
      end
      
      meth DisplayLoads(Y2s X1 X2 T)
	 case Y2s of nil then skip
	 else
	    (Y2#Key)|Y2r = Y2s
	    Y1rec  = Load,GetOldVal(key:Key $)
	    C      = Y1rec.col
	    LineTag= Y1rec.tag
	    CS     = ~@CurScale
	    Y1     = {Access Y1rec.val}
	    Y3     = case Y2r of nil then 0.0 [] Y|_ then {Access {self GetOldVal(key:Y.2 $)}.val $} end
	    Y4     = case Y2r of nil then 0.0 [] Y|_ then Y.1 end
	 in
	    Load,SetNewVal(key:Key val:Y2)
	    if self.Fill then 
	       Tk.canvas,tk(crea polygon
			    X1 CS*Y3 X1 CS*Y1 X2 CS*Y2 X2 CS*Y4
			    fill: C 
			    tags: q(T self.BothTag))
	       Tk.canvas,tk(crea line
			    X1 CS*Y1 X2 CS*Y2
			    tags: q(T self.BothTag))
	    else
	       Tk.canvas,tk(crea line
			    X1 CS*Y1 X2 CS*Y2
			    width:3
			    fill: C
			    tags: q(T self.BothTag LineTag))
	    end
	    Load,tk(lower self.BothTag self.CoverTag)
	    Load,DisplayLoads(Y2r X1 X2 T)
	 end
      end

      meth ReScale(NewLimit)
	 NewScale = {IntToFloat Height} / NewLimit
      in
	 Tk.canvas,tk(scale self.BothTag 0 0 1 NewScale / @CurScale)
	 CurScale <- NewScale
	 {self.TextTag tk(delete)}
	 Load,DrawLabel(5 ~Height div 5 NewLimit / 5.0 NewLimit=<1.0)
      end

      meth clear
	 NewLimit = {GetLimit {FoldL @PrevYs Max self.MinY}}
      in
	 {@LeftTag  tk(delete)}
	 {@RightTag tk(delete)}
	 CurX <- 0
	 if NewLimit==@CurLimit then skip else
	    Load,ReScale(NewLimit)
	    CurLimit <- NewLimit
	 end
      end

      meth slice(S)
	 Slice <- S
	 Load,clear
      end
      meth otherwise(X)
	 raise calling_unknown(X graph) end
      end
      meth raise_line(Id)
	 {self tk('raise' self.GraphDict.Id.tag)}
      end
      
      
      meth display(Ys)
	 S          = @Slice
	 Y          = {FoldL Ys proc{$ X Y Ans} {Max Y.1 X Ans} end 0.0}
	 L          = @CurLimit
	 NeedsScale = (Y > L)
      in
	 %% Check whether display needs to be scrolled
	 if @CurX+S >= LoadWidth then
	    TmpTag = @LeftTag
	 in
	    {TmpTag    tk(delete)}
	    {@RightTag tk(move ~HalfWidth 0)}
	    CurX     <- @CurX - HalfWidth
	    LeftTag  <- @RightTag
	    RightTag <- TmpTag
	    if NeedsScale then skip
	    else RightLimit = {GetLimit {Max @RightMaxY self.MinY}} in
	       if RightLimit < L then
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
	 if NeedsScale then NewLimit = {GetLimit {Max Y self.MinY}} in
	    Load,ReScale(NewLimit)
	    CurLimit <- NewLimit
	 else skip
	 end
	 if @CurX+S < HalfWidth then LeftMaxY <- {Max @LeftMaxY Y}
	 else RightMaxY <- {Max @RightMaxY Y}
	 end
	 Load,DisplayLoads(Ys @CurX @CurX+S 
			   if @CurX+S < HalfWidth then @LeftTag
			   else @RightTag
			   end)
	 CurX   <- @CurX+S
      end
   end
end
