%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   class ChoiceNode

      meth Create(Control CurDepth CurSearchDist InfoDist $)
	 case Control
	 of failed then
	    {New self.classes.failed init(self CurDepth)}
	 [] solved(_ S) then
	    isSolBelow <- True
	    {New self.classes.S init(self CurDepth Control)}
	 [] choice(_ _) then
	    choices  <- @choices + 1
	    {New self.classes.choice
	     init(self CurDepth
		  case CurDepth mod InfoDist of 1 then persistent
		  elsecase CurSearchDist of 0 then transient
		  else False
		  end
		  Control)}
	 [] unstable(_) then
	    choices <- @choices + 1
	    {self.canvas.manager.status halt}
	    {New self.classes.unstable
	     init(self CurDepth CurSearchDist InfoDist Control)}
	 end
      end

      meth wake(Node CurDepth CurSearchDist InfoDist Control)
	 choices  <- @choices - 1
	 isDirty  <- True
	 <<replaceKid(Node
		      <<Create(Control CurDepth CurSearchDist InfoDist $)>>)>>
	 {Node close}
	 case self.mom of !False then true elseof Mom then
	    {Mom leaveNode({Label Control}==solved True @choices==0)}
	 end
      end
      
      meth Add(CurDepth InfoDist CurSearchDist CurNs CurCopy
	       ?NextSearchDist ?NextNs
	       ?Information ?NewNode)
	 UseNs UseCopy
      in
	 case @toDo
	 of P#1#MaxAlt then
	    toDo          <- 2#MaxAlt
	    UseNs          = [1]
	    UseCopy        = P
	    NextSearchDist = CurSearchDist - 1
	    NextNs         = 1|CurNs
	 [] NextAlt#MaxAlt then
	    case NextAlt==MaxAlt then
	       toDo    <- nil
	       choices <- @choices  - 1
	       case (@choices==0 andthen @copy\=False) then
		  NextSearchDist = 0 % force allocation of a copy below!
		  UseCopy        = case {Label @copy}
				   of transient then
				      copy <- False
				      CurCopy
				   [] persistent then
				      {Procedure.clone CurCopy}
				   end
	       else
		  NextSearchDist = CurSearchDist - 1
		  UseCopy        = {Procedure.clone CurCopy}
	       end
	    else
	       toDo          <- NextAlt+1#MaxAlt
	       UseCopy        = {Procedure.clone CurCopy}
	       NextSearchDist = CurSearchDist - 1
	    end
	    UseNs = NextNs = NextAlt|CurNs
	 end
         Information = {Solve UseCopy {Reverse UseNs}}
	 NewNode     = <<ChoiceNode Create(Information CurDepth+1
					   NextSearchDist InfoDist $)>> 
	 isDirty <- True
	 <<addKid(NewNode)>>
      end

      meth NextKids(Break CurDepth InfoDist 
		    CurSearchDist SearchDist
		    CurNs CurCopy
		    Ks N $)
	 case Ks of nil then False
	 [] K|Kr then 
	    case K.kind==choice then
	       SolBelow IsDirtyBelow DecChoices
	    in
	       {K Next(Break CurDepth InfoDist CurSearchDist SearchDist
		       N|CurNs CurCopy
		       ?SolBelow ?IsDirtyBelow ?DecChoices)}
	       case DecChoices   then choices  <- @choices-1  else true end
	       case IsDirtyBelow then isDirty  <- True        else true end
	       case SolBelow of !False then
		  <<ChoiceNode NextKids(Break CurDepth InfoDist
					CurSearchDist SearchDist
					CurNs CurCopy Kr N+1 $)>>
	       else
		  isSolBelow <- True
		  SolBelow
	       end
	    else
	       <<ChoiceNode NextKids(Break CurDepth InfoDist
				     CurSearchDist SearchDist
				     CurNs CurCopy Kr N+1 $)>>
	    end  
	 end
      end

      meth NextLocal(Break CurDepth InfoDist
		     CurSearchDist SearchDist
		     CurNs CurCopy
		     ?Sol ?IsDirty ?DecChoices)
	 case @toDo\=nil andthen {System.isVar Break} then
	    Information NewNode NextSearchDist NextNs
	 in
	    <<ChoiceNode Add(CurDepth InfoDist CurSearchDist CurNs CurCopy
			     ?NextSearchDist ?NextNs
			     ?Information ?NewNode)>>
	    case Information
	    of solved(_ _) then
	       Sol         = NewNode
	       IsDirty     = True
	       DecChoices  = @choices==0
	    [] choice(_ _) then
	       SolBelow DecChoicesBelow
	    in
	       {NewNode Next(Break CurDepth+1 InfoDist
			     NextSearchDist SearchDist
			     NextNs CurCopy
			     ?SolBelow _
			     ?DecChoicesBelow)}
	       case DecChoicesBelow then choices <- @choices - 1
	       else true end
	       case SolBelow of !False then
		  <<ChoiceNode NextLocal(Break CurDepth InfoDist
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
	       <<ChoiceNode NextLocal(Break CurDepth InfoDist
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
      
      meth Next(Break CurDepth InfoDist
		CurSearchDist SearchDist
		CurNs CurCopy
		?Sol ?IsDirty ?DecChoices)
	 %% Check the existing kids
	 case @isHidden orelse @choices==0 orelse {Not {System.isVar Break}} then
	    %% In this node and below everything is done
	    Sol         = False
	    IsDirty     = False
	    DecChoices  = False
	 else
	    %% Okay there are choices left. So first check whether the already
	    %% created kids can contribute to a solution.
	    NextNs NextSearchDist NextCopy
	 in
	    case @copy of !False then
	       NextSearchDist = CurSearchDist
	       NextCopy       = CurCopy
	       NextNs         = CurNs
	    elseof Copy then
	       NextSearchDist = SearchDist
	       NextCopy       = Copy.1
	       NextNs         = nil
	    end
	    case <<ChoiceNode NextKids(Break CurDepth+1 InfoDist
				       NextSearchDist-1 SearchDist
				       NextNs NextCopy
				       @kids 1 $)>>
	    of !False then
	       %% Now we have to create new kids to find whether a solution
	       %% does exists there.
	       <<ChoiceNode NextLocal(Break CurDepth InfoDist
				      NextSearchDist SearchDist
				      NextNs NextCopy
				      ?Sol ?IsDirty ?DecChoices)>>
	    elseof S then
	       %% Okay one of our kids found a solution.
	       Sol         = S
	       IsDirty     = True
	       DecChoices  = @choices==0
	    end
	 end
      end

      meth next(Break SearchDist InfoDist ?Sol)
	 CurDepth CurNs CurSearchDist CurCopy
      in
	 <<findDepthAndCopy(?CurDepth ?CurSearchDist ?CurNs ?CurCopy)>>
	 case @choices==0 then Sol=False
	 else IsDirty DecChoices in
	    <<ChoiceNode Next(Break CurDepth InfoDist
			      SearchDist-CurSearchDist SearchDist
			      CurNs CurCopy
			      ?Sol ?IsDirty ?DecChoices)>>
	    case self.mom of !False then true elseof Mom then
	       {Mom leaveNode(Sol\=False IsDirty DecChoices)}
	    end
	 end
      end
      
      meth step(InfoDist ?Sol)
	 case @toDo==nil then Sol=False
	 else 
	    CurDepth CurNs CurSearchDist CurCopy Info NewNode
	 in
	    <<findDepthAndCopy(?CurDepth ?CurSearchDist ?CurNs ?CurCopy)>>
	    <<ChoiceNode Add(CurDepth InfoDist CurSearchDist CurNs CurCopy
			     _ _ ?Info ?NewNode)>>
	    Sol = case {Label Info}==solved then NewNode else False end
	    case self.mom of !False then true elseof Mom then
	       {Mom leaveNode(Sol\=False True @choices==0)}
	    end
	 end
      end
      
      meth isNextPossible($)
	 @choices>0
      end

      meth isStepPossible($)
	 @toDo\=nil
      end
   end

   class SolvedNode
      meth next(_ _ _ $)
	 False
      end
      meth step(_ $)
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
   
   DfNodes = c(choice: ChoiceNode
	       solved: SolvedNode)

end
