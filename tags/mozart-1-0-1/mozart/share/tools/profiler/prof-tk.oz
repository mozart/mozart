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
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.

/* a frame with a title */
class TitleFrame from Tk.frame
   feat Label
   meth tkInit(title:T<='' ...)=M
      Tk.frame,{Record.subtract M title}
      if T == '' then skip
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
      if {IsDet self.Label} then
	 {self.Label tk(conf text:S)}
      end
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

   class YScrolledTitleCanvas from YScrolledTitleWidget
      meth tkInit(...)=M
	 self.widget = Tk.canvas
	 YScrolledTitleWidget,M
      end
   end

end
