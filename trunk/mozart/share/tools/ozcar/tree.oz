%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   fun {TreeNext L Xs I}
      case Xs of nil then nil
      [] X|Xr then
	 case X == I then
	    case Xr == nil then L.1
	    else Xr.1 end
	 else
	    {TreeNext L Xr I}
	 end
      end
   end
   
   fun {TreePrev Xs I}
      case Xs of nil then nil
      [] X|Xr then
	 case Xr == nil then X else
	    case Xr.1 == I then X
	    else {TreePrev Xr I} end
	 end
      end
   end

   local
      fun {DoTreeRemoveAndNext Xs F N}
	 case Xs of nil then nil
	 [] X|Xr then case {F X} then
			 X | {DoTreeRemoveAndNext Xr F N}
		      else
			 N = Xr
			 {DoTreeRemoveAndNext Xr F N}
		      end
	 end
      end
   in
      fun {TreeRemoveAndNext Xs F}
	 N R in
	 {DoTreeRemoveAndNext {Reverse Xs} F N R}
	 case N == nil then
	    case R == nil then nil # nil else {Reverse R} # {Reverse R}.1 end
	 else {Reverse R} # N.1 end
      end
   end
   
   fun {GetColor State}
      case State
      of runnable then RunningThreadColor # RunningThreadText
      [] blocked  then BlockedThreadColor # BlockedThreadText
      [] dead     then DeadThreadColor    # DeadThreadText
      end
   end
   
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

      meth setParent(Q)
	 q <- Q
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

      meth find(I $)
	 {List.filter @nodes fun {$ N} {N get($)}.i == I end}.1
      end
	 
      meth reorg(I)
	 NC = BaseTree,Nodegroup(I $)
	 NI = BaseTree,find(I $)
	 NQ = {NI get($)}.q
      in
	 {ForAll NC
	  proc {$ N} {N setParent(NQ)} end}
      end
      
      meth Nodegroup(Q $)
	 {Reverse {List.filter @nodes
		   fun{$ N}
		      {N get($)}.q == Q
		   end}}
      end

      meth DoCalculatePositions(Q X)
	 NG = BaseTree,Nodegroup(Q $)
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
	 LastSelected   : undef

	 SyncCalc       : _
      
      meth tkInit(...)=M
	 BaseTree,init
	 ScrolledTitleCanvas,M
      end
      
      meth add(I Q)
	 %% each new thread is runnable, initially... (hope so?)
	 lock
	    nodes <- {New Node init(I Q runnable)} | @nodes
	    Tree,syncCalc
	 end
      end
      
      meth remove(I)
	 Tree,mark(I dead)
      end

      meth syncCalc
	 New in
	 SyncCalc <- New = unit
	 thread
	    lock
	       {WaitOr New {Alarm TimeoutToCalcTree}}
	       case {IsDet New} then skip else
		  BaseTree,calculatePositions
		  Tree,display
	       end
	    end
	 end
      end
      
      meth kill(I Next)
	 R # N = {TreeRemoveAndNext @nodes fun {$ N} {N get($)}.i \= I end}
      in
	 BaseTree,reorg(I)
	 nodes <- R
	 Tree,syncCalc
	 Next = case N == nil then 0 else {N get($)}.i end
      end

      meth selectPrevious
	 N = {TreeNext @nodes @nodes @Selected}
      in
	 case N == nil then skip else
	    Tree,SwitchToThread({N get($)}.i)
	 end
      end
      
      meth selectNext
	 N = {TreePrev @nodes @Selected}
      in
	 case N == nil then skip else
	    Tree,SwitchToThread({N get($)}.i)
	 end
      end
      
      meth select(I)
	 case I == 0 then
	    LastSelected <- undef
	    Selected     <- undef
	 else
	    CT OldCT N = {List.filter @nodes fun {$ X} {X get($)}.i == I end}
	 in
	    case N \= nil then
	       LastSelected <- @Selected
	       Selected <- N.1

	       case @LastSelected \= undef then
		  node(ct:OldCT ...) = {@LastSelected get($)}
		  ScrolledTitleCanvas,tk(itemconfigure OldCT
					 font:ThreadTreeFont)
	       else skip end
	       
	       node(ct:CT ...) = {@Selected get($)}
	       ScrolledTitleCanvas,tk(itemconfigure CT font:ThreadTreeBoldFont)
	    else
	       {OzcarMessage 'Select unknown node?!'}
	    end
	 end
      end
      
      meth mark(I How)
	 CT N = {List.filter @nodes fun {$ X} {X get($)}.i == I end}
      in
	 case N == nil then
	    {OzcarMessage 'Mark unknown node?!'}
	 else
	    CL = {GetColor How}
	 in
	    node(ct:CT ...) = {N.1 get($)}
	    ScrolledTitleCanvas,tk(itemconfigure CT fill:CL.1 text:I#CL.2)
	    {N.1 setState(How)}
	 end
      end

      meth display
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
		CL = {GetColor S}
	     in
		{self tk(crea text X*SFX Y*SFY
			 text:   I # CL.2
			 fill:   CL.1
			 tags:   CT
			 anchor: w
			 font:   case N == Sel then ThreadTreeBoldFont
				 else               ThreadTreeFont end)}
		{CT tkBind(event:  '<1>'
			   action: self # SwitchToThread(I))}
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
	 {ForAll [status(SwitchMessage # I)
		  switch(I)] Ozcar}
      end
   end
end
