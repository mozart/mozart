%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   class EmptyClass
   end

   \insert layout-nodes.oz

   \insert tk-nodes.oz

   \insert search-nodes.oz

   \insert move-nodes.oz

   \insert stat-nodes.oz

   \insert hide-nodes.oz

   \insert action-nodes.oz

   class FailedNode
      from
	 UrObject
	 LayoutNodes.failed
	 HideNodes.failed
	 MoveNodes.failed
	 SearchNodes.failed
	 StatNodes.failed
	 ActionNodes.failed
	 TkNodes.failed
      feat
	 kind: failed
	 mom
      meth init(Mom Depth)
	 self.mom = Mom
	 TkNodes.failed,init
	 {self.status addFailed(Depth)}
      end
   end

   local
      fun {UnwrapBlocked UC}
	 case UC of blocked(C) then {UnwrapBlocked C} else UC end
      end
   in
      class BlockedNode
	 from
	    UrObject
	    LayoutNodes.blocked
	    HideNodes.blocked
	    MoveNodes.blocked
	    SearchNodes.blocked
	    StatNodes.blocked
	    ActionNodes.blocked
	    TkNodes.blocked
	 feat
	    kind: blocked
	    mom
	 
	 meth init(Mom Depth Control)
	    Status         = self.status
	    UnwrapControl  = thread {UnwrapBlocked Control} end
	    KillFlag KillId
	 in
	    self.mom = Mom
	    {Status getKill(?KillFlag ?KillId)}
	    thread
	       {WaitOr UnwrapControl KillFlag}
	       case {IsDet UnwrapControl} then
		  {self.manager wake(self KillId)}
	       else skip
	       end
	    end
	    TkNodes.blocked,init
	    {Status addBlocked(Depth)}
	 end
      end
   end

   local
      class SucceededNode
	 from
	    UrObject
	    LayoutNodes.succeeded
	    MoveNodes.succeeded
	    SearchNodes.succeeded
	    StatNodes.succeeded
	    HideNodes.succeeded
	    ActionNodes.succeeded
	 feat
	    kind: succeeded
	    mom
      end
   in
      class EntailedNode from SucceededNode TkNodes.entailed
	 meth init(Mom Depth S AllocateCopy)
	    self.mom = Mom
	    copy <- case
		       case self.order==False then AllocateCopy
		       else persistent
		       end
		    of transient  then transient(S)
		    [] flushable  then flushable(S)
		    [] persistent then persistent(S)
		    else False
		    end
	    TkNodes.entailed,init
	    {self.status addSolution(Depth)}
	 end
      end

      class SuspendedNode from SucceededNode TkNodes.suspended
	 meth init(Mom Depth S AllocateCopy)
	    self.mom = Mom
	    copy <- case
		       case self.order==False then AllocateCopy
		       else persistent
		       end		       
		    of transient  then transient(S)
		    [] flushable  then flushable(S)
		    [] persistent then persistent(S)
		    else False
		    end
	    TkNodes.suspended,init
	    {self.status addSolution(Depth)}
	 end
      end
   end

   
   class ChooseNode
      from
	 UrObject
	 HideNodes.choose
	 MoveNodes.choose
	 SearchNodes.choose
	 StatNodes.choose
	 LayoutNodes.choose
	 ActionNodes.choose
	 TkNodes.choose
      feat
	 kind: choose
	 mom               % The mom of this node (False if topmost node)
      attr
	 isDirty:    True  % No layout computed
	 kids:       nil   % The list of nodes below
	 toDo:       nil   % What is to be done (nil if nothing)
	 isSolBelow: False % Is there a solution below
	 choices:    1     % unfinished choices below?
	 copy:       False
      
      meth init(Mom Depth PrevSol AllocateCopy S MaxAlt)
	 self.mom  = Mom
	 copy <- case AllocateCopy
		 of transient  then transient({Space.clone S})
		 [] flushable  then flushable({Space.clone S})
		 [] persistent then persistent({Space.clone S})
		 else False
		 end
	 toDo <- PrevSol # S # 1 # MaxAlt
	 TkNodes.choose,init
	 {self.status addChoose(Depth)}
      end
      meth getKids($)
	 @kids
      end
   end


in

   create Sentinel
      from
	 UrObject
	 LayoutNodes.sentinel
	 HideNodes.sentinel
	 MoveNodes.sentinel
	 SearchNodes.sentinel
	 StatNodes.sentinel
	 ActionNodes.sentinel
	 TkNodes.sentinel
   end
      
   fun {MakeRoot Manager Query Order}
      class Features
	 feat
	    classes:  Classes
	    canvas:   Manager.canvas
	    order:    Order
	    status:   Manager.status
	    manager:  Manager
      end
      Classes = c(failed:    class $ from FailedNode    Features end
		  blocked:   class $ from BlockedNode   Features end
		  entailed:  class $ from EntailedNode  Features end
		  suspended: class $ from SuspendedNode Features end
		  choose:    class $ from ChooseNode    Features end)
      S = {Space.new Query}
   in   
      case thread {Space.askVerbose S} end
      of failed then
	 {New Classes.failed init(Sentinel 1)}
      [] succeeded(SA) then
	 {New Classes.SA init(Sentinel 1 S persistent)}
      [] alternatives(MaxAlt) then
	 {New Classes.choose  init(Sentinel 1 False persistent S MaxAlt)}
      [] blocked(Ctrl) then
	 {New Classes.blocked init(Sentinel 1 Ctrl)}
      end
   end
			
end
