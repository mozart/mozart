%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   \insert basic-nodes.oz

   \insert layout-nodes.oz

   \insert tk-nodes.oz

   \insert search-nodes.oz

   \insert df-nodes.oz

   \insert bab-nodes.oz

   \insert move-nodes.oz

   \insert stat-nodes.oz

   \insert hide-nodes.oz

   \insert action-nodes.oz

   class FailedNode
      from
	 BasicNodes.leaf
	 LayoutNodes.leaf
	 HideNodes.leaf
	 MoveNodes.failed
	 SearchNodes.failed
	 TkNodes.failed
	 StatNodes.failed
	 ActionNodes.failed
      
      feat
	 kind: failed
      
      meth close
	 <<BasicNodes.leaf close>>
	 <<TkNodes.failed close>>
      end
   end


   class UnstableNode
      from
	 BasicNodes.leaf
	 LayoutNodes.leaf
	 HideNodes.leaf
	 MoveNodes.unstable
	 SearchNodes.unstable
	 TkNodes.unstable
	 StatNodes.unstable
	 ActionNodes.unstable
      
      feat
	 kind: unstable
      
   end

   class SolvedNode
      from 
	 BasicNodes.leaf
	 LayoutNodes.leaf
	 MoveNodes.solved
	 SearchNodes.solved
	 StatNodes.solved
	 HideNodes.leaf
	 ActionNodes.solved
      
      feat
	 kind: solved
      meth close true end
   end

   class ChoiceNode
      from
	 BasicNodes.inner
	 HideNodes.inner
	 MoveNodes.choice
	 TkNodes.choice
	 SearchNodes.choice
	 StatNodes.choice
	 LayoutNodes.inner
	 ActionNodes.choice
      
      feat
	 kind: choice
      meth close true end
   end

   fun {UnwrapUnstable UC}
      case UC of unstable(C) then {UnwrapUnstable C} else UC end
   end
   

