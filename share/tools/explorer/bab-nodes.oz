%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   NoChoicesLeft = {NewName}
   
   fun {CheckDone Ns}
      case Ns of nil then True
      [] N|Nr then
	 case N.kind
	 of failed then True
	 [] solved then True
	 else {N NoChoicesLeft($)} andthen {CheckDone Nr}
         end
      end
   end

   fun {NotHiddenChoices Ns}
      case Ns of nil then False
      [] N|Nr then
	 case N.kind
	 of solved then {NotHiddenChoices Nr}
	 [] failed then {NotHiddenChoices Nr}
	 [] unstable then True
	 [] choice then
	    case {N isHidden($)} orelse {N NoChoicesLeft($)} then
	       {NotHiddenChoices Nr}
	    elsecase {N isStepPossible($)} then True
	    else
	       {NotHiddenChoices {N getKids($)}} orelse
	       {NotHiddenChoices Nr}
	    end
	 end
      end
   end

   class ChoiceNode

      meth CreateNode(PrevSol NextDepth Control AllocateCopy $)
	 case Control
	 of failed then
	    {New self.classes.failed init(self NextDepth)}
	 [] solved(_ S) then
	    isSolBelow <- True
	    {New self.classes.S init(self NextDepth Control)}
	 [] choice(_ _) then
	    choices  <- @choices + 1
	    {New self.classes.choice
	     init(self NextDepth PrevSol AllocateCopy Control)}
	 [] unstable(_) then
	    choices <- @choices + 1
	    {self.canvas.manager.status halt}
	    {New self.classes.unstable
	     init(self PrevSol NextDepth Control AllocateCopy)}
	 end
      end

      meth wake(Node PrevSol NextDepth Control AllocateCopy)
	 isDirty  <- True
	 choices  <- @choices - 1
	 <<replaceKid(Node
		      <<CreateNode(PrevSol NextDepth
				   Control AllocateCopy $)>>)>>
	 {Node close}
	 case self.mom of !False then true elseof Mom then
	    {Mom leaveNode({Label Control}==solved True @choices==0)}
	 end
      end

      meth NextKids(PrevSol CurDepth MaxDepth
		    CurNs CurDist MaxDist CurCopy Ks N $)
	 case Ks of nil then False
	 [] K|Kr then 
	    case K.kind==choice then
	       SolBelow IsDirtyBelow DecChoices
	    in
	       {K Next(PrevSol CurDepth MaxDepth
		       N|CurNs CurDist MaxDist CurCopy
		       ?SolBelow
		       ?IsDirtyBelow
		       ?DecChoices)}
	       case DecChoices   then choices  <- @choices-1  else true end
	       case IsDirtyBelow then isDirty  <- True        else true end
	       case SolBelow of !False then
		  <<ChoiceNode NextKids(PrevSol CurDepth MaxDepth
					CurNs CurDist MaxDist CurCopy
					Kr N+1 $)>>
	       else
		  isSolBelow <- True
		  SolBelow
	       end
	    else
	       <<ChoiceNode NextKids(PrevSol CurDepth MaxDepth
				     CurNs CurDist MaxDist CurCopy
				     Kr N+1 $)>>
	    end  
	 end
      end

      meth NextLocal(PrevSol CurDepth MaxDepth
		     CurNs CurDist MaxDist CurCopy
		     ?Sol ?IsDirty ?DecChoices)
	 case @toDo==nil orelse {self.canvas.manager.status isHalted($)} then
	    %% Oh, we are doomed: no sols at all
	    Sol         = False
	    IsDirty     = @isDirty
	    DecChoices  = @choices==0
	 else
	    Information
	    NewNode
	    UseNs CopyToUse
	    NextDist NextNs
	    AllocateCopy NextCopy
	 in
	    case @toDo
	    of Sol#P#1#MaxAlt then
	       toDo          <- Sol#2#MaxAlt
	       UseNs         = [1]
	       CopyToUse     = P
	       NextDist      = CurDist - 1
	       NextNs        = 1|CurNs
	       AllocateCopy  = case CurDepth mod self.infoDist==1 then
				  persistent
			       elsecase NextDist==0 then transient
			       else False
			       end
	       NextCopy      = CurCopy
	    [] Sol#NextAlt#MaxAlt then
	       case PrevSol==Sol then
		  case NextAlt==MaxAlt then
		     toDo         <- nil
		     choices      <- @choices  - 1
		     case @choices==0 andthen @copy\=False then
			NextDist  = 0 % force allocation of a copy below!
			CopyToUse = case @copy of transient(_) then
				       copy <- False
				       CurCopy
				    else {Procedure.clone CurCopy}
				    end
		     else
			CopyToUse = {Procedure.clone CurCopy}
			NextDist  = CurDist - 1
		     end
		  else
		     toDo         <- Sol#NextAlt+1#MaxAlt
		     CopyToUse     = {Procedure.clone CurCopy}
		     NextDist      = CurDist - 1
		  end
		  UseNs = NextNs = NextAlt|CurNs
		  AllocateCopy  = case CurDepth mod self.infoDist==1 then
				     persistent
				  elsecase NextDist==0 then transient
				  else False
				  end
		  NextCopy      = CurCopy
	       else
		  SkipCopy = case {Solve {Procedure.clone CurCopy}
				   {Reverse 
				    case NextAlt==MaxAlt then NextAlt
				    else NextAlt#MaxAlt
				    end|CurNs}}
			     of choice(P _) then P
			     [] solved(P _) then P
			     [] failed      then
				proc {$ X} false end
			     end
	       in
		  %% I'm finished
		  toDo     <- nil
		  choices  <- @choices  - 1
		  case @copy of transient(_) then copy <- False else true end
		  CopyToUse = proc {$ X}
				 {SkipCopy X}
				 {self.order {PrevSol} X}
			      end
		  UseNs     = NextNs = nil
		  NextDist    = 0
		  AllocateCopy = persistent
		  NextCopy = CurCopy
	       end
	    end
	    Information = {Solve CopyToUse {Reverse UseNs}}
	    <<CreateNode(PrevSol CurDepth+1 Information AllocateCopy ?NewNode)>> 
	    isDirty <- True
	    <<addKid(NewNode)>>
	    case Information
	    of solved(_ _) then
	       Sol         = NewNode
	       IsDirty     = True
	       DecChoices  = @choices==0
	    [] choice(_ _) then
	       SolBelow DecChoicesBelow
	    in
	       {NewNode Next(PrevSol CurDepth+1 MaxDepth
			     NextNs NextDist MaxDist NextCopy
			     ?SolBelow _
			     ?DecChoicesBelow)}
	       case DecChoicesBelow  then choices <- @choices - 1
	       else true end
	       case SolBelow of !False then
		  <<ChoiceNode NextLocal(PrevSol CurDepth MaxDepth
					 CurNs CurDist MaxDist CurCopy
					 ?Sol ?IsDirty
					 ?DecChoices)>>
	       else
		  isSolBelow <- True
		  Sol         = SolBelow
		  IsDirty     = True
		  DecChoices  = @choices==0
	       end
	    else
	       <<ChoiceNode NextLocal(PrevSol CurDepth MaxDepth
				      CurNs CurDist MaxDist CurCopy
				      ?Sol ?IsDirty
				      ?DecChoices)>>
	    end
	 end
      end
      
      meth Next(PrevSol CurDepth MaxDepth
		CurNs CurDist MaxDist CurCopy
		?Sol ?IsDirty ?DecChoices)
	 %% Check the existing kids
	 case CurDepth==MaxDepth then
	    {self.canvas.manager.status halt}
	    Sol         = False
	    IsDirty     = False
	    DecChoices  = False
	 elsecase @isHidden orelse @choices==0 then
	    %% In this node and below everything is done
	    Sol         = False
	    IsDirty     = False
	    DecChoices  = False
	 else
	    %% Okay there are choices left. So first check whether the already
	    %% created kids can contribute to a solution.
	    NextNs NextDist NextCopy
	 in
	    case @copy of !False then
	       NextDist = CurDist
	       NextCopy = CurCopy
	       NextNs   = CurNs
	    elseof Copy then
	       NextDist = MaxDist
	       NextCopy = Copy.1
	       NextNs   = nil
	    end
	    case <<ChoiceNode NextKids(PrevSol CurDepth+1 MaxDepth
				       NextNs NextDist-1 MaxDist NextCopy
				       @kids 1 $)>>
	    of !False then
	       %% Now we have to create new kids to find whether a solution
	       %% does exists there.
	       <<ChoiceNode NextLocal(PrevSol CurDepth MaxDepth
				      NextNs NextDist MaxDist NextCopy
				      ?Sol ?IsDirty ?DecChoices)>>
	    elseof S then
	       %% Okay one of our kids found a solution.
	       Sol         = S
	       IsDirty     = True
	       DecChoices  = @choices==0
	    end
	 end
      end

      meth next(PrevSol MaxDepth MaxDist ?Sol)
	 CurDepth CurNs CurDist CurCopy
      in
	 <<findDepthAndCopy(?CurDepth ?CurNs ?CurDist ?CurCopy)>>
	 case CurDepth==MaxDepth then
	    {self.canvas.manager.status halt} Sol=False
	 elsecase @choices==0 then Sol=False
	 else IsDirty DecChoices in
	    <<ChoiceNode Next(PrevSol CurDepth
			      case MaxDepth
			      of ~1 then ~1
			      [] !False then CurDepth+1
			      else CurDepth+MaxDepth+1
			      end
			      CurNs MaxDist-CurDist MaxDist
			      CurCopy
			      ?Sol
			      ?IsDirty ?DecChoices)>>
	    case self.mom of !False then true elseof Mom then
	       {Mom leaveNode(Sol\=False IsDirty DecChoices)}
	    end
	 end
      end
      
      meth !NoChoicesLeft($)
	 @choices==0
      end

      meth isNextPossible($)
	 @choices>=0 andthen
	 (<<isStepPossible($)>> orelse {NotHiddenChoices @kids})
      end

      meth isStepPossible($)
	 @toDo \= nil andthen {Not @isHidden} andthen {CheckDone @kids}
      end

   end
      
   class SolvedNode

      meth next(_ _ _ $)
	 False
      end

      meth isNextPossible($)
	 False
      end

      meth isStepPossible($)
	 False
      end

   end

in
   
   BABNodes = c(choice: ChoiceNode
		solved: SolvedNode)

end
