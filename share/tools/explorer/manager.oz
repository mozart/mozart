%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   \insert shapes-and-images.oz

   \insert menu-manager.oz
   
   \insert nodes.oz

   \insert toplevel-manager.oz

   \insert dialog-manager.oz

   \insert status-manager.oz
   
in

   class Manager
      from
	 UrObject
	 DialogManager
	 MenuManager
	 StatusManager
	 ToplevelManager
      
      feat
	 explorer

      attr
	 IsBAB:   false
	 root:    false
	 query:   false
	 order:   false
	 PrevSol: false
	 ToClose: nil

      
      meth init(EXPLORER)
	 PackAll = ToplevelManager,init($)
      in
	 DialogManager,init
	 MenuManager,init
	 StatusManager,init
	 self.explorer = EXPLORER
	 {PackAll}
	 ToplevelManager,configurePointer(idle)
      end
      
      meth clear
	 Manager,clearDialogs
	 MenuManager,clear
	 StatusManager,clear
	 ToplevelManager,clear
	 root    <- false
	 PrevSol <- false
      end

      meth clearDialogs
	 case @ToClose of nil then skip elseof POs then
	    {ForAll POs
	     proc {$ PO}
		case {IsDet PO} then 
		   case {Object.is PO} then
		      thread {PO close} end
		   elsecase
		      {Procedure.is PO} andthen {Procedure.arity PO}==0
		   then thread {PO} end
		   end
		else skip
		end
	     end}
	    ToClose <- nil
	 end
      end

      meth reset
	 case @query of false then skip elseof Query then
	    Manager,query(Query @order)
	 end
      end

      meth setCursor(CurNode IsVisible <= false)
	 %% Might only be invoked if root is not false
	 MenuManager,normal(explorer([clear postscript reset]))
	 case {@root isFinished($)} then
	    StatusManager,finish
	 else skip end
	 case CurNode.kind==failed orelse CurNode.kind==blocked then
	    %% Can only happen if there is a single failed or blocked node
	    MenuManager,disable([move([top cur]) nodes(stat)])
	 else
	    %% Move: Top, Current, and stat are always possible
	    MenuManager,normal([move([top cur]) nodes(stat)])
	               ,state({CurNode back($)}\=false move(back))
	    %% Move: Previous Solution, Next Solution
	               ,state({CurNode leftMost($)}\=CurNode move(leftMost))
	               ,state({CurNode rightMost($)}\=CurNode move(rightMost))
	    case {@root hasSolutions($)} then
	       MenuManager,state({CurNode nextSol($)}\=false move(nextSol))
	                  ,state({CurNode prevSol($)}\=false move(prevSol))
	    else
	       MenuManager,disable(move([prevSol nextSol]))
	    end
	    %% Search
	    case StatusManager,hasBlocked($) then
	       MenuManager,disable(search([next all step]))
	    else
	       MenuManager,state({CurNode isNextPossible($)}
				 search([next all]))
	                  ,state({CurNode isStepPossible($)} search(step))
	    end
	    %% Nodes
	    MenuManager,state({CurNode isHidable($)} hide(toggle))
	    case {CurNode isHidden($)} then
	       MenuManager,normal(hide([all butfailed]))
	                  ,disable([nodes([info cmp selCmp deselCmp])
				    hide(failed)])
	    else
	       MenuManager,normal(nodes(info))
	                  ,state(@cmpNode==false nodes(selCmp))
	                  ,state(@cmpNode\=false nodes([deselCmp cmp]))
	                  ,state({CurNode isUnhidable($)}     hide(all))
	                  ,state({CurNode isFailedHidable($)} hide(failed))
	                  ,state({CurNode isButFailedUnhidable($)}
				 hide(butfailed))
	                  ,state(CurNode.kind==choose orelse
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

      meth query(Query Order)
	 Manager,clear
	 IsBAB   <- (Order\=false)
	 query   <- Query
	 order   <- Order
	 curNode <- false
	 PrevSol <- false
	 {self.status setBAB(@IsBAB)}
	 StatusManager,start(_)
	 root <- {MakeRoot self Query Order}
	 Manager,prepare
      end

      meth prepare
	 StatusManager,stop
	 Manager,Layout,setCursor(@root)
	 ToplevelManager,configurePointer(idle)
      end
      
      %%
      %% Implementation of ``Move'' functionality
      %%

      meth moveTop
	 Manager,setCursor(@root)
      end

      meth moveCurrent
	 CurNode = @curNode
      in
	 curNode <- false
	 Manager,setCursor(CurNode)
      end
      
      meth moveFrom(What)
	 case {@curNode What($)} of false then skip elseof Dest then
	    Manager,setCursor(Dest)
	 end
      end

      meth idle
	 MenuManager,idle
	 ToplevelManager,configurePointer(idle)
      end
      
      %%
      %% Implementation of ``Search'' functionality
      %%

      meth getPrevSol($)
	 case @PrevSol of false then false
	 elseof Sol then {Sol getOriginalSpace($)}
	 end
      end
      
      meth startSearch($ <= _)
	 MenuManager,busy
	            ,normal(explorer([halt break]))
	 ToplevelManager,configurePointer(searching)
	 StatusManager,start($)
      end

      meth stopSearch(Sol Cursor <= false)
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
	           ,setCursor({TryCursor getOverHidden(TryCursor $)})
	    MenuManager,disable(explorer(halt))
	    Manager,idle
	 end
      end
      
      meth next
	 CurNode = @curNode
	 Break   = Manager,startSearch($)
	 O       = self.options.search
      in
	 Manager,stopSearch({CurNode
			     next(Break Manager,getPrevSol($)
				  {Dictionary.get O search}
				  {Dictionary.get O information} $)})
      end
      
      meth all
	 Manager,startSearch
	        ,DoAll({Dictionary.get self.options.drawing update})
      end

      meth DoAll(NoSol)
	 Break   = StatusManager,getBreakFlag($)
	 CurNode = @curNode
	 O       = self.options.search
	 Sol     = {CurNode next(Break Manager,getPrevSol($)
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
	              ,LayoutAfterSearch
	       StatusManager,unbreak
	                    ,startTime
	       Manager,DoAll({Dictionary.get self.options.drawing update})
	    else Manager,DoAll(NoSol-1)
	    end
	 else Manager,stopSearch(Sol)
	 end
      end

      meth step
	 CurNode = @curNode
      in
	 Manager,startSearch(_)
	        ,stopSearch({CurNode
			     step(Manager,getPrevSol($)
				  {Dictionary.get self.options.search
				   information} $)})
      end

      meth nodes(ToDo)
	 MenuManager,busy
	            ,normal(explorer(break))
	 StatusManager,clearBreak
	 Manager,hideCursor
	 {@curNode ToDo}
	 Manager,Layout
	        ,setCursor(@curNode false)
	        ,idle
      end
      
      meth stat
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

      meth setByXY(X Y)
	 Manager,setCursor(ToplevelManager,findByXY(X Y $) true)
      end
      
      meth nodesByXY(What X Y)
	 Manager,setCursor(ToplevelManager,findByXY(X Y $) false)
	        ,nodes(What)
      end
      
      meth doByXY(What X Y)
	 Manager,setCursor(ToplevelManager,findByXY(X Y $) false)
	        ,What
      end
      
      meth selInfo(X Y)
	 Node = ToplevelManager,findByXY(X Y $)
      in
	 case {Node isHidden($)} then skip else
	    Manager,nodesInfo(Node)
	 end
      end
      
      meth nodesInfo(Node <= false)
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
	    Manager,setCursor(CurNode)
	           ,idle
	 end
      end

      meth nodesSelCmp
	 CurNode = @curNode
      in
	 cmpNode <- CurNode
	 Manager,getNumber(CurNode _)
	 MenuManager,disable(nodes(selCmp))
	            ,normal(nodes(deselCmp))
	 Manager,setCursor(CurNode)
      end

      meth nodesDeselCmp
	 cmpNode <- false
	 MenuManager,disable(nodes([deselCmp cmp]))
	 Manager,setCursor(@curNode)
      end
      
      meth nodesCmp
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
	    Manager,setCursor(CurNode)
	           ,idle
	 end
      end	    

      meth wake(Node KillId)
	 case {self.status getKill(_ $)}==KillId then
	    case Node.mom of !Sentinel then
	       Manager,reset
	    elseof Mom then
	       {Mom  removeLast(Manager,getPrevSol($))}
	       {Node close}
	       {self.status removeBlocked}
	       curNode <- Mom
	       Manager,step
	    end
	 else skip
	 end
      end

      meth updateAfterOption
	 case {Dictionary.get self.options.drawing scale} then
	    ToplevelManager,scaleToFit
	 else skip
	 end
      end
      
      meth guiOptions(What)
	 MenuManager,busy
	 DialogManager,guiOptions(What)
	 Manager,updateAfterOption
	        ,idle
	 case @curNode\=false then
	    Manager,setCursor(@curNode false)
	 else skip
	 end
      end
      
      meth postscript
	 Manager,hideCursor
	 ToplevelManager,hideNumbers
	 MenuManager,busy
	 DialogManager,postscript
	 ToplevelManager,unhideNumbers
	 Manager,idle
	 case @curNode\=false then
	    Manager,setCursor(@curNode false)
	 else skip
	 end
      end
      
      meth close
	 UrObject,close
	 {self.explorer ManagerClosed}
	 case @root of false then skip elseof Root then
	    {Root close}
	 end
	 Manager,clearDialogs
	 ToplevelManager,close
      end

   end


end


