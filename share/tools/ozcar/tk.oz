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
	 self.Label = {New Tk.label tkInit(parent:self text:T
					   bd:1 relief:raised)}
	 {Tk.send grid(self.Label row:0 column:0 sticky:we)}
      end
   end
   meth title(S)
      case {IsDet self.Label} then
	 {self.Label tk(conf text:S)}
      else skip end
   end
end

/* widget with a title and both vertical and horizontal scrollbar */
local
   class ScrolledTitleWidget from TitleFrame
      feat
	 widget W
      meth tkInit(parent:P title:T ...)=M
	 TitleFrame,tkInit(parent:P title:T)
	 self.W  = {New self.widget
		    {Record.subtract {Record.adjoinAt M parent self} title}}
	 local
	    SX = {New Tk.scrollbar tkInit(parent:self orient:horizontal)}
	    SY = {New Tk.scrollbar tkInit(parent:self)}
	 in
	    {Tk.addXScrollbar self.W SX}
	    {Tk.addYScrollbar self.W SY}
	    {Tk.batch [grid(self.W row:1 column:0 sticky:nswe)
		       grid(SX     row:2 column:0 sticky:we)
		       grid(SY     row:1 column:1 sticky:ns)]}
	    {Tk.batch [grid(rowconfigure    self 1 weight:1)
		       grid(columnconfigure self 0 weight:1)]}
	 end
      end
      meth tk(...)=M
	 {self.W M}
      end
      meth w($)
	 self.W
      end
   end
in
   class ScrolledTitleText from ScrolledTitleWidget
      meth tkInit(...)=M
	 self.widget = Tk.text
	 ScrolledTitleWidget,M
      end
   end
   
   class ScrolledTitleCanvas from ScrolledTitleWidget
      meth tkInit(...)=M
	 self.widget = Tk.canvas
	 ScrolledTitleWidget,M
      end
   end
end

/*

declare
W = {New Tk.toplevel tkInit}
T = {New ScrolledText tkInit(parent:W title:gurgl)}
{Tk.send pack(T)}

{T tk(insert '0.0' "Hallo Benni")}

*/		   
