%%%
%%% Authors:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
%%%   Leif Kornstaedt, 2001
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

local

   local
      fun {DoRemoveAndNext Ns T Last ?Node ?Next}
	 case Ns of N|Nr then
	    if {N getT($)} == T then
	       Node = N
	       Next = case Nr of N|_ then N else Last end
	       Nr
	    else
	       N|{DoRemoveAndNext Nr T N ?Node ?Next}
	    end
	 end
      end
   in
      proc {RemoveAndNext Ns T ?Node ?Next ?Rest}
	 {DoRemoveAndNext Ns T unit ?Node ?Next ?Rest}
      end
   end

   class Node
      attr
	 x  : 1                   %% xpos
	 y  : 1                   %% ypos
	 r  : false               %% root node?
	 s  : unit                %% state
	 ct : unit                %% canvas tag
	 dy : 0                   %% distance to upper sibling
	 t  : unit                %% thread
	 q  : unit                %% parent node id

      meth init(T S)
	 t <- T
	 q <- {Primitives.getParentId T}
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
	 node(x:@x y:@y r:@r s:@s ct:@ct dy:@dy t:@t)
      end
      meth getT($) @t end
      meth getI($) {Primitives.getThreadId @t} end
      meth getQ($) @q end
   end

   class Counter
      attr I
      meth init() I <- 1 end
      meth inc($) I <- @I + 1 @I end
      meth get($) @I end
   end

   NodeConfigurations =
   nodeConfigurations(
      runnable:        ThreadTreeBoldFont#RunnableThreadColor
      blocked:         ThreadTreeBoldFont#BlockedThreadColor
      stoppedRunnable: ThreadTreeFont    #RunnableThreadColor
      stoppedBlocked:  ThreadTreeFont    #BlockedThreadColor
      terminated:      ThreadTreeFont    #DeadThreadColor
      exc:             ThreadTreeFont    #ExcThreadColor)

in

   class Tree from ScrolledTitleCanvas
      prop locking

      attr
	 Nodes
	 NodeCount

	 GotCompilerNode

	 Selected: unit
	 CalcSync: _

      feat
	 ytoNodeDic
	 idtoYDic

      meth tkInit(...)=M
	 self.ytoNodeDic = {Dictionary.new}
	 self.idtoYDic = {Dictionary.new}
	 Nodes <- nil
	 NodeCount <- 0
	 ScrolledTitleCanvas,M
      end

      meth add(T S)
	 lock
	    N = {New Node init(T S)}
	 in
	    Nodes <- N|@Nodes
	    Tree,SyncCalc
	 end
      end

      meth remove(T $)
	 lock Node Next Rest in
	    {RemoveAndNext @Nodes T ?Node ?Next ?Rest}
	    Tree,Reorg(Node)
	    Nodes <- Rest
	    Selected <- unit
	    Tree,SyncCalc
	    case Next of unit then unit
	    else {Next getT($)}
	    end
	 end
      end

      meth previous($)
	 lock
	    case @Selected of unit then unit
	    elseof Old then
	       OldY = {Dictionary.get self.idtoYDic {Old getI($)}}
	       NewY = if OldY == 2 then @NodeCount + 1
		      else OldY - 1
		      end
	    in
	       {{Dictionary.get self.ytoNodeDic NewY} getT($)}
	    end
	 end
      end

      meth next($)
	 lock
	    case @Selected of unit then unit
	    elseof Old then
	       OldY = {Dictionary.get self.idtoYDic {Old getI($)}}
	       NewY = if OldY == @NodeCount + 1 then 2
		      else OldY + 1
		      end
	    in
	       {{Dictionary.get self.ytoNodeDic NewY} getT($)}
	    end
	 end
      end

      meth select(T)
	 lock
	    case T of unit then
	       Selected <- unit
	    else
	       case @Selected of unit then skip
	       elseof N then
		  case {N get($)} of node(ct:OldCT t:OldT ...) then
		     {self tk(itemconfigure OldCT
			      text:{Primitives.getThreadName OldT})}
		  end
	       end
	       Selected <- Tree,GetNode(T $)
	       {self tk(itemconfigure {@Selected get($)}.ct
			text:{Primitives.getThreadName T}#' *')}
	       Tree,ScrollTo
	    end
	 end
      end

      meth mark(T S)
	 lock
	    case Tree,GetNode(T $) of unit then skip
	    elseof N then
	       case {N get($)} of node(ct:CT ...) then
		  F#C = NodeConfigurations.S
	       in
		  {self tk(itemconfigure CT font:F fill:C)}
		  {N setState(S)}
	       end
	    end
	 end
      end

      meth GetNode(T $)
	 case {Filter @Nodes fun {$ X} {X getT($)} == T end} of [N] then N
	 else unit
	 end
      end

      meth Reorg(N)
	 NewQ = {N getQ($)}
      in
	 for N in Tree,Nodegroup({N getI($)} $) do
	    {N setParent(NewQ)}
	 end
      end

      meth Nodegroup(Q $)
	 {Reverse {Filter @Nodes
		   fun {$ N}
		      if {N getI($)} == 1 then %% debugging OPI compiler
			 if @GotCompilerNode then false
			 else
			    GotCompilerNode <- true
			    true
			 end
		      else {N getQ($)} == Q
		      end
		   end}}
      end

      meth SyncCalc New in
	 CalcSync <- New = unit
	 thread
	    {WaitOr New {Alarm TimeoutToUpdate*3}}
	    if {IsFree New} then
	       lock
		  Tree,CalculatePositions
		  Tree,Display
		  Tree,ScrollTo
	       end
	    end
	 end
      end

      meth CalculatePositions
	 Roots = {Dictionary.new}
	 WC = {New Counter init()}
      in
	 {Dictionary.removeAll self.ytoNodeDic}
	 {Dictionary.removeAll self.idtoYDic}
	 for N in @Nodes do
	    Q = {N getQ($)}
	 in
	    if {{Ozcar PrivateSend(getThreadDic($))} isKnownI(Q $)} then
	       {N setRoot(false)}
	    else
	       {N setRoot(true)}
	       {Dictionary.put Roots Q unit}
	    end
	 end

	 GotCompilerNode <- false
	 {self DoCalculatePositions(1 3 WC)}       %% Emacs queries...
	 {self DoCalculatePositions(2 3 WC)}       %% ...Tk actions...
	 for Q in {Dictionary.keys Roots} do
	    if Q \= 1 andthen Q \= 2 then
	       {self DoCalculatePositions(Q 3 WC)} %% ...and all the others
	    end
	 end

	 NodeCount <- {WC get($)} - 1

	 Nodes <- {Map {Sort {Dictionary.entries self.ytoNodeDic}
			fun {$ Y1#_ Y2#_} Y1 > Y2 end}
		   fun {$ _#N} N end}
      end

      meth DoCalculatePositions(Q X WC)
	 case Tree,Nodegroup(Q $) of nil then skip   % leaf node
	 elseof NG then
	    Wold = {WC get($)}
	 in
	    for N in NG do
	       Wnew = {WC inc($)}
	       I    = {N getI($)}
	    in
	       {Dictionary.put self.ytoNodeDic Wnew N}
	       {Dictionary.put self.idtoYDic I Wnew}
	       {N setXY(X Wnew Wnew-Wold)}
	       {self DoCalculatePositions(I X+1 WC)}
	    end
	 end
      end

      meth Display
	 SFX = ThreadTreeStretchX
	 SFY = ThreadTreeStretchY
	 OS  = ThreadTreeOffset
	 Sel = @Selected
      in
	 {self tk(delete all)}
	 for N in @Nodes do
	    CT = {New Tk.canvasTag tkInit(parent:self)}
	 in
	    case {N get($)} of node(x:X y:Y r:R s:S dy:DY t:T ...) then
	       F#C = NodeConfigurations.S
	    in
	       %% the horizontal line
	       {self tk(crea line
			X*SFX-OS     Y*SFY
			(X-1)*SFX-OS Y*SFY
			width:2 capstyle:projecting fill:TrunkColor)}
	       {self tk(crea line
			(X-1)*SFX-OS Y*SFY
			(X-1)*SFX-OS (Y-DY+1)*SFY-if R then 5 else SFY end
			width:2 capstyle:projecting fill:TrunkColor)}

	       {self tk(crea text X*SFX Y*SFY
			text:   ({Primitives.getThreadName T} #
				 if N == Sel then ' *' else '' end)
			font:   F
			fill:   C
			tags:   CT
			anchor: w)}
	       {CT tkBind(event:  '<1>'
			  action: self # SwitchToThread(T))}
	       {N setTag(CT)}
	    end
	 end
	 {self tk(conf scrollregion:q(0 0 ThreadTreeWidth
				      SFY * (@NodeCount + 3)))}
      end

      meth ScrollTo
	 case @Selected of unit then skip
	 elseof Node then
	    Y     = {Dictionary.get self.idtoYDic {Node getI($)}}
	    YF    = {Int.toFloat Y}
	    [A B] = {Tk.returnListFloat o(self yview)}
	    N     = {Int.toFloat @NodeCount + 3}
	 in
	    if N*A+1.0 =< YF andthen YF =< N*B-1.0 then skip  % already visible
	    else  % make visible
	       A2 = YF/N - (B-A)/2.0
	    in
	       {self tk(yview moveto if A2 < 0.0 then 0.0
				     elseif A2 > 1.0 then 1.0
				     else A2
				     end)}
	    end
	 end
      end

      meth SwitchToThread(T)
	 {Ozcar PrivateSend(status('Selected thread ' #
				   {Primitives.getThreadName T}))}
	 {Ozcar PrivateSend(switch(T))}
      end
   end
end
