functor
import
   Graph(graph:GraphClass)
   Tk
export
   TitleGraph
define
     class TitleGraph from Tk.frame
      feat graph 

      meth tkInit(parent:P title:T ...)=M G L Init in
	 Tk.frame, tkInit(parent:P)
	 Init={Record.adjoin {Record.subtract M title} init(parent:self)}
	 self.graph=G={New GraphClass Init}
	 L={New Tk.label tkInit(parent:self text:T)}
	 {Tk.batch [grid(L row:0 column:0 sticky:we)
		    grid(G row:1 column:0 sticky:news)]}
      end

      meth otherwise(X)
	 {self.graph X}
      end
   end
end
   