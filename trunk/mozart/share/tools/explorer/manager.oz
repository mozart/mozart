%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
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
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
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

      
      meth init(EXPLORER Options)
	 lock
	    PackAll = ToplevelManager,init($)
	 in
	    MenuManager,init
	    StatusManager,init
	    self.explorer = EXPLORER
	    self.options  = Options
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

      meth reset
	 lock
	    case @script of false then skip elseof Script then
	       Manager,script(Script @order)
	    end
	 end
      end

      meth SetCursor(CurNode IsVisible <= false)
	 %% Might only be invoked if root is not false
	 MenuManager,normal(explorer([postscript reset]))
	 case {@root isFinished($)} then
	    StatusManager,finish
	 else skip end
	 case CurNode.kind==failed orelse CurNode.kind==blocked then
	    %% Can only happen if there is a single failed or blocked node
	    MenuManager,disable([move([top cur]) nodes(stat)])
	 else
	    %% Move: Top, Current, and stat are always possible
	    MenuManager,normal([move([top cur]) nodes(stat)])
	    MenuManager,state({CurNode back($)}\=false move(back))
	    %% Move: Previous Solution, Next Solution
	    MenuManager,state({CurNode leftMost($)}\=CurNode move(leftMost))
	    MenuManager,state({CurNode rightMost($)}\=CurNode move(rightMost))
	    case {@root hasSolutions($)} then
	       MenuManager,state({CurNode nextSol($)}\=false move(nextSol))
	       MenuManager,state({CurNode prevSol($)}\=false move(prevSol))
	    else
	       MenuManager,disable(move([prevSol nextSol]))
	    end
	    %% Search
	    case StatusManager,hasBlocked($) then
	       MenuManager,disable(search([next all step]))
	    else
	       MenuManager,state({CurNode isNextPossible($)}
				 search([next all]))
	       MenuManager,state({CurNode isStepPossible($)} search(step))
	    end
	    %% Nodes
	    MenuManager,state({CurNode isHidable($)} hide(toggle))
	    case {CurNode isHidden($)} then
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
	 case {Dictionary.get self.options.drawing scale} then
	    ToplevelManager,scaleToFit
	 else skip
	 end
      end

      meth LayoutAfterSearch
	 case {Dictionary.get self.options.drawing hide} then
	    {@curNode hideFailed}
	 else skip
	 end
	 Manager,Layout
      end

      meth script(Script Order)
	 lock
	    Manager,clear
	    IsBAB   <- (Order\=false)
	    script  <- Script
	    order   <- Order
	    curNode <- false
	    PrevSol <- false
	    {self.status setBAB(@IsBAB)}
	    StatusManager,start(_)
	    root <- {MakeRoot self Script Order}
	    StatusManager,stop
	    Manager,Layout
	    Manager,SetCursor(@root)
	    ToplevelManager,configurePointer(idle)
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
	 case @root==nil then skip else
	    TryCursor = case Cursor==false then
			   case Sol==false then @curNode
			   else
			      case @IsBAB then PrevSol <- Sol
			      else skip
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
	    Manager,StopSearch({CurNode
				next(Break Manager,GetPrevSol($)
				     {Dictionary.get O search}
				     {Dictionary.get O information} $)})
	 end
      end
      
      meth all
	 lock
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
	 case Sol\=false andthen StatusManager,getBreakStatus($)==none then
	    case @IsBAB then PrevSol <- Sol
	    else skip
	    end
	    case NoSol==1 then
	       StatusManager,stop
	       Manager,hideCursor
	       Manager,LayoutAfterSearch
	       StatusManager,unbreak
	       StatusManager,startTime
	       Manager,DoAll({Dictionary.get self.options.drawing update})
	    else Manager,DoAll(NoSol-1)
	    end
	 else Manager,StopSearch(Sol)
	 end
      end

      meth step
	 lock
	    CurNode = @curNode
	 in
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
	    case StatNode==false then skip else
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
	    case {Node isHidden($)} then skip else
	       Manager,nodesInfo(Node)
	    end
	 end
      end
      
      meth nodesInfo(Node <= false)
	 lock
	    RealNode = case Node==false then @curNode else Node end
	 in
	    case RealNode of false then skip elseof CurNode then
	       MenuManager,busy
	       Action  = {self.infoAction get($)}
	       Handler = Action.3
	       Cast    = Action.4
	       Number  = Manager,getNumber(RealNode $)
	       Info    = {RealNode findSpace($)}
	    in
	       case Info==false then
		  DialogManager,error('Recomputation of information failed.')
	       elsecase {Procedure.arity Handler}
	       of 2 then thread {Handler Number {Cast Info}} end
	       [] 3 then CloseAction in
		  ToClose <- CloseAction|@ToClose
		  thread {Handler Number {Cast Info} ?CloseAction} end
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
	    case CurNode==CmpNode then skip else
	       MenuManager,busy
	       CurNumber = ToplevelManager,getNumber(CurNode $)
	       CmpNumber = ToplevelManager,getNumber(CmpNode $)
	       Action    = {self.cmpAction get($)}
	       Handler   = Action.3
	       Cast      = Action.4
	       CurInfo   = {CurNode findSpace($)}
	       CmpInfo   = {CmpNode findSpace($)}
	    in
	       case CmpInfo==false orelse CurInfo==false then
		  DialogManager,error('Recomputation of information failed.')
	       elsecase {Procedure.arity Handler}
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
	       Manager,SetCursor(CurNode)
	       Manager,Idle
	    end
	 end
      end	    

      meth wake(Node KillId)
	 lock
	    case {self.status getKill(_ $)}==KillId then
	       Mom = Node.mom
	    in
	       case Mom.sentinel then
		  Manager,reset
	       else
		  {Mom  removeLast(Manager,GetPrevSol($))}
		  {Node deleteTree}
		  {self.status removeBlocked}
		  curNode <- Mom
		  Manager,step
	       end
	    else skip
	    end
	 end
      end

      meth updateAfterOption
	 lock
	    case {Dictionary.get self.options.drawing scale} then
	       ToplevelManager,scaleToFit
	    else skip
	    end
	 end
      end
      
      meth guiOptions(What)
	 lock
	    MenuManager,busy
	    DialogManager,guiOptions(What)
	    Manager,updateAfterOption
	    Manager,Idle
	    case @curNode\=false then
	       Manager,SetCursor(@curNode false)
	    else skip
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
	    case @curNode\=false then
	       Manager,SetCursor(@curNode false)
	    else skip
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


