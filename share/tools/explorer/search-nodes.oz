%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   FindSpace = {NewName}
   
   local

      fun {FindDepth Node CurDepth}
	 case Node of !False then CurDepth
	 else {FindDepth Node.mom CurDepth+1}
	 end
      end

      fun {GetIndex Xs Y N}
	 !Xs=X|Xr in case X==Y then N else {GetIndex Xr Y N+1} end
      end
      
   in

      class Choose
	 attr
	    isSolBelow:      False
	    toDo:            nil
	    copy:            False
	    choices:         1

	 meth isFinished($)
	    @choices==0
	 end
	 
	 meth leaveNode(IsSolBelow IsDirty DecChoices)
	    ChoicesReachZero
	 in
	    case IsSolBelow  then isSolBelow <- True          else true end
	    case IsDirty     then isDirty    <- True          else true end
	    case DecChoices  then
	       case @choices of 1 then
		  ChoicesReachZero = True
		  choices <- 0
		  case @copy of transient(_) then copy <- False else true end
	       elseof Choices then
		  ChoicesReachZero = False
		  choices    <- Choices - 1
	       end
	    else
	       ChoicesReachZero=False
	    end
	    case self.mom of !False then true elseof Mom then
	       {Mom leaveNode(IsSolBelow IsDirty ChoicesReachZero)}
	    end
	 end
	 
	 meth GotoCopyAbove(CurDepthIn ?CurDepthOut
			    Node CurDistIn ?CurDistOut ?RevNs ?CurCopy)
	    !RevNs = {GetIndex @kids Node 1}|RevNr
	 in
	    case @copy of !False then
	       {self.mom GotoCopyAbove(CurDepthIn+1 ?CurDepthOut
				       self
				       CurDistIn+1 ?CurDistOut
				       ?RevNr ?CurCopy)}
	    elseof TaggedCopy then
	       RevNr       = nil
	       CurDistOut  = CurDistIn
	       CurCopy     = {Space.clone TaggedCopy.1}
	       CurDepthOut = {FindDepth self CurDepthIn + 1}
	    end
	 end
      
	 meth findDepthAndCopy(?CurDepth ?CurDist ?RevNs ?CurCopy)
	    case @copy of !False then
	       {self.mom GotoCopyAbove(1 ?CurDepth
				       self 
				       1 ?CurDist
				       ?RevNs ?CurCopy)}
	    elseof TaggedCopy then
	       CurCopy  = {Space.clone TaggedCopy.1}
	       CurDist  = 0
	       RevNs    = nil
	       CurDepth = {FindDepth self 0}
	    end
	 end

	 meth !FindSpace(Node Copy)
	    case @copy of !False then {self.mom FindSpace(self Copy)}
	    elseof TaggedCopy then Copy={Space.clone TaggedCopy.1}
	    end
	    {Space.choose Copy {GetIndex @kids Node 1}}
	 end
	 
	 meth findSpace($)
	    case @copy of !False then {self.mom FindSpace(self $)}
	    elseof TaggedCopy then {Space.clone TaggedCopy.1}
	    end
	 end
	 
	 meth hasSolutions($)
	    @isSolBelow
	 end

      end
   end
      
   local

      class Leaf
	 meth isNextPossible($)  False end
	 meth isLayerPossible($) False end
	 meth isStepPossible($)  False end
      end

   in

      class Succeeded from Leaf
	 meth isFinished($)      True  end
	 meth hasSolutions($)    True  end
	 meth findSpace($)
	    {self.mom FindSpace(self $)}
	 end
      end
      
      class Failed from Leaf
	 meth isFinished($)      True  end
	 meth hasSolutions($)    False end
      end
      
      class Blocked from Leaf
	 meth isFinished($)      False end
	 meth hasSolutions($)    False end
      end

   end
   
in
   
   SearchNodes = classes(choose:    Choose
			 succeeded: Succeeded
			 failed:    Failed
			 blocked:   Blocked)

end
