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
	 IsBAB:        False
	 root:         False
	 query:        False
	 order:        False
	 PrevSol:   False
	 ToClose:      nil

      
      meth init(EXPLORER)
	 PackAll = <<ToplevelManager init($)>>
      in
	 <<DialogManager   init>>
	 <<MenuManager     init>>
	 <<StatusManager   init>>
	 self.explorer       = EXPLORER
	 {PackAll}
	 <<ToplevelManager configurePointer(idle)>>
      end
      
      meth clear
	 <<MenuManager     clear>>
	 <<StatusManager   clear>>
	 <<ToplevelManager clear>>
	 root    <- False
	 PrevSol <- False
      end

      meth clearDialogs
	 case @ToClose of nil then true elseof POs then
	    {ForAll POs
	     proc {$ PO}
		case {System.isVar PO} then true
		elsecase {Object.is PO} then
		   thread {PO close} end
		elsecase
		   {Procedure.is PO} andthen {Procedure.arity PO}==0
		then thread {PO} end
		else true
		end
	     end}
	    ToClose <- nil
	 end
      end

      meth setOptions(O)
	 case {HasSubtreeAt O search} then
	    <<setSearchOptions(O.search)>>
	 else true end
	 case {HasSubtreeAt O information} then
	    <<setInfoOptions(O.information)>>
	 else true end
	 case {HasSubtreeAt O drawing} then
	    <<setLayoutOptions(O.drawing)>>
	 else true end
	 case {HasSubtreeAt O postscript} then
	    <<setPostscriptOptions(O.postscript)>>
	 else true end
      end
      
      meth reset
	 case @query of !False then true elseof Query then
	    <<Manager query(Query @order)>>
	 end
      end

      meth setCursor(CurNode IsVisible <= False)
	 %% Might only be invoked if root is not false
	 <<MenuManager normal(explorer([clear postscript reset]))>>
	 case {@root isFinished($)} then
	    <<StatusManager finish>>
	 else true end
	 case CurNode.kind==failed orelse CurNode.kind==blocked then
	    %% Can only happen if there is a single failed or blocked node
	    <<MenuManager disable([move([top cur])
				   nodes(stat)])>>
	 else
	    %% Move: Top, Current, and stat are always possible
	    <<MenuManager normal([move([top cur]) nodes(stat)])>>
	    <<MenuManager state({CurNode back($)}\=False move(back))>>
	    %% Move: Previous Solution, Next Solution
	    <<MenuManager state({CurNode leftMost($)}\=CurNode
				move(leftMost))>>
	    <<MenuManager state({CurNode rightMost($)}\=CurNode
				move(rightMost))>>
	    case {@root hasSolutions($)} then
	       <<MenuManager state({CurNode nextSol($)}\=False move(nextSol))>>
	       <<MenuManager state({CurNode prevSol($)}\=False move(prevSol))>>
	    else
	       <<MenuManager disable(move([prevSol nextSol]))>>
	    end
	    %% Search
	    case <<StatusManager hasBlocked($)>> then
	       <<MenuManager disable(search([next all step]))>>
	    else
	       <<MenuManager state({CurNode isNextPossible({Not @IsBAB} $)}
				   search([next all]))>>
	       <<MenuManager state({CurNode isStepPossible({Not @IsBAB} $)}
				   search(step))>>
	    end
	    %% Nodes
	    <<MenuManager state({CurNode isHidable($)} hide(toggle))>>
	    case {CurNode isHidden($)} then
	       <<MenuManager  normal(hide([all butfailed]))>>
	       <<MenuManager  disable([nodes([info cmp selCmp deselCmp])
				       hide(failed)])>>
	    else
	       <<MenuManager normal(nodes(info))>>
	       <<MenuManager state(@cmpNode==False nodes(selCmp))>>
	       <<MenuManager state(@cmpNode\=False nodes([deselCmp cmp]))>>
	       <<MenuManager state({CurNode isUnhidable($)}     hide(all))>>
	       <<MenuManager state({CurNode isFailedHidable($)} hide(failed))>>
	       <<MenuManager state({CurNode isButFailedUnhidable($)}
				   hide(butfailed))>>
	       <<MenuManager state(CurNode.kind==choose orelse
				   CurNode.kind==succeeded
				   nodes(stat))>>
	    end
	    %% Bring cursor to front
	    <<ToplevelManager setCursor(CurNode IsVisible)>>
	 end
      end

      meth Layout
	 Scale = @scale
	 Font  = @curFont
	 Root  = @root
      in
	 <<ToplevelManager configurePointer(drawing)>>
	 case <<StatusManager getBreakStatus($)>>
	 of kill then true
	 [] break then
	    {Root hideUndrawn}
	    {Root layout(_ Scale Font)}
	    <<ToplevelManager refreshNumbers>>
	    <<StatusManager getBrokenNodes(_)>>
	 else
	    <<StatusManager unbreak>>
	    {Root layout(<<StatusManager getBreakFlag($)>> Scale Font)}
	    case <<StatusManager getBreakStatus($)>>
	    of break then
	       <<ToplevelManager makeDirty(<<StatusManager
					   getBrokenNodes($)>>)>>
	       {Root layout(_ Scale Font)}
	       <<ToplevelManager refreshNumbers>>
	    else true
	    end
	 end
      end

      meth LayoutAfterSearch
	 case <<DialogManager getAutoHide($)>> then {@curNode hideFailed}
	 else true
	 end
	 <<Manager Layout>>
      end

      meth query(Query Order)
	 <<Manager clear>>
	 IsBAB   <- (Order\=False)
	 query   <- Query
	 order   <- Order
	 curNode <- False
	 PrevSol <- False
	 {self.status setBAB(@IsBAB)}
	 <<StatusManager start(_)>>
	 root <- {MakeRoot self Query Order}
	 <<Manager prepare>>
      end

      meth prepare
	 <<StatusManager stop>>
	 <<Manager Layout>>
	 <<Manager setCursor(@root)>>
	 <<ToplevelManager configurePointer(idle)>>
      end
      
      %%
      %% Implementation of ``Move'' functionality
      %%

      meth moveTop
	 <<Manager setCursor(@root)>>
      end

      meth moveCurrent
	 CurNode = @curNode
      in
	 curNode <- False
	 <<Manager setCursor(CurNode)>>
      end
      
      meth moveFrom(What)
	 case {@curNode What($)} of !False then true elseof Dest then
	    <<Manager setCursor(Dest)>>
	 end
      end

      meth busy
	 <<MenuManager busy>>
      end

      meth idle
	 <<MenuManager     idle>>
	 <<ToplevelManager configurePointer(idle)>>
      end
      
      %%
      %% Implementation of ``Search'' functionality
      %%

      meth getPrevSol($)
	 case @PrevSol of !False then False
	 elseof Sol then {Sol getOriginalSpace($)}
	 end
      end
      
      meth startSearch($ <= _)
	 <<Manager         busy>>
	 <<MenuManager     normal(explorer(halt))>>
	 <<ToplevelManager configurePointer(searching)>>
	 <<StatusManager   start($)>>
      end

      meth stopSearch(Sol Cursor <= False)
	 case @root==nil then true else
	    PutCursor = case Cursor==False then
			   case Sol==False then @curNode
			   else
			      case @IsBAB then PrevSol <- Sol
			      else true
			      end
			      case {Sol isHidden($)} then @curNode
			      else Sol
			      end
			   end
			else Cursor
			end
	 in
	    <<StatusManager   stop>>
	    <<ToplevelManager hideCursor>>
	    <<Manager         LayoutAfterSearch>>
	    <<Manager         setCursor(PutCursor)>>
	    <<MenuManager     disable(explorer(halt))>>
	    <<Manager         idle>>
	 end
      end
      
      meth next
	 CurNode = @curNode
	 Break   = <<Manager startSearch($)>>
      in
	 <<Manager stopSearch({CurNode next(Break <<getPrevSol($)>>
					    <<getSearchDist($)>>
					    <<getInfoDist($)>> $)})>>
      end
      
      meth all
	 <<Manager startSearch>>
	 <<Manager DoAll(<<DialogManager getUpdateSol($)>>)>>
      end

      meth DoAll(NoSol)
	 Break   = <<StatusManager getBreakFlag($)>>
	 CurNode = @curNode
	 Sol     = {CurNode next(Break <<getPrevSol($)>>
				 <<getSearchDist($)>>
				 <<getInfoDist($)>> $)}
      in
	 case Sol\=False andthen <<StatusManager getBreakStatus($)>>==none then
	    case @IsBAB then PrevSol <- Sol
	    else true
	    end
	    case NoSol==1 then
	       <<StatusManager stop>>
	       <<Manager       hideCursor>>
	       <<Manager       LayoutAfterSearch>>
	       <<StatusManager unbreak>>
	       <<Manager       DoAll(<<DialogManager getUpdateSol($)>>)>>
	    else
	       <<Manager DoAll(NoSol-1)>>
	    end
	 else <<Manager stopSearch(Sol)>>
	 end
      end

      meth step
	 CurNode = @curNode
      in
	 <<Manager startSearch(_)>>
	 <<Manager stopSearch({CurNode step(<<getPrevSol($)>>
					    <<getInfoDist($)>> $)})>>
      end

      meth nodes(ToDo)
	 <<Manager busy>>
	 <<StatusManager clearBreak>>
	 <<Manager hideCursor>>
	 {@curNode ToDo}
	 <<Manager Layout>>
	 <<Manager setCursor(@curNode False)>>
	 <<Manager idle>>
      end
      
      meth stat
	 StatNode = case @curNode
		    of !False then @root
		    elseof CurNode then CurNode
		    end
      in
	 case StatNode==False then true else
	    Number  = <<Manager getNumber(StatNode $)>>
	    Handler = {self.statAction get($)}
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
	 <<Manager setCursor(<<ToplevelManager findByXY(X Y $)>> True)>>
      end
      
      meth nodesByXY(What X Y)
	 <<Manager setCursor(<<ToplevelManager findByXY(X Y $)>> False)>>
	 <<Manager nodes(What)>>
      end
      
      meth selInfo(X Y)
	 Node = <<ToplevelManager findByXY(X Y $)>>
      in
	 case {Node isHidden($)} then true else
	    <<Manager nodesInfo(Node)>>
	 end
      end
      
      meth nodesInfo(Node <= False)
	 RealNode = case Node==False then @curNode else Node end
      in
	 case RealNode of !False then true elseof CurNode then
	    <<Manager busy>>
	    case {self.infoAction get($)}
	    of !False then true
	    elseof Handler then
	       Number = <<Manager getNumber(RealNode $)>>
	       Info   = {RealNode findSpace($)}
	    in
	       case Info==False then
		  <<DialogManager
		  error('Recomputation of information failed.')>>
	       elsecase {Procedure.arity Handler}
	       of 2 then thread {Handler Number Info} end
	       [] 3 then CloseAction in
		  ToClose <- CloseAction|@ToClose
		  thread {Handler Number Info ?CloseAction} end
	       end
	    end
	    <<Manager setCursor(CurNode)>>
	    <<Manager idle>>
	 end
      end

      meth nodesSelCmp
	 CurNode = @curNode
      in
	 cmpNode <- CurNode
	 <<Manager     getNumber(CurNode _)>>
	 <<MenuManager disable(nodes(selCmp))>>
	 <<MenuManager normal(nodes(deselCmp))>>
	 <<Manager setCursor(CurNode)>>
      end

      meth nodesDeselCmp
	 cmpNode <- False
	 <<MenuManager disable(nodes([deselCmp cmp]))>>
	 <<Manager setCursor(@curNode)>>
      end
      
      meth nodesCmp
	 CmpNode = @cmpNode
	 CurNode = @curNode
      in
	 case CurNode==CmpNode then true else
	    <<Manager busy>>
	    CurNumber = <<ToplevelManager getNumber(CurNode $)>>
	    CmpNumber = <<ToplevelManager getNumber(CmpNode $)>>
	    CurInfo   = {CurNode findSpace($)}
	    CmpInfo   = {CmpNode findSpace($)}
	    Handler   = {self.cmpAction get($)}
	 in
	    case CmpInfo==False orelse CurInfo==False then
	       <<DialogManager
	       error('Recomputation of information failed.')>>
	    elsecase {Procedure.arity Handler}
	    of 4 then
	       thread {Handler CmpNumber CmpInfo CurNumber CurInfo} end
	    [] 5 then CloseAction in
	       ToClose <- CloseAction|@ToClose
	       thread
		  {Handler CmpNumber CmpInfo CurNumber CurInfo ?CloseAction}
	       end
	    end
	    <<Manager setCursor(CurNode)>>
	    <<Manager idle>>
	 end
      end	    

      meth wake(Node KillId)
	 case {self.status getKill(_ $)}==KillId then
	    case Node.mom of !Sentinel then
	       <<Manager reset>>
	    elseof Mom then
	       {Mom  removeLast(<<Manager getPrevSol($)>>)}
	       {Node close}
	       {self.status removeBlocked}
	       curNode <- Mom
	       <<Manager step>>
	    end
	 else true
	 end
      end

      meth postscript
	 <<Manager hideCursor>>
	 <<ToplevelManager hideNumbers>>
	 <<Manager busy>>
	 <<DialogManager postscript>>
	 <<ToplevelManager unhideNumbers>>
	 <<Manager idle>>
	 case @curNode\=False then
	    <<Manager setCursor(@curNode False)>>
	 else true
	 end
      end
      
      meth close
	 <<UrObject close>>
	 {self.explorer ManagerClosed}
	 case @root of !False then true elseof Root then
	    {Root close}
	 end
	 <<Manager         clearDialogs>>
	 <<ToplevelManager close>>
      end

   end


end


