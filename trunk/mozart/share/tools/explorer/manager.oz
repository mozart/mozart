%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   \insert menu-manager.oz
   
   \insert nodes.oz

   \insert toplevel-manager.oz

   \insert dialog-manager.oz

   \insert status-manager.oz

   Solve = Search.combinator.debug
   
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
	 killer

      attr
	 IsBAB:        False
	 Classes:      nil
	 root:         False
	 query:        False
	 order:        False
	 PrevBABSol:   False
	 ToClose:      nil

      
      meth init(EXPLORER)
	 PackAll = <<ToplevelManager init($)>>
      in
	 <<DialogManager   init>>
	 <<MenuManager     init>>
	 <<StatusManager   init>>
	 create self.killer
	    from UrObject
	    attr Flag Id
	    meth clear
	       Id <- {NewName}   @Flag=True   Flag <- _
	    end
	    meth get(?F ?I)
	       F=@Flag   I=@Id
	    end
	 end
	 self.explorer       = EXPLORER
	 {PackAll}
	 <<ToplevelManager configurePointer(idle)>>
      end
      
      meth clear
	 {self.killer clear}
	 <<MenuManager     clear>>
	 <<StatusManager   clear>>
	 <<DialogManager   clear>>
	 <<ToplevelManager clear>>
	 <<Manager         clearNumbers>>
	 Classes    <- False
	 case @root of !False then true elseof Root then
	    root <- False
	 end
	 PrevBABSol <- False
      end

      meth clearNumbers
	 <<MenuManager disable(nodes(clear))>>
	 <<ToplevelManager clearNumbers>>
	 <<Manager         clearDialogs>>
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
	 case {HasSubtreeAt O search} then <<setSearchOptions(O.search)>>
	 else true end
	 case {HasSubtreeAt O information} then <<setInfoOptions(O.information)>>
	 else true end
	 case {HasSubtreeAt O drawing} then <<setLayoutOptions(O.drawing)>>
	 else true end
	 case {HasSubtreeAt O postscript} then <<setPostscriptOptions(O.postscript)>>
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
	 case CurNode.kind==failed orelse CurNode.kind==unstable then
	    %% Can only happen if there is a single failed or unstable node
	    <<MenuManager disable([move([top cur]) nodes(stat)])>>
	 else
	    %% Move: Top and Current are always possible
	    <<MenuManager normal([move([top cur]) nodes(stat)])>>
	    local
	       IsBack = {CurNode back($)}\=False
	    in
	       <<MenuManager state(IsBack move(back))>>
	    end
	    %% Move: Previous Solution, Next Solution
	    <<MenuManager state({CurNode leftMost($)}\=CurNode move(leftMost))>>
	    <<MenuManager state({CurNode rightMost($)}\=CurNode move(rightMost))>>
	    case {@root hasSolutions($)} then
	       <<MenuManager state({CurNode nextSol($)}\=False move(nextSol))>>
	       <<MenuManager state({CurNode prevSol($)}\=False move(prevSol))>>
	    else
	       <<MenuManager disable(move([prevSol nextSol]))>>
	    end
	    %% Search
	    case <<StatusManager hasUnstable($)>> then
	       <<MenuManager  disable(search([next all step]))>>
	    else
	       IsNextPossible = {CurNode isNextPossible($)}
	       IsStepPossible = {CurNode isStepPossible($)}
	    in
	       <<MenuManager  state(IsNextPossible  search([next all]))>>
	       <<MenuManager  state(IsStepPossible  search(step))>>
	    end
	    %% Nodes
	    case {CurNode isHidden($)} then
	       <<MenuManager  normal(nodes([hide unhide]))>>
	       <<MenuManager  disable(nodes([info cmp selCmp deselCmp
					     hideFailed]))>>
	    else
	       <<MenuManager normal(nodes(info))>>
	       <<MenuManager state(@cmpNode==False nodes(selCmp))>>
	       <<MenuManager state(@cmpNode\=False nodes([deselCmp cmp]))>>
	       <<MenuManager state(CurNode.kind==choice nodes(hide))>>
	       local
		  IsUnhidable     = {CurNode isUnhidable($)}
		  IsFailedHidable = {CurNode isFailedHidable($)}
	       in
		  <<MenuManager  state(IsUnhidable nodes(unhide))>>
		  <<MenuManager  state(IsFailedHidable nodes(hideFailed))>>
	       end
	       <<MenuManager state(CurNode.kind==choice orelse
				   CurNode.kind==solved
				   nodes(stat))>>
	    end
	    %% Bring cursor to front
	    <<ToplevelManager setCursor(CurNode IsVisible)>>
	 end
      end

      meth LayoutAfterBreak(Root Scale Font)
	 <<ToplevelManager makeDirty(<<StatusManager getBrokenNodes($)>>)>>
	 {Root layout(_ Scale Font)}
	 <<ToplevelManager refreshNumbers>>
      end
      
      meth Layout
	 BreakFlag = {self.status getBreakFlag($)}
	 Scale     = @scale
	 Font      = case @curFont of !False then False
		     elseof CF then CF.name
		     end
	 Root      = @root
      in
	 <<ToplevelManager configurePointer(drawing)>>
	 case {System.isVar BreakFlag} then
	    {Root layout(BreakFlag Scale Font)}
	    case {System.isVar BreakFlag} then
	       <<ToplevelManager refreshNumbers>>
	    elsecase <<StatusManager isKilled($)>> then true
	    else
	       <<ToplevelManager makeDirty(<<StatusManager
					   getBrokenNodes($)>>)>>
	       {Root layout(_ Scale Font)}
	       <<ToplevelManager refreshNumbers>>
	    end
	 elsecase <<StatusManager isKilled($)>> then true
	 else
	    {Root hideUndrawn}
	    {Root layout(_ Scale Font)}
	    <<ToplevelManager refreshNumbers>>
	 end
      end

      meth LayoutAfterSearch
	 case <<DialogManager getAutoHide($)>> then
	    {@curNode hideFailed}
	 else true
	 end
	 <<Manager Layout>>
      end

      meth query(Query Order)
	 <<Manager clear>>
	 IsBAB <- (Order\=False)
	 query      <- Query
	 order      <- Order
	 curNode    <- False
	 PrevBABSol <- False
	 {self.status setBAB(@IsBAB)}
	 Classes    <- {MakeClasses 
			@IsBAB
			<<DialogManager getKeepSolutions($)>>
			<<DialogManager getInfoDistance($)>>
			self
			Order}
	 <<StatusManager start>>
	 local Root={MakeRoot Query @IsBAB @Classes}
	 in case {Det Root}
	    then
	       root  <- Root
	    end
	 end
	 <<Manager prepare>>
      end

      meth prepare
	 <<StatusManager stop>>
	 <<StatusManager update>>
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
	 <<MenuManager idle>>
	 <<ToplevelManager configurePointer(idle)>>
      end
      
      %%
      %% Implementation of ``Search'' functionality
      %%
      meth next
	 <<MenuManager busy>>
	 CurNode  = @curNode
	 MaxDepth = {self.menu.search.depth getNumber($)}
	 <<ToplevelManager configurePointer(searching)>>
	 <<StatusManager   allow({self.menu.search.nodes getNumber($)})>>
	 <<MenuManager     normal(search(halt))>>
	 <<StatusManager start>>
	 Sol = case @IsBAB then
		  {CurNode next(@PrevBABSol MaxDepth
				<<getSearchDistance($)>> $)}
	       else
		  {CurNode next(MaxDepth <<getSearchDistance($)>> $)}
	       end
      in
	 <<StatusManager   stop>>
	 <<StatusManager   update>>
	 <<ToplevelManager hideCursor>>
	 <<Manager LayoutAfterSearch>>
	 case Sol==False then
	    <<Manager setCursor(@curNode)>>
	 else
	    case @IsBAB then
	       PrevBABSol <- {Sol getInfo($)}
	    else true
	    end
	    <<Manager setCursor(Sol)>>
	 end
	 <<MenuManager  disable(search(halt))>>
	 <<Manager idle>>
      end
      
      meth all
	 case @curNode==False then true else
	    <<Manager busy>>
	    <<MenuManager  normal(search(halt))>>
	    <<StatusManager allow({self.menu.search.nodes getNumber($)})>>
	    <<StatusManager start>>
	    <<Manager DoAll(<<DialogManager getUpdateSol($)>>)>>
	 end
      end

      meth DoAll(NoSol)
	 <<ToplevelManager configurePointer(searching)>>
	 MaxDepth = {self.menu.search.depth getNumber($)}
	 Sol      = case @IsBAB then
		       {@curNode next(@PrevBABSol
				      MaxDepth
				      <<getSearchDistance($)>> $)}
		    else
		       {@curNode next(MaxDepth
				      <<getSearchDistance($)>> $)}
		    end
      in
	 case Sol==False then
	    <<StatusManager stop>>
	    <<StatusManager update>>
	    <<Manager hideCursor>>
	    <<Manager LayoutAfterSearch>>
	    <<Manager setCursor(@curNode)>>
	    <<MenuManager   disable(search(halt))>>
	    <<Manager idle>>
	 else
	    case @IsBAB then
	       PrevBABSol <- {Sol getInfo($)}
	    else true
	    end
	    case NoSol==1 then
	       <<StatusManager stop>>
	       <<StatusManager update>>
	       <<Manager hideCursor>>
	       <<Manager LayoutAfterSearch>>
	       <<Manager DoAll(<<DialogManager getUpdateSol($)>>)>>
	    else
	       <<Manager DoAll(NoSol-1)>>
	    end
	 end
      end

      meth step
	 case @curNode==False then true else
	    <<Manager busy>>
	    <<ToplevelManager configurePointer(searching)>>
	    <<StatusManager allow(1)>>
	    <<StatusManager start>>
	    Sol = case @IsBAB then
		     {@curNode next(@PrevBABSol
				     False <<getSearchDistance($)>> $)}
		  else
		     {@curNode next(False
				     <<getSearchDistance($)>> $)}
		  end
	 in
	    <<StatusManager stop>>
	    <<StatusManager update>>
	    <<Manager hideCursor>>
	    <<Manager LayoutAfterSearch>>
	    case @IsBAB andthen Sol\=False then
	       PrevBABSol <- {Sol getInfo($)}
	    else true
	    end
	    <<Manager setCursor({@curNode rightMost($)})>>
	    <<Manager idle>>
	 end
      end

      meth nodes(ToDo)
	 <<Manager busy>>
	 <<StatusManager unbreak>>
	 <<Manager hideCursor>>
	 {@curNode ToDo}
	 <<Manager Layout>>
	 <<Manager setCursor(@curNode False)>>
	 <<Manager idle>>
      end
      
      meth getNumber(Node ?N)
	 <<ToplevelManager getNumber(Node ?N)>>
	 <<MenuManager normal(nodes(clear))>>
      end
      
      meth stat
	 StatNode = case @curNode
		    of !False then @root
		    elseof CurNode then CurNode
		    end
      in
	 case StatNode==False then true else
	    Number  = <<Manager getNumber(StatNode $)>>
	    Handler = {self.menu.nodes.chooseStat.group.'self' getValue($)}
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
	    case {self.menu.nodes.chooseInfo.group.'self' getValue($)}
	    of !False then true
	    elseof Handler then
	       Number = <<Manager getNumber(RealNode $)>>
	       Info   = {RealNode getInfo($)}
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
	 <<Manager setCursor(@curNode)>>
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
	    CurInfo   = {CurNode getInfo($)}
	    CmpInfo   = {CmpNode getInfo($)}
	    Handler   = {self.menu.nodes.chooseCmp.group.'self' getValue($)}
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
	    <<Manager idle>>
	 end
      end	    

      meth wake(Mom Id Control Message)
	 case {self.killer get(_ $)}==Id then
	    case Mom of !False then
	       <<StatusManager   clear>>
	       <<ToplevelManager clear>>
	       <<Manager busy>>
	       {self.killer clear}
	       local Root={MakeRoot @query @IsBAB @Classes}
	       in
		  case {Det Root}
		  then
		     {@root close}
		     root  <- Root
		  end
	       end
	       <<Manager setCursor(@root)>>
	    else
	       <<Manager busy>>
	       case @IsBAB andthen {Label Control}==solved then
		  PrevBABSol <- Control.1
	       else true
	       end
	       <<StatusManager start>>
	       {self.status removeUnstable}
	       {Mom Message}
	       <<Manager setCursor(@curNode)>>
	    end
	    <<Manager prepare>>
	    <<Manager idle>>
	 else true
	 end
      end

      meth postscript
	 <<Manager hideCursor>>
	 <<Manager busy>>
	 <<DialogManager postscript>>
	 <<Manager idle>>
	 case @curNode\=False then
	    <<Manager setCursor(@curNode False)>>
	 else true
	 end
      end
      
\ifdef EXPLORER_DEBUG
      meth debug(M)
	 {@curNode M}
      end
\endif
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

