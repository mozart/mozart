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

/* widget with a title and scrollbar */
local
   class ScrolledTitleWidget from TitleFrame
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
   class ScrolledTitleText from ScrolledTitleWidget
      feat
	 TagBase
      attr
	 CurTag

      meth tkInit(...)=M
	 self.widget  = Tk.text
	 self.TagBase = 1000 % low integers are reserved for stack frame clicks
	 CurTag <- self.TagBase
	 ScrolledTitleWidget,M
      end

      meth newTag($)
	 CurTag <- @CurTag + 1
      end

      meth resetTags
	 ScrolledTitleText,DeleteTags(CurTag <- self.TagBase)
      end

      meth resetReservedTag(N)
	 {self tk(tag delete N)}
      end

      meth resetReservedTags(N)
	 ScrolledTitleText,DeleteTags(N 0)
      end

      meth DeleteTags(N Base<=self.TagBase)
	 case N < Base then skip else
	    {self tk(tag delete N)}
	    ScrolledTitleText,DeleteTags(N-1 Base)
	 end
      end
   end

   class ScrolledTitleCanvas from ScrolledTitleWidget
      meth tkInit(...)=M
	 self.widget = Tk.canvas
	 ScrolledTitleWidget,M
      end
   end
end

class TkExtEntry from Tk.entry
%% the original widget doesn't have some important bindings
   meth tkInit(...)=M
      Tk.entry,M
      {self tkBind(event: '<Control-u>'
		   action: self # tk(delete 0 'end'))}
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

   
   fun {MakeClass C Fs}
      case Fs of nil then Class else
	 {Class.extendFeatures C f Fs}
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
