%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local
   
   NextSol      = {NewName}
   NextSolBelow = {NewName}
   PrevSol      = {NewName}
   PrevSolBelow = {NewName}
   Back         = {NewName}
   BackBelow    = {NewName}
   
   fun {GetRight Ns M}
      case Ns of nil then nil
      [] N|Nr then case N==M then Nr else {GetRight Nr M} end
      end
   end
   
   fun {GetLeft Ns M}
      case Ns of nil then nil
      [] N|Nr then case N==M then nil else N|{GetLeft Nr M} end
      end
   end
      
   fun {FindNextSolBelow Ns}
      case Ns of nil then False
      [] N|Nr then
	 case N.kind
	 of succeeded then N
	 [] choose then
	    case {N NextSolBelow($)} of !False then {FindNextSolBelow Nr}
	    elseof Sol then Sol
	    end
	 else {FindNextSolBelow Nr}
	 end
      end
   end
      
   fun {FindPrevSolBelow Ns}
      case Ns of nil then False
      [] N|Nr then
	 case N.kind
	 of succeeded then N
	 [] choose then
	    case {N PrevSolBelow($)} of !False then {FindPrevSolBelow Nr}
	    elseof Sol then Sol
	    end
	 else {FindPrevSolBelow Nr}
	 end
      end
   end
   
   fun {FindBackBelow Ns}
      case Ns of nil then False
      [] N|Nr then
	 case N.kind\=choose then {FindBackBelow Nr}
	 elsecase {N BackBelow($)} of !False then {FindBackBelow Nr}
	 elseof B then B
	 end
      end
   end
      
      
   class ChooseNode
      
      meth !NextSolBelow($)
	 @isSolBelow andthen
	 case @isHidden then self else {FindNextSolBelow @kids} end
      end
      
      meth !NextSol(N $)
	 case
	    case @isSolBelow then
	       case @isHidden then self
	       else {FindNextSolBelow {GetRight @kids N}}
	       end
	    else False
	    end
	 of !False then {self.mom NextSol(self $)}
	 elseof N then N
	 end
      end
      
      meth nextSol($)
	 case @isSolBelow then
	    case @isHidden then self
	    else ChooseNode,NextSolBelow($)
	    end
	 else {self.mom NextSol(self $)}
	 end
      end
      
      meth !PrevSolBelow($)
	 @isSolBelow andthen
	 case @isHidden then self
	 else {FindPrevSolBelow {Reverse @kids}}
	 end
      end
      
      meth !PrevSol(N $)
	 case
	    case @isSolBelow then
	       case @isHidden then self
	       else {FindPrevSolBelow {Reverse {GetLeft @kids N}}}
	       end
	    else False
	    end
	 of !False then {self.mom PrevSol(self $)}
	 elseof N then N
	 end
      end
      
      meth prevSol($)
	 {self.mom PrevSol(self $)}
      end
      
      meth leftMost($)
	 Ks = @kids
      in
	 case Ks==nil orelse @isHidden then self
	 else {Ks.1 leftMost($)}
	 end
      end
      
      meth rightMost($)
	 Ks = @kids
      in
	 case Ks==nil orelse @isHidden then self
	 else {{List.last Ks} rightMost($)}
	 end
      end
      
      meth !BackBelow($)
	 case @isHidden then False
	 elsecase @choices==0 then False
	 elsecase {FindBackBelow {Reverse @kids}}
	 of !False then
	    case @toDo\=nil then self else False end
	 elseof N then N
	 end
      end
      
      meth !Back(Son $)
	 case
	    case @isHidden then False
	    elsecase @choices==0 then False
	    else {FindBackBelow {Reverse {GetLeft @kids Son}}}
	    end
	 of !False then
	    case @toDo\=nil then self
	    else self,back($)
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
   
   class FailedOrBlockedNode
      
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
      meth !PrevSol(_ $) False end
      meth !NextSol(_ $) False end
      meth !Back(_ $)    False end
   end
   
in

   MoveNodes = classes(choose:    !ChooseNode
		       succeeded: !SucceededNode
		       failed:    !FailedOrBlockedNode
		       blocked:   !FailedOrBlockedNode
		       sentinel:  !Sentinel)
		       
end
