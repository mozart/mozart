%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

%declare
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

class Tree from BaseTree Tk.canvas

   attr
      Selected : undef

   meth init(parent:P width:W height:H)
      BaseTree,init
      Tk.canvas,tkInit(parent:P width:W height:H)
      {Tk.send pack(self)}
   end

   meth select(T)
      % todo: handle error when parent ID == 1 (nothing to debug anymore)
      Selected <- {List.filter @nodes fun {$ N} {N get($)}.t == T end}.1
      Tree,display
   end

   meth display
      SF = 70
      OS = 20
      Sel = @Selected
   in
      {self tk(delete all)}
      {ForAll @nodes
       proc{$ N}
	  X Y DY I T
	  CT = {New Tk.canvasTag tkInit(parent:self)}
       in
	  node(x:X y:Y dy:DY i:I t:T ...) = {N get($)}
	  {ForAll [tk(crea oval X*SF-OS Y*SF-OS X*SF+OS Y*SF+OS
		      outline:black width:3 tags:CT
		      fill:white)
		   tk(crea text X*SF+1 Y*SF text:I tags:CT)
		   tk(crea line X*SF-OS Y*SF (X-1)*SF+OS Y*SF
		      width:3)
		   tk(crea line (X-1)*SF+OS Y*SF (X-1)*SF+OS (Y-DY)*SF
		      width:3)] self}
	  case N == Sel then
	     {self tk(crea rect X*SF-OS Y*SF-OS X*SF+OS Y*SF+OS width:2)}
	  else
	     {CT tkBind(event:  '<1>'
			action: Ozcar # switch(T {Thread.id T}))}
	  end
       end}
   end
end


/*

local
   X = 1 Y = 1
   SF = 70 OS = 20
   X1 = X*SF-OS
   Y1 = Y*SF-OS
   X2 = X*SF+OS
   Y2 = Y*SF+OS
   TX = X*SF
   TY = Y*SF
in
   {ForAll [tk(crea oval X1 Y1 X2 Y2)
	    tk(crea text TX+1 TY text:10*X#11*Y)] C}
end

declare
W = {New Tk.toplevel tkInit}
T = {New Tree init(parent:W width:500 height:700)}
{T add(1 2 undef)}
{T add(1 3 undef)}
{T add(1 4 undef)}
{T add(2 9 undef)}
{T add(2 11 undef)}
{T add(3 7 undef)}
{T add(3 8 undef)}
{T add(4 5 undef)}
{T add(4 6 undef)}
{T add(5 10 undef)}
{T add(5 12 undef)}
{T add(6 20 undef)}
{T add(6 21 undef)}
{T add(20 22 undef)}
{T add(20 23 undef)}
{T add(22 24 undef)}
{T display}

{T print}


declare
W = {New Tk.toplevel tkInit}
C = {New Tk.canvas tkInit(parent:W)}
{Tk.send pack(C)}

{C tk(delete all)}

*/


