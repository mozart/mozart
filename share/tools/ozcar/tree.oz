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
      of runnable then RunnableThreadColor # RunnableThreadText
      [] running  then RunningThreadColor  # RunnableThreadText
      [] blocked  then BlockedThreadColor  # BlockedThreadText
      [] dead     then DeadThreadColor     # DeadThreadText
      end
   end

   class Node
      attr
	 x  : 1                   %% xpos
	 y  : 1                   %% ypos
	 r  : false               %% root node?
	 s  : runnable            %% state
	 ct : unit                %% canvas tag
	 dy : 0                   %% distance to upper sibling

	 i  : unit                %% thread id
	 q  : unit                %% parent id

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

      meth setRoot(R)
	 r <- R
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
	 node(x:@x y:@y r:@r s:@s ct:@ct dy:@dy i:@i q:@q)
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

      feat
	 ThreadDic

      attr
	 nodes
	 width

	 trees

	 GotCompilerNode

      meth init(O)
	 self.ThreadDic = {O getThreadDic($)}
	 nodes <- nil
	 width <- 0
      end

      meth calculatePositions
	 trees <- {Dictionary.new}
	 {ForAll @nodes
	  proc {$ N}
	     Q = {N get($)}.q
	  in
	     case {Dictionary.member self.ThreadDic Q} then
		{N setRoot(false)}
	     else
		{N setRoot(true)}
		%% we don't care if Q gets inserted twice...
		{Dictionary.put @trees Q unit}
	     end
	  end}

	 {WC init}

	 GotCompilerNode <- false
	 {self DoCalculatePositions(1 3)}     %% Emacs queries...
	 {self DoCalculatePositions(2 3)}     %% ...Tk actions...
	 {ForAll {Filter {Dictionary.keys @trees}
		  fun {$ Q} Q \= 1 andthen Q \= 2 end}
	  proc {$ T}
	     {self DoCalculatePositions(T 3)} %% ...and all the others
	  end}

	 width <- {WC get($)} - 1
      end

      meth find(I $)
	 {List.filter @nodes fun {$ N} {N get($)}.i == I end}.1
      end

      meth reorg(I)
	 NC = BaseTree,Nodegroup(I $)
	 NI = BaseTree,find(I $)
	 NQ = {NI get($)}.q
      in
	 {ForAll NC proc {$ N} {N setParent(NQ)} end}
      end

      meth Nodegroup(Q $)
	 {Reverse {Filter @nodes
		   fun{$ N}
		      D = {N get($)}
		   in
		      case D.i == 1 then %% whoops, debugging OPI compiler...
			 case @GotCompilerNode then false else
			    GotCompilerNode <- true
			    true
			 end
		      else
			 D.q == Q
		      end
		   end}}
      end

      meth DoCalculatePositions(Q X)
	 NG = BaseTree,Nodegroup(Q $)
      in
	 case NG == nil then /* leaf node */ skip else
	    Wold = {WC get($)}
	 in
	    {ForAll NG
	     proc{$ N}
		Wnew = {WC inc($)}
	     in
		{N setXY(X Wnew Wnew-Wold)}
		{self DoCalculatePositions({N get($)}.i X+1)}
	     end}
	 end
      end
   end

