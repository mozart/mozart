%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   FindSpace = {NewName}

   local

      fun {Replace Xs Y Z}
	 case Xs of nil then nil
	 [] X|Xr then case X==Y then Z|Xr else X|{Replace Xr Y Z} end
	 end
      end
   
      proc {ReDo S Is}
	 case Is of nil then skip
	 [] I|Ir then {ReDo S Ir} {Space.commit S I}
	 end
      end

      fun {GetIndex Xs Y N}
	 X|Xr=Xs in case X==Y then N else {GetIndex Xr Y N+1} end
      end
      
   in

      class Choose

	 meth isFinished($)
	    @choices==0
	 end
	 
	 meth leaveNode(IsSolBelow IsDirty DecChoices)
	    ChoicesReachZero
	 in
	    case IsSolBelow  then isSolBelow <- true else skip end
	    case IsDirty     then isDirty    <- true else skip end
	    case DecChoices  then
	       case @choices of 1 then
		  ChoicesReachZero = true
		  choices <- 0
		  case @copy
		  of transient(_) then
		     copy <- false
		  [] flushable(_) then
		     case IsSolBelow then skip
		     elsecase
			{Dictionary.get self.manager.options.search failed}
		     then
			copy <- false
		     else skip
		     end
		  else skip
		  end
	       elseof Choices then
		  ChoicesReachZero = false
		  choices <- Choices - 1
	       end
	    else
	       ChoicesReachZero=false
	    end
	    {self.mom leaveNode(IsSolBelow IsDirty ChoicesReachZero)}
	 end

	 meth findDepth(CurDepth $)
	    {self.mom findDepth(CurDepth+1 $)}
	 end
	 
	 meth GotoCopyAbove(CurDepthIn ?CurDepthOut
			    Node CurDistIn ?CurDistOut ?RevNs ?CurCopy)
	    {GetIndex @kids Node 1}|RevNr = RevNs
	 in
	    case @copy of false then
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
	    case @copy of false then
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
	    FindCopy = case @copy of false then {self.mom FindSpace(self $)}
		       elseof TaggedCopy then {Space.clone TaggedCopy.1}
		       end
	 in
	    case FindCopy==false then false
	    elsecase {Space.ask FindCopy}
	    of alternatives(N) then
	       I={GetIndex @kids Node 1}
	    in
	       case I>N then false
	       else {Space.commit FindCopy I} FindCopy
	       end
	    else false
	    end
	 end
	 
	 meth findSpace($)
	    case @copy of false then {self.mom FindSpace(self $)}
	    elseof TaggedCopy then {Space.clone TaggedCopy.1}
	    end
	 end
	 
	 meth hasSolutions($)
	    @isSolBelow
	 end

	 meth isNextPossible($)
	    @choices>0
	 end
	 
	 meth isStepPossible($)
	    @toDo \= nil
	 end

	 meth Create(PrevSol NextDepth Space Control AllocateCopy $)
	    case Control
	    of failed then
	       {New self.classes.failed init(self NextDepth)}
	    [] succeeded(S) then
	       isSolBelow <- true
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
			       of 1 then flushable
			       elsecase NextDist of 0 then transient
			       else false
			       end
	       NextCopy      = CurCopy
	    [] Sol#NextAlt#MaxAlt then
	       case PrevSol==Sol then
		  case NextAlt==MaxAlt then
		     toDo    <- nil
		     choices <- @choices - 1
		     case @choices==0 andthen @copy\=false then
			NextDist = 0 % force allocation of a copy below!
			UseCopy  = case @copy of transient(_) then
				      copy <- false
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
				  of 1 then flushable
				  elsecase NextDist of 0 then transient
				  else false
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
		  case @copy of transient(_) then copy <- false else skip end
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
	    Choose,Create(PrevSol CurDepth+1 UseCopy Information
			  AllocateCopy ?NewNode)
	    isDirty <- true
	    kids    <- {Append @kids [NewNode]}
	 end
	 
	 meth NextKids(Break PrevSol CurDepth InfoDist
		       CurSearchDist SearchDist CurNs CurCopy Ks N $)
	    case Ks of nil then false
	    [] K|Kr then 
	       case K.kind==choose then
		  SolBelow IsDirtyBelow DecChoices
	       in
		  {K Next(Break PrevSol CurDepth InfoDist
			  CurSearchDist SearchDist N|CurNs CurCopy
			  ?SolBelow ?IsDirtyBelow ?DecChoices)}
		  case DecChoices   then choices  <- @choices-1  else skip end
		  case IsDirtyBelow then isDirty  <- true        else skip end
		  case SolBelow of false then
		     Choose,NextKids(Break PrevSol CurDepth InfoDist
				     CurSearchDist SearchDist CurNs CurCopy
				     Kr N+1 $)
		  else
		     isSolBelow <- true
		     SolBelow
		  end
	       else
		  Choose,NextKids(Break PrevSol CurDepth InfoDist
				  CurSearchDist SearchDist CurNs CurCopy
				  Kr N+1 $)
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
	       Choose,Add(PrevSol CurDepth InfoDist CurSearchDist
			  CurNs CurCopy
			  ?NextDist ?NextNs ?NextCopy
			  ?Information ?NewNode)
	       case Information
	       of succeeded(_) then
		  Sol         = NewNode
		  IsDirty     = true
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
		  case SolBelow of false then
		     Choose,NextLocal(Break PrevSol CurDepth InfoDist
				      CurSearchDist SearchDist
				      CurNs CurCopy
				      ?Sol ?IsDirty
				      ?DecChoices)
		  else
		     isSolBelow <- true
		     Sol         = SolBelow
		     IsDirty     = true
		     DecChoices  = @choices==0
		  end
	       else
		  Choose,NextLocal(Break PrevSol CurDepth InfoDist
				   CurSearchDist SearchDist
				   CurNs CurCopy
				   ?Sol ?IsDirty
				   ?DecChoices)
	       end
	    else
	       %% Oh, we are doomed: no sols at all
	       Sol         = false
	       IsDirty     = @isDirty
	       DecChoices  = @choices==0
	       case DecChoices then
		  case @isSolBelow then skip
		  elsecase @copy of flushable(_) then
		     case {Dictionary.get self.manager.options.search failed}
		     then copy <- false
		     else skip
		     end
		  else skip
		  end
	       else skip
	       end
	    end
	 end
	 
	 meth Next(Break PrevSol CurDepth InfoDist
		   CurSearchDist SearchDist
		   CurNs CurCopy
		   ?Sol ?IsDirty ?DecChoices)
	    %% Check the existing kids
	    case @isHidden orelse @choices==0 then
	       %% In this node and below everything is done
	       Sol         = false
	       IsDirty     = false
	       DecChoices  = false
	    else
	       %% Okay there are choices left. So first check whether the 
	       %% already created kids can contribute to a solution.
	       NextNs NextDist NextCopy
	    in
	       case @copy of false then
		  NextDist = CurSearchDist
		  NextCopy = CurCopy
		  NextNs   = CurNs
	       elseof Copy then
		  NextDist = SearchDist
		  NextCopy = Copy.1
		  NextNs   = nil
	       end
	       case Choose,NextKids(Break PrevSol CurDepth+1 InfoDist
				    NextDist-1 SearchDist NextNs NextCopy
				    @kids 1 $)
	       of false then
		  %% Now we have to create new kids to find whether a solution
		  %% does exists there.
		  Choose,NextLocal(Break PrevSol CurDepth InfoDist
				   NextDist SearchDist NextNs NextCopy
				   ?Sol ?IsDirty ?DecChoices)
	       elseof S then
		  %% Okay one of our kids found a solution.
		  Sol         = S
		  IsDirty     = true
		  DecChoices  = @choices==0
	       end
	    end
	 end
	 
	 meth next(Break PrevSol SearchDist InfoDist ?Sol)
	    CurDepth CurNs CurSearchDist CurCopy
	 in
	    Choose,findDepthAndCopy(?CurDepth ?CurSearchDist
				    ?CurNs ?CurCopy)
	    case @choices==0 then Sol=false
	    else IsDirty DecChoices in
	       Choose,Next(Break PrevSol CurDepth InfoDist
			   SearchDist-CurSearchDist SearchDist
			   CurNs CurCopy
			   ?Sol ?IsDirty ?DecChoices)
	       {self.mom leaveNode(Sol\=false IsDirty DecChoices)}
	    end
	 end
	 
	 meth step(PrevSol InfoDist ?Sol)
	    case @toDo==nil then Sol=false
	    else
	       Info NewNode CurDepth CurNs CurSearchDist CurCopy
	    in
	       Choose,findDepthAndCopy(?CurDepth ?CurSearchDist
				       ?CurNs ?CurCopy)
	       Choose,Add(PrevSol CurDepth InfoDist CurSearchDist
			  CurNs CurCopy
			  _ _ _
			  ?Info ?NewNode)
	       Sol = case {Label Info}==succeeded then NewNode else false end
	       {self.mom leaveNode(Sol\=false true @choices==0)}
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
      
   class Leaf
      meth isFinished($)
	 true
      end
      meth hasSolutions($)
	 false
      end
      meth isNextPossible($)
	 false
      end
      meth isStepPossible($)
	 false
      end
   end

   class Succeeded from Leaf
      attr copy: false
      meth hasSolutions($)
	 true
      end
      meth findSpace($)
	 case @copy of false then {self.mom FindSpace(self $)}
	 elseof TaggedCopy then {Space.clone TaggedCopy.1}
	 end
      end
      meth getOriginalSpace($)
	 @copy.1
      end
      meth next(_ _ _ _ $)
	 false
      end
      meth step(_ _ $)
	 false
      end
   end
      
   class Blocked from Leaf
      meth isFinished($)
	 false
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
			 failed:    Leaf
			 blocked:   Blocked
			 sentinel:  Sentinel)

end
