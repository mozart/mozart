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

local

   local
      fun {DoTreeRemoveAndNext Xs F N}
	 case Xs of nil then nil
	 [] X|Xr then if {F X} then
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
	 if N == nil then
	    if R == nil then nil # nil else {Reverse R} # {Reverse R}.1 end
	 else {Reverse R} # N.1 end
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
	 ytoNodeDic
	 idtoYDic

      attr
	 nodes
	 width

	 trees

	 GotCompilerNode

      meth init(O)
	 self.ThreadDic = {O getThreadDic($)}
	 self.ytoNodeDic = {Dictionary.new}
	 self.idtoYDic = {Dictionary.new}
	 nodes <- nil
	 width <- 0
      end

      meth calculatePositions
	 trees <- {Dictionary.new}
	 {Dictionary.removeAll self.ytoNodeDic}
	 {Dictionary.removeAll self.idtoYDic}
	 {ForAll @nodes
	  proc {$ N}
	     Q = {N get($)}.q
	  in
	     if {Dictionary.member self.ThreadDic Q} then
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
		      if D.i == 1 then %% whoops, debugging OPI compiler...
			 if @GotCompilerNode then false else
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
	 if NG == nil then /* leaf node */ skip else
	    Wold = {WC get($)}
	 in
	    {ForAll NG
	     proc{$ N}
		Wnew = {WC inc($)}
		I    = {N get($)}.i
	     in
		{Dictionary.put self.ytoNodeDic Wnew N}
		{Dictionary.put self.idtoYDic I Wnew}
		{N setXY(X Wnew Wnew-Wold)}
		{self DoCalculatePositions(I X+1)}
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
	 CalcSync       : _

      meth tkInit(ozcar:O ...)=M
	 BaseTree,init(O)
	 ScrolledTitleCanvas,{Record.subtract M ozcar}
      end

      meth add(I Q)
	 lock
	    nodes <- {New Node init(I Q stopped#runnable)} | @nodes
	    Tree,syncCalc(I)
	 end
      end

      meth syncCalc(I<=unit)
	 New in
	 CalcSync <- New = unit
	 thread
	    {WaitOr New {Alarm TimeoutToUpdate*3}}
	    if {IsDet New} then skip else
	       lock
		  BaseTree,calculatePositions
		  Tree,display(I)
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
	 Next = if N == nil then 0 else {N get($)}.i end
      end

      meth selectPrevious
	 Old = @Selected
	 OldY = {Dictionary.get self.idtoYDic {Old get($)}.i}
	 NewY = if OldY == 2 then
		   @width + 1
		else
		   OldY - 1
		end
	 New = {Dictionary.get self.ytoNodeDic NewY}
      in
	 if New \= Old then
	    Tree,SwitchToThread({New get($)}.i)
	 end
      end

      meth selectNext
	 Old = @Selected
	 OldY = {Dictionary.get self.idtoYDic {Old get($)}.i}
	 NewY = if OldY == @width + 1 then
		   2
		else
		   OldY + 1
		end
	 New = {Dictionary.get self.ytoNodeDic NewY}
      in
	 if New \= Old then
	    Tree,SwitchToThread({New get($)}.i)
	 end
      end

      meth IsVisible(Y $)
	 [A B] = {Tk.returnListFloat o(self yview)}
	 N     = {Int.toFloat @width + 3}
	 YF    = {Int.toFloat Y}
      in
	 N*A+1.0 =< YF andthen YF =< N*B-1.0
      end

      meth MakeVisible(Y $)
	 [A B] = {Tk.returnListFloat o(self yview)}
	 N     = {Int.toFloat @width + 3}
	 YF    = {Int.toFloat Y}
	 A2    = YF/N - (B-A)/2.0
      in
	 if A2 < 0.0 then 0.0 elseif A2 > 1.0 then 1.0 else A2 end
      end

      meth condScroll(I)
	 try
	    Y = {Dictionary.get self.idtoYDic I}
	 in
	    if Tree,IsVisible(Y $) then skip else
	       {self tk(yview moveto Tree,MakeVisible(Y $))}
	    end
	 catch _ then skip end
      end

      meth select(I)
	 if I == 0 then
	    LastSelected <- unit
	    Selected     <- unit
	 else
	    N = {List.filter @nodes fun {$ X} {X get($)}.i == I end}
	 in
	    if N \= nil then
	       LastSelected <- @Selected
	       Selected <- N.1
	       if @LastSelected \= unit then
		  case {@LastSelected get($)} of node(ct:OldCT i:OldI ...) then
		     {self tk(itemconfigure OldCT text:OldI)}
                  end
	       end
	       {self tk(itemconfigure {@Selected get($)}.ct text:I#' *')}
	       Tree,condScroll(I)
	    else
	       {OzcarError 'attempt to select unknown node ' # I}
	    end
	 end
      end

      meth mark(I How)
	 N = {List.filter @nodes fun {$ X} {X get($)}.i == I end}
      in
	 if N == nil then
	    {OzcarError 'attempt to mark unknown node ' # I}
	 elsecase {N.1 get($)} of node(ct:CT s:S ...) then
	    case How
	    of running then
	       {self tk(itemconfigure CT font:ThreadTreeBoldFont)}
	       {N.1 setState(How#S.2)}
	    [] stopped then
	       {self tk(itemconfigure CT font:ThreadTreeFont)}
	       {N.1 setState(How#S.2)}
	    [] runnable then
	       {self tk(itemconfigure CT fill:RunnableThreadColor)}
	       {N.1 setState(S.1#How)}
	    [] blocked then
	       {self tk(itemconfigure CT fill:BlockedThreadColor)}
	       {N.1 setState(S.1#How)}
	    [] exc then
	       {self tk(itemconfigure CT fill:ExcThreadColor)}
	       {N.1 setState(S.1#How)}
	    [] dead then
	       {self tk(itemconfigure CT fill:DeadThreadColor)}
	       {N.1 setState(S.1#How)}
	    end
	 end
      end

      meth display(ScrollTo)
	 SFX    = ThreadTreeStretchX
	 SFY    = ThreadTreeStretchY
	 OS     = ThreadTreeOffset
	 Sel    = @Selected
      in
	 {self tk(delete all)}
	 {ForAll @nodes
	  proc{$ N}
	     CT = {New Tk.canvasTag tkInit(parent:self)}
	  in
	     case {N get($)} of node(x:X y:Y r:R s:S dy:DY i:I ...) then

	     %% the horizontal line
	     {self tk(crea line X*SFX-OS Y*SFY (X-1)*SFX-OS Y*SFY
		      width:2 capstyle:projecting fill:TrunkColor)}
	     if R then
		{self tk(crea line (X-1)*SFX-OS Y*SFY
			 (X-1)*SFX-OS (Y-DY+1)*SFY-5
			 width:2 capstyle:projecting fill:TrunkColor)}
	     else
		{self tk(crea line (X-1)*SFX-OS Y*SFY
			 (X-1)*SFX-OS (Y-DY+1)*SFY-SFY
			 width:2 capstyle:projecting fill:TrunkColor)}
	     end

	     {self tk(crea text X*SFX Y*SFY
		      text:   I # if N == Sel then ' *' else '' end
		      fill:   case S.2
			      of runnable then RunnableThreadColor
			      [] blocked  then BlockedThreadColor
			      [] exc      then ExcThreadColor
			      [] dead     then DeadThreadColor
			      end
		      font:   if S.1 == running then
				 ThreadTreeBoldFont
			      else
				 ThreadTreeFont
			      end
		      tags:   CT
		      anchor: w)}
	     {CT tkBind(event:  '<1>'
			action: self # SwitchToThread(I))}
	     {N setTag(CT)}
             end
	  end}
	 local
	    Height = ThreadTreeStretchY * (@width + 3)
	 in
	    {self tk(conf scrollregion:q(0 0 ThreadTreeWidth Height))}
	    if ScrollTo \= unit then
	       Tree,condScroll(ScrollTo)
	    end
	 end
      end

      meth SwitchToThread(I)
	 {Ozcar PrivateSend(status('New selected thread is ' # I))}
	 {Ozcar PrivateSend(switch(I))}
      end
   end
end
