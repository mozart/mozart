%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local
   class Node
      attr
	 x  : 1                   %% xpos
	 y  : 1                   %% ypos
	 c  : ZombieThreadColor   %% color
	 ct : undef               %% canvas tag
	 dy : 0                   %% distance to upper sibling

	 i  : undef               %% thread id
	 q  : undef               %% parent id
      
      meth init(I Q C)
	 i <- I
	 q <- Q
	 c <- C
      end
      
      meth setXY(X Y DY)
	 x  <- X
	 y  <- Y
	 dy <- DY
      end

      meth setColor(C)
	 c <- C
      end

      meth setTag(CT)
	 ct <- CT
      end
      
      meth get($)
	 node(x:@x y:@y c:@c ct:@ct dy:@dy i:@i q:@q)
      end
   end
   
   WC =
   {New class
	   attr w
	   meth init w <- 1 end
	   meth inc($) w <- @w + 1 @w end
	   meth get($) @w end
	end init}

   
   class BaseTree

      attr
	 nodes
      
      meth init
	 nodes <- nil
      end

      meth calculatePositions
	 {WC init}
	 {self DoCalculatePositions(1 1)}  %% toplevel queries
	 {WC inc(_)}
	 {self DoCalculatePositions(0 1)}  %% threads with unknown parent
      end

      meth Nodegroup(Q $)
	 {Reverse {List.filter @nodes
		   fun{$ N}
		      {N get($)}.q == Q
		   end}}
      end

      meth DoCalculatePositions(Q X)
	 NG = {self Nodegroup(Q $)}
      in
	 case NG == nil then
	    skip
	 else
	    Wold = {WC get($)}
	 in
	    {NG.1 setXY(X {WC get($)} 0)}
	    {self DoCalculatePositions({NG.1 get($)}.i X+1)}
	    case NG.2 \= nil then
	       {ForAll NG.2
		proc{$ N}
		   Wnew = {WC inc($)}
		in
		   {N setXY(X Wnew Wnew-Wold)}
		   {self DoCalculatePositions({N get($)}.i X+1)}
		end}
	    else
	       skip
	    end
	 end
      end
      
      meth print
	 {ForAll @nodes proc{$ N} {Show {N get($)}} end}
      end
   end

in
   
   class Tree from BaseTree ScrolledTitleCanvas
      
      attr
	 Selected : undef
      
      meth tkInit(...)=M
	 BaseTree,init
	 ScrolledTitleCanvas,M
      end
      
      meth add(I Q)
	 %% each new thread is runnable, initially... (hope so?)
	 nodes <- {New Node init(I Q RunningThreadColor)} | @nodes
	 BaseTree,calculatePositions
      end
      
      meth remove(I)
	 Tree,mark(I dead)
      end

      meth kill(I)
	 nodes <- {List.filter @nodes fun {$ N} {N get($)}.i \= I end}
	 BaseTree,calculatePositions
	 Tree,display %% should be avoided, but... f*cking Tk...
      end
      
      meth select(I CT<=gaga)
	 case I == 0 then
	    Selected <- undef
	 else
	    N = {List.filter @nodes fun {$ X} {X get($)}.i == I end}
	 in
	    case N \= nil then
	       Selected <- N.1
	       %{self tk(itemconfigure CT font:ThreadTreeBoldFont)}
	    else
	       {OzcarMessage 'Select unknown node?!'}
	    end
	 end
      end
      
      meth mark(I How)
	 N = {List.filter @nodes fun {$ X} {X get($)}.i == I end}
      in
	 case N == nil then
	    {OzcarMessage 'Mark unknown node?!'}
	 else
	    CT
	    Color = case How
		    of         runnable then RunningThreadColor
		    elseof     blocked  then BlockedThreadColor
		    elseof     dead     then DeadThreadColor
		    end
	 in
	    node(ct:CT ...) = {N.1 get($)}
	    %ScrolledTitleCanvas,tk(itemconfigure CT outline:Color)
	    {N.1 setColor(Color)}
	    Tree,display %% should be avoided, but... f*cking Tk...
	 end
      end
      
      meth display
	 SF = ThreadTreeStretch
	 OS = ThreadTreeOffset
	 Sel = @Selected
      in
	 {self tk(delete all)}
	 {ForAll @nodes
	  proc{$ N}
	     X Y C DY I Q
	     CT = {New Tk.canvasTag tkInit(parent:{self w($)})}
	  in
	     node(x:X y:Y c:C dy:DY i:I q:Q ...) = {N get($)}
	     {ForAll [tk(crea oval X*SF-OS Y*SF-OS X*SF+OS Y*SF+OS
			 outline:C width:3 tags:CT
			 fill:white)
		      tk(crea line X*SF-OS Y*SF (X-1)*SF+OS Y*SF
			 width:3)] self}
	     case Q > 0 then
		{self tk(crea line (X-1)*SF+OS Y*SF (X-1)*SF+OS (Y-DY)*SF
			 width:3)}
	     else skip end
	     case N == Sel then
		{self tk(crea text X*SF+1 Y*SF text:I tags:CT
			 font:ThreadTreeBoldFont)}
	     else
		{self tk(crea text X*SF+1 Y*SF text:I tags:CT
			 font:ThreadTreeFont)}
		{CT tkBind(event:  '<1>'
			%action: self # select(I CT))}
			   action: Ozcar # switch(I))}
	     end
	     {N setTag(CT)}
	  end}
      end
   end
end
