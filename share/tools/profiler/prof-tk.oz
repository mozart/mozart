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
					   bd:     NoBorderSize
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
	    SY     = {New Tk.scrollbar
		      tkInit(parent: self
			     width:  ScrollbarWidth
			     borderwidth: SmallBorderSize
			     elementborderwidth: SmallBorderSize)}
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

%% this will go away when TkTools.oz has been changed
%% to support small borderwidths
local
   BarBorder = 1
   BarRelief = raised
   AccSpace  = '    '
   AccCtrl   = 'C-'
   AccAlt    = 'A-'

   local
      fun {MakeFree Fs}
	 case Fs of nil then nil
	 [] F|Fr then F#`ooFreeFlag`|{MakeFree Fr}
	 end
      end
   in
      fun {MakeClass Class Features}
	 case Features of nil then Class
	 else
	    {`class` [Class] nil {MakeFree Features} nil nil '' `send`}
	 end
      end
   end

   fun {GetFeatures Ms}
      case Ms of nil then nil
      [] M|Mr then
	 case {HasFeature M feature} then M.feature|{GetFeatures Mr}
	 else {GetFeatures Mr}
	 end
      end
   end

   local
      fun {MakeEvent R}
	 case R
	 of ctrl(S) then
	    '<Control-'#case S of alt(T) then 'Alt-'#T else S end#'>'
	 [] alt(S) then
	    '<Alt-'#case S of ctrl(T) then 'Control-'#T else S end#'>'
	 else R
	 end
      end
   in
      proc {MakeKey M Menu Item KeyBinder}
	 case {HasFeature M key} then
	    B={HasFeature M event}
	    E={MakeEvent M.key}
	 in
	    {Tk.send bind(KeyBinder
			  case B
			  then M.event
			  else E
			  end
			  v('{') Menu invoke Item v('}'))}
	 else skip
	 end
      end
   end
      

   local
      fun {MakeAcc R}
	 case R
	 of ctrl(S) then AccCtrl#{MakeAcc S}
	 [] alt(S)  then AccAlt#{MakeAcc S}
	 else R
	 end
      end
	 
      proc {ProcessMessage As M ?AMs}
	 case As of nil then AMs=nil
	 [] A|Ar then AMr in
	    AMs = case A
		  of key     then acc#(AccSpace#{MakeAcc M.key})|AMr
		  [] event   then AMr
		  [] feature then AMr
		  [] menu    then AMr
		  else A#M.A|AMr
		  end
	    {ProcessMessage Ar M AMr}
	 end
      end
   in
      fun {MakeMessage M P}
	 {AdjoinList tkInit parent#P|{ProcessMessage {Arity M} M}}
      end
   end
   
   proc {MakeItems Ms Item Menu KeyBinder}
      case Ms of nil then skip
      [] M|Mr then
	 HasMenu = {HasFeature M menu}
	 BaseCl  = Tk.menuentry.{Label M}
	 UseCl   = case HasMenu then
		      FS={GetFeatures M.menu}
		   in
		      {MakeClass BaseCl menu|FS}
		   else BaseCl
		   end
	 M1 = {MakeMessage M Menu}
	 NewItem = {New UseCl M1}
      in
	 {MakeKey M Menu NewItem KeyBinder}
	 case HasMenu then
	    NewMenu = {New Tk.menu tkInit(parent:Menu tearoff:false)}
	 in
	    NewItem.menu = NewMenu
	    {MakeItems M.menu NewItem NewMenu KeyBinder}
	    {NewItem tk(entryconf o(menu:NewMenu))}
	 else skip end
	 case {HasFeature M feature} then Item.(M.feature)=NewItem
	 else skip
	 end
	 {MakeItems Mr Item Menu KeyBinder}
      end
   end

   fun {MakeButtons Ms Bar KeyBinder}
      case Ms of nil then nil
      [] M|Mr then
	 MenuButton = {New {MakeClass Tk.menubutton
			    menu|{GetFeatures M.menu}}
		       {MakeMessage M Bar}}
	 Menu       = {New Tk.menu tkInit(parent:MenuButton tearoff:false)}
      in
	 {MakeItems M.menu MenuButton Menu KeyBinder}
	 {MenuButton tk(conf menu:Menu)}
	 case {HasFeature M feature} then Bar.(M.feature)=MenuButton
	 else skip
	 end
	 MenuButton.menu = Menu
	 MenuButton | {MakeButtons Mr Bar KeyBinder}
      end
   end
      
in

   fun {MyMenuBar Parent KeyBinder L R}
      MenuBar      = {New {MakeClass Tk.frame {Append {GetFeatures L}
					       {GetFeatures R}}}
		      tkInit(parent: Parent
			     border: BarBorder
			     relief: BarRelief)}
      LeftButtons  = {MakeButtons L MenuBar KeyBinder}
      RightButtons = {MakeButtons R MenuBar KeyBinder}
   in
      case {Append
	    case LeftButtons of nil then nil
	    else [pack(b(LeftButtons) side:left fill:x)]
	    end
	    case RightButtons of nil then nil
	    else [pack(b(RightButtons) side:right fill:x)]
	    end}
      of nil then skip
      elseof Tcls then {Tk.batch Tcls}
      end
      MenuBar
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