in

   fun {MakeClasses IsBAB KeepSolutions ChoiceDistance Manager Order}
      Canvas         = Manager.canvas
      Status         = Manager.status
      StrategyNodes  = case IsBAB then BABNodes else DfNodes end
      ChoiceFeatures = case IsBAB then
			       class $
				  feat
				     classes:  Classes
				     canvas:   Canvas
				     order:    Order
				     infoDist: ChoiceDistance
			       end
			    else
			       class $
				  feat
				     classes:  Classes
				     canvas:   Canvas
				     infoDist: ChoiceDistance
			       end
			    end
      Classes =
      c(failed:
	   class $ from FailedNode
	      feat canvas:Canvas
	      meth init(Mom Depth)
		 self.mom = Mom
		 <<TkNodes.failed init>>
		 {Status addFailed(Depth)}
	      end
	   end
	unstable:
	   case IsBAB then
	      class $ from UnstableNode
		 feat canvas:Canvas
		 meth init(Mom PrevSol NextDepth Control AllocateCopy)
		    KillFlag KillId UnwrapControl
		 in
		    self.mom = Mom
		    {Manager.killer get(KillFlag KillId)}
		    thread
		       UnwrapControl = {UnwrapUnstable Control}
		    end
		    thread
		       if {Wait UnwrapControl} then
			  {Manager wake(Mom KillId UnwrapControl
					wake(self PrevSol NextDepth
					     UnwrapControl AllocateCopy))}
		       [] {Wait KillFlag} then true
		       end
		    end
		    <<TkNodes.unstable init>>
		    {Status addUnstable(NextDepth)}
		 end
		 meth close
		    <<BasicNodes.leaf close>>
		    <<TkNodes.unstable close>>
		 end
	      end
	   else
	      class $ from UnstableNode
		 feat canvas:Canvas
		 meth init(Mom CurDepth CurDist Control)
		    KillFlag KillId UnwrapControl
		 in
		    self.mom = Mom
		    {Manager.killer get(KillFlag KillId)}
		    thread
		       UnwrapControl = {UnwrapUnstable Control}
		    end
		    thread
		       if {Wait UnwrapControl} then
			  {Manager wake(Mom KillId UnwrapControl
					wake(self CurDepth CurDist
					     UnwrapControl))}
		       [] {Wait KillFlag} then true
		       end
		    end
		    <<TkNodes.unstable init>>
		    {Status addUnstable(CurDepth)}
		 end
		 meth close
		    <<BasicNodes.leaf close>>
		    <<TkNodes.unstable close>>
		 end
	      end
	   end
	entailed:
	   case IsBAB orelse KeepSolutions then
	      class $
		 from
		    SolvedNode
		    TkNodes.entailed
		    StrategyNodes.solved
		 feat
		    canvas:Canvas
		 attr
		    solution: False
		 meth init(Mom Depth Info)
		    self.mom  = Mom
		    solution <- Info.1
		    <<TkNodes.entailed   init>>
		    {Status addSolution(Depth)}
		 end
		 meth getInfo(?P)
		    P = @solution
		 end
		 meth close
		    <<TkNodes.entailed close>>
		 end
	      end
	   else
	      class $
		 from
		    SolvedNode
		    TkNodes.entailed
		    StrategyNodes.solved
		 feat
		    canvas:Canvas
		 meth init(Mom Depth _)
		    self.mom  = Mom
		    <<TkNodes.entailed   init>>
		    {Status addSolution(Depth)}
		 end
		 meth close
		    <<TkNodes.entailed close>>
		 end
	      end
	   end	   
	stable:
	   case IsBAB orelse KeepSolutions then
	      class $
		 from
		    SolvedNode
		    TkNodes.stable
		    StrategyNodes.solved
		 feat
		    canvas: Canvas
		 attr
		    solution: False
		 meth init(Mom Depth Info)
		    self.mom  = Mom
		    solution <- Info.1
		    <<TkNodes.stable init>>
		    {Status addSolution(Depth)}
		 end
		 meth getInfo(?P)
		    P = @solution
		 end
		 meth close
		    <<TkNodes.stable close>>
		 end
	      end
	   else	
	      class $
		 from
		    SolvedNode
		    TkNodes.stable
		    StrategyNodes.solved
		 feat
		    canvas:Canvas 
		 meth init(Mom Depth _)
		    self.mom  = Mom
		    <<TkNodes.stable init>>
		    {Status addSolution(Depth)}
		 end
		 meth close
		    <<TkNodes.stable close>>
		 end
	      end
	   end
	choice:
	   case IsBAB then
	      class $
		 from
		    ChoiceNode
		    StrategyNodes.choice
		    ChoiceFeatures
		 meth init(Mom Depth PrevSol AllocateCopy Info)
		    choice(P MaxAlt) = !Info
		 in
		    self.mom  = Mom
		    copy <- case AllocateCopy
			    of transient then 
			       transient({Procedure.clone P})
			    [] persistent then
			       persistent({Procedure.clone P})
			    else False
			    end
		    toDo     <- PrevSol#P#1#MaxAlt
		    <<TkNodes.choice init>>
		    {Status addChoice(Depth)}
		 end
		 meth close
		    <<TkNodes.choice close>>
		 end
	      end
	   else
	      class $
		 from
		    ChoiceNode
		    StrategyNodes.choice
		    ChoiceFeatures
		 meth init(Mom Depth AllocateCopy Info)
		    choice(P MaxAlt) = !Info
		 in
		    self.mom  = Mom
		    copy <- case AllocateCopy
			    of transient then 
			       transient({Procedure.clone P})
			    [] persistent then
			       persistent({Procedure.clone P})
			    else False
			    end
		    toDo     <- P#1#MaxAlt
		    <<TkNodes.choice init>>
		    {Status addChoice(Depth)}
		 end
		 meth close
		    <<TkNodes.choice close>>
		 end
	      end
	   end
       )
   in
      Classes
   end


   fun {MakeRoot Query IsBAB Classes}
      create FakedRoot
	 from Classes.choice
      end
      Info = {Solve Query nil}
   in
      
      case {Label Info}
      of failed then
	 {New Classes.failed init(False 1)}
      [] solved then
	 LocalClasses = Classes.(Info.2)
      in
	 create $
	    attr solution:False
	    from LocalClasses
	    with init
	    meth init
	       <<LocalClasses init(False 1 Info)>>
	       solution <- Info.1
	    end
	    meth getInfo(?P)
	       P = @solution
	    end
	 end
      [] choice then
	 {New Classes.choice [case IsBAB then 
				 init(False 1 False persistent Info)
			      else
				 init(False 1 persistent Info)
			      end sync($)] _}
      [] unstable then
	 {New Classes.unstable [case IsBAB then
				   init(False False 1 Info False)
				else
				   init(False 1 0 Info)
				end sync($)] _}
      end
   end
			
end
