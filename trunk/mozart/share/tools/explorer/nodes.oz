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


   class BlockedNode
      from
	 BasicNodes.leaf
	 LayoutNodes.leaf
	 HideNodes.leaf
	 MoveNodes.blocked
	 SearchNodes.blocked
	 TkNodes.blocked
	 StatNodes.blocked
	 ActionNodes.blocked
      
      feat
	 kind: blocked
      
   end

   class SucceededNode
      from 
	 BasicNodes.leaf
	 LayoutNodes.leaf
	 MoveNodes.succeeded
	 SearchNodes.succeeded
	 StatNodes.succeeded
	 HideNodes.leaf
	 ActionNodes.succeeded
      
      feat
	 kind: succeeded
      meth close true end
   end

   class ChooseNode
      from
	 BasicNodes.inner
	 HideNodes.inner
	 MoveNodes.choose
	 TkNodes.choose
	 SearchNodes.choose
	 StatNodes.choose
	 LayoutNodes.inner
	 ActionNodes.choose
      
      feat
	 kind: choose
      meth close true end
   end

   fun {UnwrapBlocked UC}
      case UC of blocked(C) then {UnwrapBlocked C} else UC end
   end
   

in

   fun {MakeClasses IsBAB Manager Order}
      Canvas         = Manager.canvas
      Status         = Manager.status
      StrategyNodes  = case IsBAB then BABNodes else DfNodes end
      ChooseFeatures = case IsBAB then
			       class $
				  feat
				     classes:  Classes
				     canvas:   Canvas
				     order:    Order
			       end
			    else
			       class $
				  feat
				     classes:  Classes
				     canvas:   Canvas
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
	blocked:
	   case IsBAB then
	      class $ from BlockedNode
		 feat canvas:Canvas
		 meth init(Mom PrevSol NextDepth Control AllocateCopy)
		    KillFlag KillId UnwrapControl
		 in
		    self.mom = Mom
		    {Manager.killer get(KillFlag KillId)}
		    thread
		       UnwrapControl = {UnwrapBlocked Control}
		    end
		    thread
		       if {Wait UnwrapControl} then
			  {Manager wake(Mom KillId UnwrapControl
					wake(self PrevSol NextDepth
					     UnwrapControl AllocateCopy))}
		       [] {Wait KillFlag} then true
		       end
		    end
		    <<TkNodes.blocked init>>
		    {Status addBlocked(NextDepth)}
		 end
		 meth close
		    <<BasicNodes.leaf close>>
		    <<TkNodes.blocked close>>
		 end
	      end
	   else
	      class $ from BlockedNode
		 feat canvas:Canvas
		 meth init(Mom CurDepth CurDist CurInfo Control)
		    KillFlag KillId UnwrapControl
		 in
		    self.mom = Mom
		    {Manager.killer get(KillFlag KillId)}
		    thread
		       UnwrapControl = {UnwrapBlocked Control}
		    end
		    thread
		       if {Wait UnwrapControl} then
			  {Manager wake(Mom KillId UnwrapControl
					wake(self CurDepth CurDist CurInfo
					     UnwrapControl))}
		       [] {Wait KillFlag} then true
		       end
		    end
		    <<TkNodes.blocked init>>
		    {Status addBlocked(CurDepth)}
		 end
		 meth close
		    <<BasicNodes.leaf close>>
		    <<TkNodes.blocked close>>
		 end
	      end
	   end
	entailed:
	   case IsBAB then
	      class $
		 from
		    SucceededNode
		    TkNodes.entailed
		    StrategyNodes.succeeded
		 feat
		    canvas:Canvas
		 attr
		    solution: False
		 meth init(Mom Depth S)
		    self.mom  = Mom
		    solution <- S
		    <<TkNodes.entailed   init>>
		    {Status addSolution(Depth)}
		 end
		 meth findSpace($)
		    {Space.clone @solution}
		 end
		 meth getSol($)
		    @solution
		 end
		 meth close
		    <<TkNodes.entailed close>>
		 end
	      end
	   else
	      class $
		 from
		    SucceededNode
		    TkNodes.entailed
		    StrategyNodes.succeeded
		 feat
		    canvas:Canvas
		 meth init(Mom Depth _)
		    self.mom  = Mom
		    <<TkNodes.entailed init>>
		    {Status addSolution(Depth)}
		 end
		 meth close
		    <<TkNodes.entailed close>>
		 end
	      end
	   end	   
	suspended:
	   case IsBAB then
	      class $
		 from
		    SucceededNode
		    TkNodes.suspended
		    StrategyNodes.succeeded
		 feat
		    canvas: Canvas
		 attr
		    solution: False
		 meth init(Mom Depth S)
		    self.mom  = Mom
		    solution <- S
		    <<TkNodes.suspended init>>
		    {Status addSolution(Depth)}
		 end
		 meth findSpace($)
		    {Space.clone @solution}
		 end
		 meth getSol($)
		    @solution
		 end
		 meth close
		    <<TkNodes.suspended close>>
		 end
	      end
	   else	
	      class $
		 from
		    SucceededNode
		    TkNodes.suspended
		    StrategyNodes.succeeded
		 feat
		    canvas:Canvas 
		 meth init(Mom Depth _)
		    self.mom  = Mom
		    <<TkNodes.suspended init>>
		    {Status addSolution(Depth)}
		 end
		 meth close
		    <<TkNodes.suspended close>>
		 end
	      end
	   end
	choose:
	   case IsBAB then
	      class $
		 from
		    ChooseNode
		    StrategyNodes.choose
		    ChooseFeatures
		 meth init(Mom Depth PrevSol AllocateCopy S Info)
		    alternatives(MaxAlt) = !Info
		 in
		    self.mom  = Mom
		    copy <- case AllocateCopy
			    of transient then 
			       transient({Space.clone S})
			    [] persistent then
			       persistent({Space.clone S})
			    else False
			    end
		    toDo <- PrevSol#S#1#MaxAlt
		    <<TkNodes.choose init>>
		    {Status addChoose(Depth)}
		 end
		 meth close
		    <<TkNodes.choose close>>
		 end
	      end
	   else
	      class $
		 from
		    ChooseNode
		    StrategyNodes.choose
		    ChooseFeatures
		 meth init(Mom Depth AllocateCopy S Info)
		    alternatives(MaxAlt) = !Info
		 in
		    self.mom  = Mom
		    copy <- case AllocateCopy
			    of transient then 
			       transient({Space.clone S})
			    [] persistent then
			       persistent({Space.clone S})
			    else False
			    end
		    toDo     <- S#1#MaxAlt
		    <<TkNodes.choose init>>
		    {Status addChoose(Depth)}
		 end
		 meth close
		    <<TkNodes.choose close>>
		 end
	      end
	   end
       )
   in
      Classes
   end


   fun {MakeRoot Query IsBAB Classes}
      create FakedRoot
	 from Classes.choose
      end
      S = {Space.newDebug Query}
      A = {Space.ask S}
   in   
      case A
      of failed then
	 {New Classes.failed init(False 1)}
      [] succeeded(SA) then
	 LocalClasses = Classes.SA
      in
	 create $
	    attr solution:False
	    from LocalClasses
	    with init
	    meth init
	       <<LocalClasses init(False 1 S)>>
	       solution <- S
	    end
	    meth getSol($)
	       @solution
	    end
	    meth findSpace($)
	       {Space.clone @solution}
	    end
	 end
      [] alternatives(_) then
	 {New Classes.choose [case IsBAB then 
				 init(False 1 False persistent S A)
			      else
				 init(False 1 persistent S A)
			      end sync($)] _}
      [] blocked then
	 {New Classes.blocked [case IsBAB then
				  init(False False 1 S A False)
			       else
				  init(False 1 0 1 S A)
			       end sync($)] _}
      end
   end
			
end
