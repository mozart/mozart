%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local
   class Node
      attr
	 x  : undef
	 y  : undef

	 dy : undef
      
	 t  : undef
	 i  : undef
      
      meth init(T I)
	 t <- T
	 i <- I
      end
      
      meth setXY(X Y DY)
	 x  <- X
	 y  <- Y
	 dy <- DY
      end
      
      meth get($)
	 node(x:@x y:@y dy:@dy t:@t i:@i)
      end
   end
   
   WC =
   {New class
	   attr w
	   meth init w <- 1 end
	   meth inc($) w <- @w + 1 @w end
	   meth get($) @w end
	end init}
in
   
   class BaseTree
      
      attr
	 nodes
      
      meth init
	 nodes <- nil
      end
      
      meth add(T I)
	 nodes <- {New Node init(T I)} | @nodes
	 BaseTree,calculatePositions
      end

      meth remove(T)
	 nodes <- {List.filter @nodes fun {$ N} {N get($)}.t \= T end}
	 
	 {self calculatePositions}
      end
      
      meth calculatePositions
	 {WC init}
	 {self DoCalculatePositions(1 1)}
      end

      meth Nodegroup(P $)
	 {Reverse {List.filter @nodes
		   fun{$ N}
		      {Thread.id {Dbg.parent {N get($)}.t}} == P
		   end}}
      end

      meth DoCalculatePositions(P X)
	 NG = {self Nodegroup(P $)}
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
end

class Tree from BaseTree ScrolledTitleCanvas
   
   attr
      Selected : undef

   meth tkInit(...)=M
      BaseTree,init
      ScrolledTitleCanvas,M
   end

   meth select(T)
      L = {List.filter @nodes fun {$ N} {N get($)}.t == T end}
   in
      case L \= nil then
	 Selected <- L.1
      else skip end
   end

   meth display
      SF = 50
      OS = 15
      Sel = @Selected
   in
      {self tk(delete all)}
      {ForAll @nodes
       proc{$ N}
	  X Y DY I T
	  CT = {New Tk.canvasTag tkInit(parent:{self w($)})}
       in
	  node(x:X y:Y dy:DY i:I t:T ...) = {N get($)}
	  {Show hugo(x:X y:Y dy:DY i:I t:T)}
	  {ForAll [tk(crea oval X*SF-OS Y*SF-OS X*SF+OS Y*SF+OS
		      outline:black width:3 tags:CT
		      fill:white)
		   tk(crea line X*SF-OS Y*SF (X-1)*SF+OS Y*SF
		      width:3)
		   tk(crea line (X-1)*SF+OS Y*SF (X-1)*SF+OS (Y-DY)*SF
		      width:3)] self}
	  case N == Sel then
	     {self tk(crea text X*SF+1 Y*SF text:I tags:CT
		      font:ThreadTreeBoldFont)}
	  else
	     {self tk(crea text X*SF+1 Y*SF text:I tags:CT
		      font:ThreadTreeFont)}
	     {CT tkBind(event:  '<1>'
			action: Ozcar # switch(T {Thread.id T}))}
	  end
       end}
   end
end
