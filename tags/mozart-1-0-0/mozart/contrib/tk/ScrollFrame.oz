functor
import Tk
export ScrollFrame
define
   class ScrollFrame from Tk.frame
      feat frame
      meth tkInit(scroll:S<=both ...)=M
	 Canvas Vscroll Hscroll
	 HS = S==x orelse S==both
	 VS = S==y orelse S==both
      in
	 Tk.frame,{Record.subtract M scroll}
	 Canvas  = {New Tk.canvas    tkInit(parent:self)}
	 Vscroll = {New Tk.scrollbar tkInit(parent:self orient:v)}
	 Hscroll = {New Tk.scrollbar tkInit(parent:self orient:h)}
	 self.frame = {New Tk.frame tkInit(parent:Canvas)}
	 {Tk.addYScrollbar Canvas Vscroll}
	 {Tk.addXScrollbar Canvas Hscroll}
	 {Canvas tk(create window 0 0 window:self.frame anchor:nw)}
	 {self.frame
	  tkBind(event :'<Configure>'
		 action:proc{$}
			   [X Y W H] = {Canvas
					tkReturnListInt(bbox all $)}
			in
			   {Canvas
			    tk(configure
			       if HS then unit else o(width :W) end
			       if VS then unit else o(height:H) end
			       scrollregion:q(X Y W H)
			       xscrollincrement:1
			       yscrollincrement:1)}
			end)}
	 local
	    L1 = grid(columnconfigure self 0 weight:1)
	    |    grid(rowconfigure    self 0 weight:1)
	    |    grid(Canvas  row:0 column:0 sticky:nswe)
	    |    L2
	    L2 = if VS then
		    grid(Vscroll row:0 column:1 sticky:nsw)
		    | L3
		 else L3 end
	    L3 = if HS then
		    grid(Hscroll row:1 column:0 sticky:nwe)
		    | L4
		 else L4 end
	    L4 = nil
	 in
	    {Tk.batch L1}
	 end
      end
   end
end
