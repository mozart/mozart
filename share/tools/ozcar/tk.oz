%%%
%%% Author:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.

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
			     width:  ScrollbarWidth)}
	 in
	    {Tk.addYScrollbar self.W SY}
	    {Tk.batch [grid(self.W row:1 column:0 sticky:nswe padx:1 pady:1)
		       grid(SY     row:1 column:1 sticky:ns   padx:1 pady:1)
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
	 NextTag

      meth tkInit(...)=M
	 self.widget  = Tk.text
	 self.TagBase = ~1
	 NextTag <- self.TagBase
	 ScrolledTitleWidget,M
      end

      meth newTag($)
	 NextTag <- @NextTag - 1
      end

      meth resetTags
	 ScrolledTitleText,DeleteTags(NextTag <- self.TagBase)
      end

      meth DeleteTags(N)
	 case N >= self.TagBase then skip else
	    N1 = N + 1 in
	    {self tk(tag delete N1)}
	    ScrolledTitleText,DeleteTags(N1)
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
