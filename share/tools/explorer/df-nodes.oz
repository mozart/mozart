%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   class ChoiceNode

      meth CreateNode(Control CurDepth CurDist $)
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
		  case CurDepth mod self.infoDist==1 then persistent
		  elsecase CurDist==0 then transient
		  else False
		  end
		  Control)}
	 [] unstable(_) then
	    choices <- @choices + 1
	    {self.canvas.manager.status halt}
	    {New self.classes.unstable
	     init(self CurDepth CurDist Control)}
	 end
      end

      meth wake(Node CurDepth CurDist Control)
	 choices  <- @choices - 1
	 isDirty  <- True
	 <<replaceKid(Node <<CreateNode(Control CurDepth CurDist $)>>)>>
	 {Node close}
	 case self.mom of !False then true elseof Mom then
	    {Mom leaveNode({Label Control}==solved True @choices==0)}
	 end
      end
	    
      meth NextKids(CurDepth MaxDepth
		    CurNs CurDist MaxDist CurCopy Ks N $)
	 case Ks of nil then False
	 [] K|Kr then 
	    case K.kind==choice then
	       SolBelow IsDirtyBelow DecChoices
	    in
	       {K Next(CurDepth MaxDepth
		       N|CurNs CurDist MaxDist CurCopy
		       ?SolBelow
		       ?IsDirtyBelow
		       ?DecChoices)}
	       case DecChoices   then choices  <- @choices-1  else true end
	       case IsDirtyBelow then isDirty  <- True        else true end
	       case SolBelow of !False then
		  <<ChoiceNode NextKids(CurDepth MaxDepth
					CurNs CurDist MaxDist CurCopy
					Kr N+1 $)>>
	       else
		  isSolBelow <- True
		  SolBelow
	       end
	    else
	       <<ChoiceNode NextKids(CurDepth MaxDepth
				     CurNs CurDist MaxDist CurCopy
				     Kr N+1 $)>>
	    end  
	 end
      end

      meth NextLocal(CurDepth MaxDepth
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
	 in
	    case @toDo
	    of P#1#MaxAlt then
	       toDo          <- 2#MaxAlt
	       UseNs         = [1]
	       CopyToUse     = P
	       NextDist      = CurDist - 1
	       NextNs        = 1|CurNs
	    [] NextAlt#MaxAlt then
	       case NextAlt==MaxAlt then
		  toDo         <- nil
		  choices      <- @choices  - 1
		  case (@choices==0 andthen @copy\=False) then
		     NextDist  = 0 % force allocation of a copy below!
		     CopyToUse = case {Label @copy}
				 of transient then
				    copy <- False
				    CurCopy
				 [] persistent then
				    {Procedure.clone CurCopy}
				 end
		  else
		     CopyToUse = {Procedure.clone CurCopy}
		     NextDist  = CurDist - 1
		  end
	       else
		  toDo         <- NextAlt+1#MaxAlt
		  CopyToUse     = {Procedure.clone CurCopy}
		  NextDist      = CurDist - 1
	       end
	       UseNs = NextNs = NextAlt|CurNs
	    end
	    Information = {self.solve CopyToUse {Reverse UseNs}}
	    <<CreateNode(Information CurDepth+1 NextDist ?NewNode)>> 
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
	       {NewNode Next(CurDepth+1 MaxDepth
			     NextNs NextDist MaxDist CurCopy
			     ?SolBelow _
			     ?DecChoicesBelow)}
	       case DecChoicesBelow  then choices <- @choices - 1
	       else true end
	       case SolBelow of !False then
		  <<ChoiceNode NextLocal(CurDepth MaxDepth
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
	       <<ChoiceNode NextLocal(CurDepth MaxDepth
				      CurNs CurDist MaxDist CurCopy
				      ?Sol ?IsDirty
				      ?DecChoices)>>
	    end
	 end
      end
      
      meth Next(CurDepth MaxDepth
		CurNs CurDist MaxDist CurCopy
		?Sol ?IsDirty ?DecChoices)
	 %% Check the existing kids
	 case CurDepth==MaxDepth orelse @isHidden orelse @choices==0 then
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
	    case <<ChoiceNode NextKids(CurDepth+1 MaxDepth
				       NextNs NextDist-1 MaxDist NextCopy
				       @kids 1 $)>>
	    of !False then
	       %% Now we have to create new kids to find whether a solution
	       %% does exists there.
	       <<ChoiceNode NextLocal(CurDepth MaxDepth
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

      meth next(MaxDepth MaxDist ?Sol)
	 CurDepth CurNs CurDist CurCopy
      in
	 <<findDepthAndCopy(?CurDepth ?CurNs ?CurDist ?CurCopy)>>
	 case CurDepth==MaxDepth orelse @choices==0 then Sol=False
	 else IsDirty DecChoices in
	    <<ChoiceNode Next(CurDepth
			      case MaxDepth
			      of ~1 then ~1
			      [] !False then CurDepth+1
			      else CurDepth+MaxDepth+1
			      end
			      CurNs MaxDist-CurDist MaxDist
			      CurCopy
			      ?Sol
			      ?IsDirty ?DecChoices )>>
	    case self.mom of !False then true elseof Mom then
	       {Mom leaveNode(Sol\=False IsDirty DecChoices)}
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
      meth next(_ _ $)
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
