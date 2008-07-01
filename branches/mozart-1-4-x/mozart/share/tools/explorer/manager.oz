%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local

   \insert shapes-and-images.oz

   \insert statistics-balloon.oz
   
   \insert menu-manager.oz
   
   \insert nodes.oz

   \insert toplevel-manager.oz

   \insert dialog-manager.oz

   \insert status-manager.oz

in

   class Manager
      from
	 BaseObject
	 DialogManager
	 MenuManager
	 StatusManager
	 ToplevelManager
      prop
	 locking
	 final
      
      feat
	 explorer options

      attr
	 IsBAB:   false
	 root:    false
	 script:  false
	 order:   false
	 PrevSol: false
	 ToClose: nil
	 Resume:  false

      
      meth init(EXPLORER Options)
	 lock
	    self.options  = Options
	    PackAll = ToplevelManager,init($)
	 in
	    MenuManager,init
	    StatusManager,init
	    self.explorer = EXPLORER
	    {PackAll}
	    ToplevelManager,configurePointer(idle)
	 end
      end

      meth clear
	 lock
	    Manager,ClearDialogs
	    MenuManager,clear
	    StatusManager,clear
	    ToplevelManager,clear
	    root    <- false
	    PrevSol <- false
	    Resume  <- false
	 end
      end

      meth ClearDialogs
	 {ForAll @ToClose
	  proc {$ A}
	     thread
		case A of O#M then {O M} else {A} end
	     end
	  end}
	 ToClose <- nil
      end

      meth reset(AwaitStable <= false)
	 lock
	    case @script of false then skip elseof Script then
	       Manager,script(Script @order 'skip' AwaitStable)
	    end
	 end
      end

      meth SetCursor(CurNode IsVisible <= false)
	 %% Might only be invoked if root is not false
	 MenuManager,normal(explorer([postscript reset]))
	 if {@root isFinished($)} then
	    StatusManager,finish
	 end
	 if CurNode.kind==failed orelse CurNode.kind==suspended then
	    %% Can only happen if there is a single failed or suspended node
	    MenuManager,disable([move([top cur]) nodes(stat)])
	 else
	    %% Move: Top, Current, and stat are always possible
	    MenuManager,normal([move([top cur]) nodes(stat)])
	    MenuManager,state({CurNode back($)}\=false move(back))
	    %% Move: Previous Solution, Next Solution
	    MenuManager,state({CurNode leftMost($)}\=CurNode move(leftMost))
	    MenuManager,state({CurNode rightMost($)}\=CurNode move(rightMost))
	    if {@root hasSolutions($)} then
	       MenuManager,state({CurNode nextSol($)}\=false move(nextSol))
	       MenuManager,state({CurNode prevSol($)}\=false move(prevSol))
	    else
	       MenuManager,disable(move([prevSol nextSol]))
	    end
	    %% Search
	    if StatusManager,hasSuspended($) then
	       MenuManager,disable(search([next all step]))
	    else
	       MenuManager,state({CurNode isNextPossible($)}
				 search([next all]))
	       MenuManager,state({CurNode isStepPossible($)} search(step))
	    end
	    %% Nodes
	    MenuManager,state({CurNode isHidable($)} hide(toggle))
	    if {CurNode isHidden($)} then
	       MenuManager,normal(hide([all butfailed]))
	       MenuManager,disable([nodes([info cmp selCmp deselCmp])
				    hide(failed)])
	    else
	       MenuManager,normal(nodes(info))
	       MenuManager,state(@cmpNode==false nodes(selCmp))
	       MenuManager,state(@cmpNode\=false nodes([deselCmp cmp]))
	       MenuManager,state({CurNode isUnhidable($)}     hide(all))
	       MenuManager,state({CurNode isFailedHidable($)} hide(failed))
	       MenuManager,state({CurNode isButFailedUnhidable($)}
				 hide(butfailed))
	       MenuManager,state(CurNode.kind==choose orelse
				 CurNode.kind==succeeded
				 nodes(stat))
	    end
	    %% Bring cursor to front
	    ToplevelManager,setCursor(CurNode IsVisible)
	 end
      end

      meth Layout
	 Scale = @scale
	 Font  = @curFont
	 Root  = @root
      in
	 ToplevelManager,configurePointer(drawing)
	 case StatusManager,getBreakStatus($)
	 of kill then skip
	 [] break then
	    {Root hideUndrawn}
	    {Root layout(_ Scale Font)}
	    ToplevelManager,refreshNumbers
	    StatusManager,getBrokenNodes(_)
	 else
	    StatusManager,unbreak
	    {Root layout(StatusManager,getBreakFlag($) Scale Font)}
	    case StatusManager,getBreakStatus($)
	    of break then
	       ToplevelManager,makeDirty(StatusManager,getBrokenNodes($))
	       {Root layout(_ Scale Font)}
	    else skip
	    end
	    ToplevelManager,refreshNumbers
	 end
	 if {Dictionary.get self.options.drawing scale} then
	    ToplevelManager,scaleToFit
	 end
      end

      meth LayoutAfterSearch
	 if {Dictionary.get self.options.drawing hide} then
	    {@curNode hideFailed}
	 end
	 Manager,Layout
      end

      meth script(Script Order Action AwaitStable <= false)
	 lock
	    Manager,clear
	    IsBAB   <- (Order\=false)
	    script  <- Script
	    order   <- Order
	    curNode <- false
	    PrevSol <- false
	    {self.status setBAB(@IsBAB)}
	    StatusManager,start(_)
	    root <- {MakeRoot self Script Order AwaitStable}
	    StatusManager,stop
	    Manager,Layout
	    Manager,SetCursor(@root)
	    ToplevelManager,configurePointer(idle)
	    if Action\='skip' then
	       Resume <- resume(node:@root action:Action)
	       if @root\=false andthen @root.kind==choose then
		  Manager,Action
	       end
	    end
	 end
      end

      
      %%
      %% Implementation of ``Move'' functionality
      %%

      meth moveTop
	 lock
	    Manager,SetCursor(@root)
	 end
      end

      meth moveCurrent
	 lock
	    CurNode = @curNode
	 in
	    curNode <- false
	    Manager,SetCursor(CurNode)
	 end
      end
      
      meth moveFrom(What)
	 lock
	    case {@curNode What($)} of false then skip elseof Dest then
	       Manager,SetCursor(Dest)
	    end
	 end
      end

      meth Idle
	 MenuManager,idle
	 ToplevelManager,configurePointer(idle)
      end
      
      %%
      %% Implementation of ``Search'' functionality
      %%

      meth GetPrevSol($)
	 case @PrevSol of false then false
	 elseof Sol then {Sol getOriginalSpace($)}
	 end
      end
      
      meth StartSearch($ <= _)
	 MenuManager,busy
	 MenuManager,normal(explorer([halt break]))
	 ToplevelManager,configurePointer(searching)
	 StatusManager,start($)
      end

      meth StopSearch(Sol Cursor <= false)
	 if @root\=nil then
	    TryCursor = if Cursor==false then
			   if Sol==false then @curNode
			   else
			      if @IsBAB then PrevSol <- Sol
			      end
			      Sol
			   end
			else Cursor
			end
	 in
	    StatusManager,stop
	    ToplevelManager,hideCursor
	    Manager,LayoutAfterSearch
	    Manager,SetCursor({TryCursor getOverHidden(TryCursor $)})
	    MenuManager,disable(explorer(halt))
	    Manager,Idle
	 end
      end
      
      meth next
	 lock
	    CurNode = @curNode
	    Break   = Manager,StartSearch($)
	    O       = self.options.search
	 in
	    Resume <- resume(node:CurNode action:next)
	    Manager,StopSearch({CurNode
				next(Break Manager,GetPrevSol($)
				     {Dictionary.get O search}
				     {Dictionary.get O information} $)})
	 end
      end
      
      meth all
	 lock
	    Resume <- resume(node:@curNode action:all)
	    Manager,StartSearch
	    Manager,DoAll({Dictionary.get self.options.drawing update})
	 end
      end

      meth DoAll(NoSol)
	 Break   = StatusManager,getBreakFlag($)
	 CurNode = @curNode
	 O       = self.options.search
	 Sol     = {CurNode next(Break Manager,GetPrevSol($)
				 {Dictionary.get O search}
				 {Dictionary.get O information} $)}
      in
	 if Sol\=false andthen StatusManager,getBreakStatus($)==none then
	    if @IsBAB then
	       PrevSol <- Sol
	    end
	    if NoSol==1 then
	       StatusManager,stop
	       Manager,hideCursor
	       Manager,LayoutAfterSearch
	       StatusManager,unbreak
	       StatusManager,startTime
	       Manager,DoAll({Dictionary.get self.options.drawing update})
	    else
	       Manager,DoAll(NoSol-1)
	    end
	 else
	    Manager,StopSearch(Sol)
	 end
      end

      meth step
	 lock
	    CurNode = @curNode
	 in
	    Resume <- false
	    Manager,StartSearch(_)
	    Manager,StopSearch({CurNode
				step(Manager,GetPrevSol($)
				     {Dictionary.get self.options.search
				      information} $)})
	 end
      end

      meth nodes(ToDo)
	 lock
	    MenuManager,busy
	    MenuManager,normal(explorer(break))
	    StatusManager,clearBreak
	    Manager,hideCursor
	    {@curNode ToDo}
	    Manager,Layout
	    Manager,SetCursor(@curNode false)
	    Manager,Idle
	 end
      end
      
      meth stat
	 lock
	    StatNode = case @curNode
		       of false then @root
		       elseof CurNode then CurNode
		       end
	 in
	    if StatNode\=false then
	       Number  = Manager,getNumber(StatNode $)
	       Handler = {self.statAction get($)}.3
	       Stat    = {StatNode stat($)}
	    in
	       case {Procedure.arity Handler}
	       of 2 then thread {Handler Number Stat} end
	       [] 3 then CloseAction in
		  ToClose <- CloseAction|@ToClose
		  thread {Handler Number Stat ?CloseAction} end
	       end
	    end
	 end
      end

      meth setByXY(X Y)
	 lock
	    Manager,SetCursor(ToplevelManager,findByXY(X Y $) true)
	 end
      end
      
      meth nodesByXY(What X Y)
	 lock
	    Manager,SetCursor(ToplevelManager,findByXY(X Y $) false)
	    Manager,nodes(What)
	 end
      end
      
      meth doByXY(What X Y)
	 lock
	    Manager,SetCursor(ToplevelManager,findByXY(X Y $) false)
	    Manager,What
	 end
      end

      meth getStatisticsByXY(X Y $)
	 lock
	    {ToplevelManager,findByXY(X Y $) stat($)}
	 end
      end

      meth selInfo(X Y)
	 lock
	    Node = ToplevelManager,findByXY(X Y $)
	 in
	    if {Node isHidden($)} then skip else
	       Manager,nodesInfo(Node)
	    end
	 end
      end
      
      meth nodesInfo(Node <= false)
	 lock
	    RealNode = if Node==false then @curNode else Node end
	 in
	    case RealNode of false then skip elseof CurNode then
	       MenuManager,busy
	       Action  = {self.infoAction get($)}
	       Handler = Action.3
	       Cast    = Action.4
	       Number  = Manager,getNumber(RealNode $)
	       Info    = {RealNode findSpace($)}
	    in
	       if Info==false then
		  DialogManager,error('Recomputation of information failed.')
	       else
		  case {Procedure.arity Handler}
		  of 2 then thread {Handler Number {Cast Info}} end
		  [] 3 then CloseAction in
		     ToClose <- CloseAction|@ToClose
		     thread {Handler Number {Cast Info} ?CloseAction} end
		  end
	       end
	       Manager,SetCursor(CurNode)
	       Manager,Idle
	    end
	 end
      end

      meth nodesSelCmp
	 lock
	    CurNode = @curNode
	 in
	    cmpNode <- CurNode
	    Manager,getNumber(CurNode _)
	    MenuManager,disable(nodes(selCmp))
	    MenuManager,normal(nodes(deselCmp))
	    Manager,SetCursor(CurNode)
	 end
      end

      meth nodesDeselCmp
	 lock
	    cmpNode <- false
	    MenuManager,disable(nodes([deselCmp cmp]))
	    Manager,SetCursor(@curNode)
	 end
      end
      
      meth nodesCmp
	 lock
	    CmpNode = @cmpNode
	    CurNode = @curNode
	 in
	    if CurNode\=CmpNode then
	       MenuManager,busy
	       CurNumber = ToplevelManager,getNumber(CurNode $)
	       CmpNumber = ToplevelManager,getNumber(CmpNode $)
	       Action    = {self.cmpAction get($)}
	       Handler   = Action.3
	       Cast      = Action.4
	       CurInfo   = {CurNode findSpace($)}
	       CmpInfo   = {CmpNode findSpace($)}
	    in
	       if CmpInfo==false orelse CurInfo==false then
		  DialogManager,error('Recomputation of information failed.')
	       else
		  case {Procedure.arity Handler}
		  of 4 then
		     thread
			{Handler CmpNumber {Cast CmpInfo} CurNumber {Cast CurInfo}}
		     end
		  [] 5 then CloseAction in
		     ToClose <- CloseAction|@ToClose
		     thread
			{Handler CmpNumber {Cast CmpInfo} CurNumber {Cast CurInfo}
			 ?CloseAction}
		     end
		  end
	       end
	       Manager,SetCursor(CurNode)
	       Manager,Idle
	    end
	 end
      end	    

      meth wake(Node KillId)
	 lock
	    if {self.status getKill(_ $)}==KillId then
	       ToResume  = @Resume
	       Mom       = Node.mom
	       CurNode
	    in
	       if Mom.sentinel then
		  Manager,reset(true)
	       else
		  {Mom  removeLast(Manager,GetPrevSol($))}
		  {Node deleteTree}
		  {self.status removeSuspended}
		  curNode <- Mom
		  Manager,step
	       end
	       CurNode = @curNode
	       if ToResume\=false andthen CurNode\=false then
		  StartNode = if ToResume.node==Node then
				 CurNode
			      else
				 ToResume.node
			      end
		  Action    = ToResume.action
	       in
		  if
		     (Action==all orelse
		      CurNode.kind\=succeeded) andthen
		     {StartNode isNextPossible($)}
		  then
		     curNode <- StartNode
		     Manager,Action
		  end
	       end
	    end
	 end
      end

      meth updateAfterOption
	 lock
	    if {Dictionary.get self.options.drawing scale} then
	       ToplevelManager,scaleToFit
	    end
	 end
      end
      
      meth guiOptions(What)
	 lock
	    MenuManager,busy
	    DialogManager,guiOptions(What)
	    Manager,updateAfterOption
	    Manager,Idle
	    if @curNode\=false then
	       Manager,SetCursor(@curNode false)
	    end
	 end
      end
      
      meth postscript
	 lock
	    Manager,hideCursor
	    ToplevelManager,hideNumbers
	    MenuManager,busy
	    DialogManager,postscript
	    ToplevelManager,unhideNumbers
	    Manager,Idle
	    if @curNode\=false then
	       Manager,SetCursor(@curNode false)
	    end
	 end
      end
      
      meth close
	 {self.explorer ManagerClosed}
	 Manager, closeByMain
      end

      meth closeByMain
	 lock
	    Manager,         ClearDialogs
	    ToplevelManager, close
	 end
      end

   end


end


