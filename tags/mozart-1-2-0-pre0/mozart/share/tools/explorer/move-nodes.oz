%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1997
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
%%%

local
   
   NextSol      = {NewName}
   NextSolBelow = {NewName}
   PrevSol      = {NewName}
   PrevSolBelow = {NewName}
   Back         = {NewName}
   BackBelow    = {NewName}
   
   fun {GetRight Ns M}
      case Ns of nil then nil
      [] N|Nr then if N==M then Nr else {GetRight Nr M} end
      end
   end
   
   fun {GetLeft Ns M}
      case Ns of nil then nil
      [] N|Nr then if N==M then nil else N|{GetLeft Nr M} end
      end
   end
      
   fun {FindNextSolBelow Ns}
      case Ns of nil then false
      [] N|Nr then
	 case N.kind
	 of succeeded then N
	 [] choose then
	    case {N NextSolBelow($)} of false then {FindNextSolBelow Nr}
	    elseof Sol then Sol
	    end
	 else {FindNextSolBelow Nr}
	 end
      end
   end
      
   fun {FindPrevSolBelow Ns}
      case Ns of nil then false
      [] N|Nr then
	 case N.kind
	 of succeeded then N
	 [] choose then
	    case {N PrevSolBelow($)} of false then {FindPrevSolBelow Nr}
	    elseof Sol then Sol
	    end
	 else {FindPrevSolBelow Nr}
	 end
      end
   end
   
   fun {FindBackBelow Ns}
      case Ns of nil then false
      [] N|Nr then
	 if N.kind\=choose then {FindBackBelow Nr}
	 else
	    case {N BackBelow($)} of false then {FindBackBelow Nr}
	    elseof B then B
	    end
	 end
      end
   end
      
      
   class ChooseNode
      
      meth !NextSolBelow($)
	 @isSolBelow andthen
	 if @isHidden then self else {FindNextSolBelow @kids} end
      end
      
      meth !NextSol(N $)
	 case
	    if @isSolBelow then
	       if @isHidden then self
	       else {FindNextSolBelow {GetRight @kids N}}
	       end
	    else false
	    end
	 of false then {self.mom NextSol(self $)}
	 elseof N then N
	 end
      end
      
      meth nextSol($)
	 if @isSolBelow then
	    if @isHidden then self
	    else ChooseNode,NextSolBelow($)
	    end
	 else {self.mom NextSol(self $)}
	 end
      end
      
      meth !PrevSolBelow($)
	 @isSolBelow andthen
	 if @isHidden then self
	 else {FindPrevSolBelow {Reverse @kids}}
	 end
      end
      
      meth !PrevSol(N $)
	 case
	    if @isSolBelow then
	       if @isHidden then self
	       else {FindPrevSolBelow {Reverse {GetLeft @kids N}}}
	       end
	    else false
	    end
	 of false then {self.mom PrevSol(self $)}
	 elseof N then N
	 end
      end
      
      meth prevSol($)
	 {self.mom PrevSol(self $)}
      end
      
      meth leftMost($)
	 Ks = @kids
      in
	 if Ks==nil orelse @isHidden then self
	 else {Ks.1 leftMost($)}
	 end
      end
      
      meth rightMost($)
	 Ks = @kids
      in
	 if Ks==nil orelse @isHidden then self
	 else {{List.last Ks} rightMost($)}
	 end
      end
      
      meth !BackBelow($)
	 if @isHidden then false
	 elseif @choices==0 then false
	 else
	    case {FindBackBelow {Reverse @kids}}
	    of false then
	       if @toDo\=nil then self else false end
	    elseof N then N
	    end
	 end
      end
      
      meth !Back(Son $)
	 case
	    if @isHidden then false
	    elseif @choices==0 then false
	    else {FindBackBelow {Reverse {GetLeft @kids Son}}}
	    end
	 of false then
	    if @toDo\=nil then self
	    else {self back($)}
	    end
	 elseof N then N
	 end
      end
	 
      meth back($)
	 {self.mom Back(self $)}
      end
      
   end

   
   class SucceededNode
      meth nextSol($)
	 {self.mom NextSol(self $)}
      end
      
      meth prevSol($)
	 {self.mom PrevSol(self $)}
      end
      
      meth leftMost($)
	 self
      end
      
      meth rightMost($)
	 self
      end
      
      meth back($)
	 {self.mom Back(self $)}
      end
      
   end
   
   class FailedOrSuspendedNode
      
      meth back($)
	 {self.mom Back(self $)}
      end
      
      meth leftMost($)
	 self.mom
      end
      
      meth rightMost($)
	 self.mom
      end
      
   end

   class Sentinel
      meth !PrevSol(_ $) false end
      meth !NextSol(_ $) false end
      meth !Back(_ $)    false end
   end
   
in

   MoveNodes = classes(choose:    ChooseNode
		       succeeded: SucceededNode
		       failed:    FailedOrSuspendedNode
		       suspended: FailedOrSuspendedNode
		       sentinel:  Sentinel)
		       
end
