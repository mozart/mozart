%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   class Node
      attr
	 x  : 1                   %% xpos
	 y  : 1                   %% ypos
	 s  : runnable            %% state
	 ct : undef               %% canvas tag
	 dy : 0                   %% distance to upper sibling

	 i  : undef               %% thread id
	 q  : undef               %% parent id
      
      meth init(I Q S)
	 i <- I
	 q <- Q
	 s <- S
      end
      
      meth setXY(X Y DY)
	 x  <- X
	 y  <- Y
	 dy <- DY
      end

      meth setState(S)
	 s <- S
      end

      meth setTag(CT)
	 ct <- CT
      end
      
      meth get($)
	 node(x:@x y:@y s:@s ct:@ct dy:@dy i:@i q:@q)
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
	 width
      
      meth init
	 nodes <- nil
	 width <- 0
      end

      meth calculatePositions
	 {WC init}

	 {self DoCalculatePositions(1 3)}  %% toplevel queries
	 {self DoCalculatePositions(0 3)}  %% threads with unknown parent

	 local
	    Width = {WC get($)} - 1
	 in
	    width <- Width
	 end
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
	    {NG.1 setXY(X {WC inc($)} 1)}
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

      prop
	 locking
      
      attr
	 Selected       : undef
	 SyncStreamCalc : _
	 SyncStreamDraw : _
      
      meth tkInit(...)=M
	 BaseTree,init
	 ScrolledTitleCanvas,M
      end
      
      meth add(I Q)
	 %% each new thread is runnable, initially... (hope so?)
	 nodes <- {New Node init(I Q runnable)} | @nodes
	 Tree,syncCalc
      end
      
      meth remove(I)
	 Tree,mark(I dead)
      end

      meth syncCalc
	 Old New in
	 Old = SyncStreamCalc <- New
	 Old = _ | New
	 thread
	    {WaitOr New {Alarm TimeoutToCalc}}
	    case {IsDet New} then skip else
	       lock BaseTree,calculatePositions end
	    end
	 end
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
	    %CT
	 %in
	    %node(ct:CT ...) = {N.1 get($)}
	    %ScrolledTitleCanvas,tk(itemconfigure CT outline:Color)
	    {N.1 setState(How)}
	    Tree,display %% should be avoided, but... f*cking Tk...
	 end
      end

      meth display
	 Old New in
	 Old = SyncStreamDraw <- New
	 Old = _ | New
	 thread
	    {WaitOr New {Alarm TimeoutToRedraw}}
	    case {IsDet New} then skip else
	       lock Tree,DoDisplay end
	    end
	 end
      end
      
      meth DoDisplay
	 SFX = ThreadTreeStretchX
	 SFY = ThreadTreeStretchY
         OS  = ThreadTreeOffset
	 Sel = @Selected
      in
	 {self tk(delete all)}
	 {ForAll @nodes
	  proc{$ N}
	     X Y S DY I Q
	     CT = {New Tk.canvasTag tkInit(parent:{self w($)})}
	  in
	     node(x:X y:Y s:S dy:DY i:I q:Q ...) = {N get($)}
             {ForAll [tk(crea line X*SFX-OS Y*SFY (X-1)*SFX-OS Y*SFY
                         width:1 fill:TrunkColor)] self}
             case Q > 0 then
                {self tk(crea line (X-1)*SFX-OS Y*SFY (X-1)*SFX-OS (Y-DY)*SFY
                         width:1 fill:TrunkColor)}
             else skip end

	     local
		CL = case S
		     of     runnable then RunningThreadColor#RunningThreadText
		     elseof blocked  then BlockedThreadColor#BlockedThreadText
		     elseof dead     then DeadThreadColor   #DeadThreadText
		     end
	     in
		case N == Sel then
		   {self tk(crea text X*SFX Y*SFY text:I#CL.2
			    fill:CL.1 tags:CT
			    anchor:w font:ThreadTreeBoldFont)}
		else
		   {self tk(crea text X*SFX Y*SFY text:I#CL.2
			    fill:CL.1 tags:CT
			    anchor:w font:ThreadTreeFont)}
		   {CT tkBind(event:  '<1>'
 			   %action: self # select(I CT))}
			      action: self # SwitchToThread(I))}
		end
	     end
	     {N setTag(CT)}
	  end}
	 local
	    Height = ThreadTreeStretchY * (@width + 3)
	 in
	    {self tk(conf scrollregion:q(0 0 ThreadTreeWidth Height))}
	 end
      end

      meth SwitchToThread(I)
	 {ForAll [rawStatus('You have selected thread #' # I)
		  switch(I)] Ozcar}
      end
   end
end
