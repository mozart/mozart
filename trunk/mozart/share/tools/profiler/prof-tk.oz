%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% some extensions to Tk widgets
%%%

/* a frame with a title */
class TitleFrame from Tk.frame
   feat Label
   meth tkInit(title:T<='' ...)=M
      Tk.frame,{Record.subtract M title}
      case T == '' then skip
      else
	 self.Label = {New Tk.label tkInit(parent: self
					   text:   T
					   font:   TitleFont
					   bd:     0
					   relief: raised)}
	 {Tk.send grid(self.Label row:0 column:0 sticky:we)}
      end
   end
   meth title(S)
      case {IsDet self.Label} then
	 {self.Label tk(conf text:S)}
      else skip end
   end
end

local

   class TitleWidget from TitleFrame
      feat
	 widget W
      meth tkInit(parent:P title:T ...)=M
	 TitleFrame,tkInit(parent:P title:T)
	 self.W  = {New self.widget
		    {Record.subtract {Record.adjoinAt M parent self} title}}
	 {Tk.batch [grid(self.W row:1 column:0 sticky:nswe padx:3)
		    grid(rowconfigure    self 1 weight:1)
		    grid(columnconfigure self 0 weight:1)]}
      end
      meth tk(...)=M
	 {self.W M}
      end
      meth tkBind(...)=M
	 {self.W M}
      end
      meth w($)
	 self.W
      end
   end

   class YScrolledTitleWidget from TitleFrame
      feat
	 widget W
      meth tkInit(parent:P title:T ...)=M
	 TitleFrame,tkInit(parent:P title:T)
	 self.W  = {New self.widget
		    {Record.subtract {Record.adjoinAt M parent self} title}}
	 local
	    SY = {New Tk.scrollbar tkInit(parent:self width:ScrollbarWidth)}
	 in
	    {Tk.addYScrollbar self.W SY}
	    {Tk.batch [grid(self.W row:1 column:0 sticky:nswe)
		       grid(SY     row:1 column:1 sticky:ns)
		       grid(rowconfigure    self 1 weight:1)
		       grid(columnconfigure self 0 weight:1)]}
	 end
      end
      meth tk(...)=M
	 {self.W M}
      end
      meth tkBind(...)=M
	 {self.W M}
      end
      meth w($)
	 self.W
      end
   end
in

   class TitleText from TitleWidget
      meth tkInit(...)=M
	 self.widget = Tk.text
	 TitleWidget,M
      end
   end

   class YScrolledTitleText from YScrolledTitleWidget
      meth tkInit(...)=M
	 self.widget = Tk.text
	 YScrolledTitleWidget,M
      end
   end

   class YScrolledTitleCanvas from YScrolledTitleWidget
      meth tkInit(...)=M
	 self.widget = Tk.canvas
	 YScrolledTitleWidget,M
      end
   end
end

%% discrete scale code of Oz Panel
local
   DarkColor       = black
   BrightColor     = white

   ValueHeight = 14
   ValueBorder = 14
   ScaleHeight = 8
   ScaleBorder = 2
   SliderWidth = 16
   TickSize    = 6


   class TickCanvas
      from Tk.canvas

      meth init(parent:P width:W ticks:N) = M
	 TickCanvas,tkInit(parent:             P
			   width:              W
			   highlightthickness: 0
			   height:             TickSize+1)
      end

      meth drawTicks(Xs)
	 case Xs of nil then skip
	 [] X|Xr then
	    X0 = X - ScaleBorder - 2
	    X1 = X0 + 1
	    X2 = X1 + 1
	    X3 = X2 + 1
	    Y0 = 0
	    Y1 = Y0 + 1
	    Y2 = TickSize - 2
	    Y3 = Y2 + 1
	 in
	    TickCanvas,tk(crea rectangle X0 Y0 X3 Y3 outline:BrightColor)
	    TickCanvas,tk(crea rectangle X1 Y1 X2 Y2 outline:DarkColor)
	    TickCanvas,drawTicks(Xr)
	 end
      end

   end

   class TickScale
      from Tk.scale
      feat Ticks
      meth init(parent:P ticks:N width:W action: A)
	 TickScale,tkInit(parent:             P
			  highlightthickness: 0
			  sliderlength:       SliderWidth
			  action:             A
			  'from':             0
			  to:                 N
			  length:             W
			  width:              ScaleHeight
			  resolution:         1
			  showvalue:          false
			  orient:             horizontal)
	 self.Ticks = N
	 TickScale,tkBind(event:  '<Configure>'
			  action: P # drawTicks)
      end

      meth getCoords($)
	 TickScale,GetCoords(0 self.Ticks $)
      end

      meth GetCoords(I N $)
	 case I>N then nil else
	    {Tk.returnInt lindex(l(self coords I) 0)} |
	    TickScale,GetCoords(I+1 N $)
	 end
      end
   end

in

   class DiscreteScale
      from Tk.frame

      feat
	 Value
	 Scale
	 Ticks
	 Coords
	 Values
      attr
	 CurValue: 0

      meth init(parent:  P
		width:   Width
		values:  Vs
		initpos: N)
	 DiscreteScale,tkInit(parent:P highlightthickness:0)
	 NoTicks   = {Length Vs} - 1
      in
	 self.Value = {New Tk.canvas tkInit(parent: self
					    width:  Width + 2*ValueBorder
					    height: ValueHeight)}
	 self.Scale = {New TickScale init(parent: self
					  width:  Width
					  action: self # Action
					  ticks:  NoTicks)}
	 self.Ticks = {New TickCanvas init(parent: self
					   width:  Width
					   ticks:  NoTicks)}
	 {Tk.batch [pack(self.Value self.Scale self.Ticks side:top)
		    update(idletasks)]}
	 self.Values = Vs
	 {self.Scale tk(set N-1)}
	 CurValue    <- {Nth Vs N}.1
      end

      meth Action(S)
	 N   = {Tk.string.toInt S}+1
	 V#L = {Nth self.Values N}
	 X   = {Nth self.Coords N}
      in
	 {self.Value tk(delete all)}
	 {self.Value tk(crea text X+ValueBorder 0
			anchor:n text:L font:TitleFont)}
	 CurValue <- V
      end

      meth drawTicks
	 Cs = {self.Scale getCoords($)}
      in
	 self.Coords = Cs
	 thread
	    {self.Ticks drawTicks(Cs)}
	 end
      end

      meth get($)
	 @CurValue
      end

   end

end
