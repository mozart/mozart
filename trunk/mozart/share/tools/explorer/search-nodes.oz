%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   FindSpace = {NewName}

   local
      NoChoicesLeft = {NewName}
   
      fun {Replace Xs Y Z}
	 case Xs of nil then nil
	 [] X|Xr then case X==Y then Z|Xr else X|{Replace Xr Y Z} end
	 end
      end
   
      fun {CheckDone Ns}
	 case Ns of nil then True
	 [] N|Nr then
	    case N.kind
	    of failed then True
	    [] succeeded then True
	    else {N NoChoicesLeft($)} andthen {CheckDone Nr}
	    end
	 end
      end
      
      proc {ReDo S Is}
	 case Is of nil then skip
	 [] I|Ir then {ReDo S Ir} {Space.choose S I}
	 end
      end

      fun {NotHiddenChoices Ns}
	 case Ns of nil then False
	 [] N|Nr then
	    case N.kind
	    of succeeded then {NotHiddenChoices Nr}
	    [] failed then {NotHiddenChoices Nr}
	    [] blocked then True
	    [] choose then
	       case {N isHidden($)} orelse {N NoChoicesLeft($)} then
		  {NotHiddenChoices Nr}
	       elsecase {N isStepPossible(False $)} then True
	       else
		  {NotHiddenChoices {N getKids($)}} orelse
		  {NotHiddenChoices Nr}
	       end
	    end
	 end
      end

      fun {GetIndex Xs Y N}
	 !Xs=X|Xr in case X==Y then N else {GetIndex Xr Y N+1} end
      end
      
   in

      class Choose

	 meth isFinished($)
	    @choices==0
	 end
	 
	 meth leaveNode(IsSolBelow IsDirty DecChoices)
	    ChoicesReachZero
	 in
	    case IsSolBelow  then isSolBelow <- True          else skip end
	    case IsDirty     then isDirty    <- True          else skip end
	    case DecChoices  then
	       case @choices of 1 then
		  ChoicesReachZero = True
		  choices <- 0
		  case @copy of transient(_) then copy <- False else skip end
	       elseof Choices then
		  ChoicesReachZero = False
		  choices    <- Choices - 1
	       end
	    else
	       ChoicesReachZero=False
	    end
	    {self.mom leaveNode(IsSolBelow IsDirty ChoicesReachZero)}
	 end

	 meth findDepth(CurDepth $)
	    {self.mom findDepth(CurDepth+1 $)}
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
	       CurDepthOut = {self.mom findDepth(CurDepthIn + 1 $)}
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
	       CurDepth = {self.mom findDepth(1 $)}
	    end
	 end

	 meth !FindSpace(Node $)
	    FindCopy = case @copy of !False then {self.mom FindSpace(self $)}
		       elseof TaggedCopy then {Space.clone TaggedCopy.1}
		       end
	 in
	    case FindCopy==False then False
	    elsecase {Space.ask FindCopy}
	    of alternatives(N) then
	       I={GetIndex @kids Node 1}
	    in
	       case I>N then False
	       else {Space.choose FindCopy I} FindCopy
	       end
	    else False
	    end
	 end
	 
	 meth findSpace($)
	    case @copy of !False then {self.mom FindSpace(self $)}
	    elseof TaggedCopy then {Space.clone TaggedCopy.1}
	    end
	 end
	 
	 meth hasSolutions($)
	    @isSolBelow
	 end

	 meth !NoChoicesLeft($)
	    @choices==0
	 end

	 meth isNextPossible(LeftToRight $)
	    @choices>0 andthen
	    (LeftToRight orelse
	     (<<isStepPossible(LeftToRight $)>> orelse
	      {NotHiddenChoices @kids}))
	 end
	 
	 meth isStepPossible(LeftToRight $)
	    @toDo \= nil andthen
	    (LeftToRight orelse {Not @isHidden} andthen {CheckDone @kids})
	 end

	 meth Create(PrevSol NextDepth Space Control AllocateCopy $)
	    case Control
	    of failed then
	       {New self.classes.failed init(self NextDepth)}
	    [] succeeded(S) then
	       isSolBelow <- True
	       {New self.classes.S init(self NextDepth Space AllocateCopy)}
	    [] alternatives(MaxAlt) then
	       choices  <- @choices + 1
	       {New self.classes.choose
		init(self NextDepth PrevSol AllocateCopy Space MaxAlt)}
	    [] blocked(Ctrl) then
	       %% Count the possibility of a choice below _and_
	       %% the case where toDo is already nil
	       choices <- @choices + case @toDo==nil then 2 else 1 end
	       {self.status halt}
	       {New self.classes.blocked init(self NextDepth Ctrl)}
	    end
	 end
	 
	 meth Add(PrevSol CurDepth InfoDist CurSearchDist CurNs CurCopy
		  ?NextDist ?NextNs ?NextCopy
		  ?Information ?NewNode)
	    UseNs UseCopy AllocateCopy
	 in
	    case @toDo
	    of Sol#S#1#MaxAlt then
	       toDo          <- Sol#2#MaxAlt
	       UseNs         = [1]
	       UseCopy       = S
	       NextDist      = CurSearchDist - 1
	       NextNs        = 1|CurNs
	       AllocateCopy  = case InfoDist>0 andthen CurDepth mod InfoDist
			       of 1 then persistent
			       elsecase NextDist of 0 then transient
			       else False
			       end
	       NextCopy      = CurCopy
	    [] Sol#NextAlt#MaxAlt then
	       case PrevSol==Sol then
		  case NextAlt==MaxAlt then
		     toDo    <- nil
		     choices <- @choices - 1
		     case @choices==0 andthen @copy\=False then
			NextDist = 0 % force allocation of a copy below!
			UseCopy  = case @copy of transient(_) then
				      copy <- False
				      CurCopy
				   else {Space.clone CurCopy}
				end
		     else
			UseCopy  = {Space.clone CurCopy}
			NextDist = CurSearchDist - 1
		     end
		  else
		     toDo     <- Sol#NextAlt+1#MaxAlt
		     UseCopy  = {Space.clone CurCopy}
		     NextDist = CurSearchDist - 1
		  end
		  UseNs = NextNs = NextAlt|CurNs
		  AllocateCopy  = case InfoDist>0 andthen CurDepth mod InfoDist
				  of 1 then persistent
				  elsecase NextDist of 0 then transient
				  else False
				  end
		  NextCopy      = CurCopy
	       else
		  ToMerge = {Space.clone PrevSol}
	       in
		  UseCopy = {Space.clone CurCopy}
		  {ReDo UseCopy NextAlt#MaxAlt|CurNs}
		  %% I'm finished
		  toDo     <- nil
		  choices  <- @choices  - 1
		  case @copy of transient(_) then copy <- False else skip end
		  {Space.inject UseCopy 
		   proc {$ X}
		      {self.order {Space.merge ToMerge} X}
		   end}
		  UseNs = NextNs = nil
		  NextDist     = 0
		  AllocateCopy = persistent
		  NextCopy     = CurCopy
	       end
	    end
	    {ReDo UseCopy UseNs}
	    Information = thread {Space.askVerbose UseCopy} end
	    <<Choose Create(PrevSol CurDepth+1 UseCopy Information
			    AllocateCopy ?NewNode)>> 
	    isDirty <- True
	    kids    <- {Append @kids [NewNode]}
	 end
	 
	 meth NextKids(Break PrevSol CurDepth InfoDist
		       CurSearchDist SearchDist CurNs CurCopy Ks N $)
	    case Ks of nil then False
	    [] K|Kr then 
	       case K.kind==choose then
		  SolBelow IsDirtyBelow DecChoices
	       in
		  {K Next(Break PrevSol CurDepth InfoDist
			  CurSearchDist SearchDist N|CurNs CurCopy
			  ?SolBelow ?IsDirtyBelow ?DecChoices)}
		  case DecChoices   then choices  <- @choices-1  else skip end
		  case IsDirtyBelow then isDirty  <- True        else skip end
		  case SolBelow of !False then
		     <<Choose NextKids(Break PrevSol CurDepth InfoDist
				       CurSearchDist SearchDist CurNs CurCopy
				       Kr N+1 $)>>
		  else
		     isSolBelow <- True
		     SolBelow
		  end
	       else
		  <<Choose NextKids(Break PrevSol CurDepth InfoDist
				    CurSearchDist SearchDist CurNs CurCopy
				    Kr N+1 $)>>
	       end  
	    end
	 end

	 meth NextLocal(Break PrevSol CurDepth InfoDist
			CurSearchDist SearchDist CurNs CurCopy
			?Sol ?IsDirty ?DecChoices)
	    case @toDo\=nil andthen {IsFree Break} then
	       NewNode Information
	       NextDist NextNs NextCopy
	    in
	       <<Choose Add(PrevSol CurDepth InfoDist CurSearchDist
			    CurNs CurCopy
			    ?NextDist ?NextNs ?NextCopy
			    ?Information ?NewNode)>>
	       case Information
	       of succeeded(_) then
		  Sol         = NewNode
		  IsDirty     = True
		  DecChoices  = @choices==0
	       [] alternatives(_) then
		  SolBelow DecChoicesBelow
	       in
		  {NewNode Next(Break PrevSol CurDepth+1 InfoDist
				NextDist SearchDist
				NextNs NextCopy
				?SolBelow _
				?DecChoicesBelow)}
		  case DecChoicesBelow  then choices <- @choices - 1
		  else skip end
		  case SolBelow of !False then
		     <<Choose NextLocal(Break PrevSol CurDepth InfoDist
					CurSearchDist SearchDist
					CurNs CurCopy
					?Sol ?IsDirty
					?DecChoices)>>
		  else
		     isSolBelow <- True
		     Sol         = SolBelow
		     IsDirty     = True
		     DecChoices  = @choices==0
		  end
	       else
		  <<Choose NextLocal(Break PrevSol CurDepth InfoDist
				     CurSearchDist SearchDist
				     CurNs CurCopy
				     ?Sol ?IsDirty
				     ?DecChoices)>>
	       end
	    else
	       %% Oh, we are doomed: no sols at all
	       Sol         = False
	       IsDirty     = @isDirty
	       DecChoices  = @choices==0
	    end
	 end
	 
	 meth Next(Break PrevSol CurDepth InfoDist
		   CurSearchDist SearchDist
		   CurNs CurCopy
		   ?Sol ?IsDirty ?DecChoices)
	    %% Check the existing kids
	    case @isHidden orelse @choices==0 then
	       %% In this node and below everything is done
	       Sol         = False
	       IsDirty     = False
	       DecChoices  = False
	    else
	       %% Okay there are choices left. So first check whether the 
	       %% already created kids can contribute to a solution.
	       NextNs NextDist NextCopy
	    in
	       case @copy of !False then
		  NextDist = CurSearchDist
		  NextCopy = CurCopy
		  NextNs   = CurNs
	       elseof Copy then
		  NextDist = SearchDist
		  NextCopy = Copy.1
		  NextNs   = nil
	       end
	       case <<Choose NextKids(Break PrevSol CurDepth+1 InfoDist
				      NextDist-1 SearchDist NextNs NextCopy
				      @kids 1 $)>>
	       of !False then
		  %% Now we have to create new kids to find whether a solution
		  %% does exists there.
		  <<Choose NextLocal(Break PrevSol CurDepth InfoDist
				     NextDist SearchDist NextNs NextCopy
				     ?Sol ?IsDirty ?DecChoices)>>
	       elseof S then
		  %% Okay one of our kids found a solution.
		  Sol         = S
		  IsDirty     = True
		  DecChoices  = @choices==0
	       end
	    end
	 end
	 
	 meth next(Break PrevSol SearchDist InfoDist ?Sol)
	    CurDepth CurNs CurSearchDist CurCopy
	 in
	    <<Choose findDepthAndCopy(?CurDepth ?CurSearchDist
				      ?CurNs ?CurCopy)>>
	    case @choices==0 then Sol=False
	    else IsDirty DecChoices in
	       <<Choose Next(Break PrevSol CurDepth InfoDist
			     SearchDist-CurSearchDist SearchDist
			     CurNs CurCopy
			     ?Sol ?IsDirty ?DecChoices)>>
	       {self.mom leaveNode(Sol\=False IsDirty DecChoices)}
	    end
	 end
	 
	 meth step(PrevSol InfoDist ?Sol)
	    case @toDo==nil then Sol=False
	    else
	       Info NewNode CurDepth CurNs CurSearchDist CurCopy
	    in
	       <<Choose findDepthAndCopy(?CurDepth ?CurSearchDist
					 ?CurNs ?CurCopy)>>
	       <<Choose Add(PrevSol CurDepth InfoDist CurSearchDist
			    CurNs CurCopy
			    _ _ _
			    ?Info ?NewNode)>>
	       Sol = case {Label Info}==succeeded then NewNode else False end
	       {self.mom leaveNode(Sol\=False True @choices==0)}
	    end
	 end
	 
	 meth removeLast(PrevSol)
	    %% Has at least a single kid, and that kid was blocked
	    choices <- @choices - 1 
	    toDo    <- case @toDo
		       of Sol#NextAlt#MaxAlt then
			  Sol#NextAlt-1#MaxAlt
		       [] nil then MaxAlt={Length @kids} in
			  %% In this case @choices is still greater than zero!
			  PrevSol#MaxAlt#MaxAlt
		       end
	    kids <- {Reverse {Reverse @kids}.2}
	 end
	 
      end
   end
      
   local
      class Leaf
	 meth isFinished($)
	    True
	 end
	 meth hasSolutions($)
	    False
	 end
	 meth isNextPossible(_ $)
	    False
	 end
	 meth isStepPossible(_ $)
	    False
	 end
      end
   in
      class Succeeded from Leaf
	 attr copy: False
	 meth hasSolutions($)
	    True
	 end
	 meth findSpace($)
	    case @copy of !False then {self.mom FindSpace(self $)}
	    elseof TaggedCopy then {Space.clone TaggedCopy.1}
	    end
	 end
	 meth getOriginalSpace($)
	    @copy.1
	 end
	 meth next(_ _ _ _ $)
	    False
	 end
	 meth step(_ _ $)
	    False
	 end
      end
      
      Failed = Leaf
      
      class Blocked from Leaf
	 meth isFinished($)
	    False
	 end
      end
   end

   class Sentinel
      meth leaveNode(_ _ _)
	 skip
      end
      meth findDepth(CurDepth $)
	 CurDepth
      end
   end
   
in
   
   SearchNodes = classes(choose:    Choose
			 succeeded: Succeeded
			 failed:    Failed
			 blocked:   Blocked
			 sentinel:  Sentinel)

end
