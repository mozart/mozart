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
	 [] succeeded then True
	 else {N NoChoicesLeft($)} andthen {CheckDone Nr}
         end
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
	    elsecase {N isStepPossible($)} then True
	    else
	       {NotHiddenChoices {N getKids($)}} orelse
	       {NotHiddenChoices Nr}
	    end
	 end
      end
   end

   class ChooseNode

      meth Create(PrevSol NextDepth Space Control AllocateCopy $)
	 case Control
	 of failed then
	    {New self.classes.failed init(self NextDepth)}
	 [] succeeded(S) then
	    isSolBelow <- True
	    {New self.classes.S init(self NextDepth Space)}
	 [] alternatives(_) then
	    choices  <- @choices + 1
	    {New self.classes.choose
	     init(self NextDepth PrevSol AllocateCopy Space Control)}
	 [] blocked(_) then
	    choices <- @choices + 1
	    {self.canvas.manager.status halt}
	    {New self.classes.blocked
	     init(self PrevSol NextDepth Space Control AllocateCopy)}
	 end
      end

      meth wake(Node PrevSol NextDepth Space Control AllocateCopy)
	 isDirty  <- True
	 choices  <- @choices - 1
	 <<replaceKid(Node
		      <<Create(PrevSol NextDepth
			       Space Control AllocateCopy $)>>)>>
	 {Node close}
	 case self.mom of !False then true elseof Mom then
	    {Mom leaveNode({Label Control}==succeeded True @choices==0)}
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
	    AllocateCopy  = case CurDepth mod InfoDist of 1 then persistent
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
	       AllocateCopy  = case CurDepth mod InfoDist of 1 then persistent
			       elsecase NextDist of 0 then transient
			       else False
			       end
	       NextCopy      = CurCopy
	    else
	       ToMerge = {Space.clone PrevSol}
	    in
	       UseCopy = {Space.clone CurCopy}
	       {Misc.recompute UseCopy case NextAlt==MaxAlt then NextAlt else NextAlt#MaxAlt end|CurNs}
	       %% I'm finished
	       toDo     <- nil
	       choices  <- @choices  - 1
	       case @copy of transient(_) then copy <- False else true end
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
	 {Misc.recompute UseCopy UseNs}
	 Information = {Space.ask UseCopy}
	 <<ChooseNode Create(PrevSol CurDepth+1 UseCopy Information
			     AllocateCopy ?NewNode)>> 
	 isDirty <- True
	 <<addKid(NewNode)>>
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
	       case DecChoices   then choices  <- @choices-1  else true end
	       case IsDirtyBelow then isDirty  <- True        else true end
	       case SolBelow of !False then
		  <<ChooseNode NextKids(Break PrevSol CurDepth InfoDist
					CurSearchDist SearchDist CurNs CurCopy
					Kr N+1 $)>>
	       else
		  isSolBelow <- True
		  SolBelow
	       end
	    else
	       <<ChooseNode NextKids(Break PrevSol CurDepth InfoDist
				     CurSearchDist SearchDist CurNs CurCopy
				     Kr N+1 $)>>
	    end  
	 end
      end

      meth NextLocal(Break PrevSol CurDepth InfoDist
		     CurSearchDist SearchDist CurNs CurCopy
		     ?Sol ?IsDirty ?DecChoices)
	 case @toDo\=nil andthen {System.isVar Break} then
	    NewNode Information
	    NextDist NextNs NextCopy
	 in
	    <<ChooseNode Add(PrevSol CurDepth InfoDist CurSearchDist
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
	       else true end
	       case SolBelow of !False then
		  <<ChooseNode NextLocal(Break PrevSol CurDepth InfoDist
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
	       <<ChooseNode NextLocal(Break PrevSol CurDepth InfoDist
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
	    %% Okay there are choices left. So first check whether the already
	    %% created kids can contribute to a solution.
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
	    case <<ChooseNode NextKids(Break PrevSol CurDepth+1 InfoDist
				       NextDist-1 SearchDist NextNs NextCopy
				       @kids 1 $)>>
	    of !False then
	       %% Now we have to create new kids to find whether a solution
	       %% does exists there.
	       <<ChooseNode NextLocal(Break PrevSol CurDepth InfoDist
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
	 <<findDepthAndCopy(?CurDepth ?CurSearchDist ?CurNs ?CurCopy)>>
	 case @choices==0 then Sol=False
	 else IsDirty DecChoices in
	    <<ChooseNode Next(Break PrevSol CurDepth InfoDist
			      SearchDist-CurSearchDist SearchDist
			      CurNs CurCopy
			      ?Sol ?IsDirty ?DecChoices)>>
	    case self.mom of !False then true elseof Mom then
	       {Mom leaveNode(Sol\=False IsDirty DecChoices)}
	    end
	 end
      end

      meth step(PrevSol InfoDist ?Sol)
	 case @toDo==nil then Sol=False
	 else
      	    Info NewNode CurDepth CurNs CurSearchDist CurCopy
	 in
	    <<findDepthAndCopy(?CurDepth ?CurSearchDist ?CurNs ?CurCopy)>>
	    <<ChooseNode Add(PrevSol CurDepth InfoDist CurSearchDist
			     CurNs CurCopy
			     _ _ _
			     ?Info ?NewNode)>>
	    Sol = case {Label Info}==succeeded then NewNode else False end
	    case self.mom of !False then true elseof Mom then
	       {Mom leaveNode(Sol\=False True @choices==0)}
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
      
   class SucceededNode

      meth next(_ _ _ _ $)
	 False
      end

      meth step(_ _ $)
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
   
   BABNodes = c(choose:    ChooseNode
		succeeded: SucceededNode)

end