in

   class Tree from BaseTree ScrolledTitleCanvas

      prop
	 locking

      attr
	 Selected       : unit
	 LastSelected   : unit

	 SyncCalc       : _

	 MsgList        : nil
	 MsgListTl      : nil

      meth tkInit(ozcar:O ...)=M
	 BaseTree,init(O)
	 ScrolledTitleCanvas,{Record.subtract M ozcar}
      end

      meth add(I Q)
	 %% each new thread is runnable, initially... (hope so?)
	 lock
	    nodes <- {New Node init(I Q runnable)} | @nodes
	    Tree,syncCalc
	 end
      end

      meth syncCalc
	 New in
	 SyncCalc <- New = unit
	 thread
	    {WaitOr New {Alarm TimeoutToCalcTree}}
	    case {IsDet New} then skip else
	       lock
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
	 Old = @Selected
	 New = {TreeNext @nodes @nodes Old}
      in
	 case New == nil orelse New == Old then skip else
	    Tree,SwitchToThread({New get($)}.i)
	 end
      end

      meth selectNext
	 Old = @Selected
	 New = {TreePrev @nodes Old}
      in
	 case New == nil orelse New == Old then skip else
	    Tree,SwitchToThread({New get($)}.i)
	 end
      end

      meth select(I)
	 case I == 0 then
	    LastSelected <- unit
	    Selected     <- unit
	 else
	    CT OldCT N = {List.filter @nodes fun {$ X} {X get($)}.i == I end}
	 in
	    case N \= nil then
	       LastSelected <- @Selected
	       Selected <- N.1

	       case @LastSelected \= unit then
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
	 SFX    = ThreadTreeStretchX
	 SFY    = ThreadTreeStretchY
	 OS     = ThreadTreeOffset
	 Sel    = @Selected
	 Canvas = {self w($)}
      in
	 Tree,Enqueue(o(Canvas delete all))
	 {ForAll @nodes
	  proc{$ N}
	     X Y R S DY I
	     CT = {New Tk.canvasTag tkInit(parent:Canvas)}
	  in
	     node(x:X y:Y r:R s:S dy:DY i:I ...) = {N get($)}

	     %% the horizontal line
	     Tree,Enqueue(o(Canvas crea line X*SFX-OS Y*SFY (X-1)*SFX-OS Y*SFY
			    width:2 capstyle:projecting fill:TrunkColor))
	     case R then
%		case Y > 2 andthen DY == 1 then
%		   %% the stippled line to separate thread trees
%		   Tree,Enqueue(o(Canvas crea line SFX (Y-DY+1)*SFY-7
%				  10*SFX (Y-DY+1)*SFY-7
%				  stipple:OzcarBitmapDir#'line.xbm'))
%		else skip end
		Tree,Enqueue(o(Canvas crea line (X-1)*SFX-OS Y*SFY
			       (X-1)*SFX-OS (Y-DY+1)*SFY-5
			       width:2 capstyle:projecting fill:TrunkColor))

	     else
		Tree,Enqueue(o(Canvas crea line (X-1)*SFX-OS Y*SFY
			       (X-1)*SFX-OS (Y-DY+1)*SFY-SFY
			       width:2 capstyle:projecting fill:TrunkColor))
	     end

	     local
		CL = {GetColor S}
	     in
		Tree,Enqueue(o(Canvas crea text X*SFX Y*SFY
			       text:   I # CL.2
			       fill:   CL.1
			       tags:   CT
			       anchor: w
			       font:   case N == Sel then ThreadTreeBoldFont
				       else               ThreadTreeFont end))
		{CT tkBind(event:  '<1>'
			   action: self # SwitchToThread(I))}
	     end
	     {N setTag(CT)}
	  end}
	 local
	    Height = ThreadTreeStretchY * (@width + 3)
	 in
	    Tree,Enqueue(o(Canvas conf
			   scrollregion:q(0 0 ThreadTreeWidth Height)))
	 end
	 Tree,ClearQueue
      end

      meth SwitchToThread(I)
	 {Ozcar PrivateSend(status('New selected thread is #' # I))}
	 {Ozcar PrivateSend(switch(I))}
      end

      meth Enqueue(Ticklet)
	 case Ticklet
	 of nil  then skip
	 [] T|Tr then
	    Gui,Enqueue(T)
	    Gui,Enqueue(Tr)
	 else NewTl in
	    case {IsDet @MsgListTl} then
	       MsgList <- Ticklet|NewTl
	    else
	       @MsgListTl = Ticklet|NewTl
	    end
	    MsgListTl <- NewTl
	 end
      end

      meth ClearQueue
	 @MsgListTl = nil
	 {Tk.batch @MsgList}
	 MsgList <- nil
      end

   end
end
