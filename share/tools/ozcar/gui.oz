%%%
%%% Author:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.

local

   local
      fun {IsPerhapsList Xs}
	 case Xs of _|_ then true else false end
      end
   in
      fun {IsDefList X}
	 S = {Space.new fun {$} {IsPerhapsList X} end}
	 V = thread {Space.askVerbose S} end
      in
	 case V
	 of succeeded(entailed)
	 then
	    {Space.merge S}
	 [] blocked(_)
	 then
	    {Space.inject S proc {$ _} fail end}
	    false
	 end
      end
   end

   fun {FormatArgs A}
      {Map A
       fun {$ X}
	  case {Cget envPrintTypes} then
	     {CheckType X} # X
	  else
	     {V2VS X} # X
	  end
       end}
   end

   fun {CheckType X}
      case {IsDet X} then
	 case     {IsArray X}            then ArrayType
	 elsecase {BitArray.is X}        then BitArrayType
	 elsecase {IsThread X}           then ThreadType
	 elsecase {IsAtom X}           then {System.valueToVirtualString X 0 0}
	 elsecase {IsBool X}           then {System.valueToVirtualString X 0 0}
	 elsecase {IsCell X}             then CellType
	 elsecase {IsClass X}            then ClassType
	 elsecase {IsDictionary X}       then DictionaryType
	 elsecase {IsFloat X}            then case X >= BigFloat then
						 BigFloatType
					      else
						 {V2VS X}
					      end
%	 elsecase {IsChar X}             then CharType
%	 elsecase {FD.is X}              then FDValueType
	 elsecase {IsInt X}              then case X >= BigInt then
						 BigIntType
					      else
						 X
					      end
	 elsecase {IsUnit X}             then UnitType
	 elsecase {IsName X}             then NameType
	 elsecase {IsLock X}             then LockType
	 elsecase {IsObject X}           then ObjectType
	 elsecase {IsPort X}             then PortType
	 elsecase {IsProcedure X}        then ProcedureType
	 elsecase {IsDefList X}          then ListType
	 elsecase {IsTuple X}            then TupleType
	 elsecase {IsRecord X}           then RecordType
	 elsecase {IsChunk X}            then ChunkType
	 elsecase {IsSpace X}            then SpaceType
	 elsecase {FS.value.is X}        then FSValueType
	 elsecase {ForeignPointer.is X}  then ForeignPointerType
	 else                                 UnknownType
	 end
      elsecase {IsKinded X} then
	 case     {FD.is X}              then FDVarType
	 elsecase {FS.var.is X}          then FSVarType
	 elsecase {IsRecordC X}          then KindedRecordType
	 else                                 UnknownType
	 end
      else                                    {System.printName X}
      end
   end

   fun {MakeLines N}
      case N < 1 then ""
      else &\n | {MakeLines N-1} end
   end

   StackTag

in

   class Gui from Menu Dialog Help

      prop
	 locking

      feat
	 toplevel
	 menuBar

	 ButtonFrame
	 emacsThreadsMenu
	 subThreadsMenu

	 ThreadTree
	 StackText
	 GlobalEnvText
	 LocalEnvText

	 StatusFrame
	 StatusText

      attr
	 LastSelectedFrame : 0
	 EnvSync           : _
	 MarkStackSync     : _
	 MarkEnvSync       : _

	 LastClicked       : unit

      meth resetLastSelectedFrame
	 LSF = @LastSelectedFrame
      in
	 case LSF > 0 then
	    Gui,DeactivateLine(LSF)
	 else skip end
	 LastSelectedFrame <- 0
      end

      meth lastClickedValue(?V)
	 try
	    V = @LastClicked
	 catch failure(...) then skip
	 end
      end

      meth init
	 MinX # MinY = ToplevelMinSize
	 MaxX # MaxY = ToplevelMaxSize
      in
	 %% create the main window, but delay showing it
	 self.toplevel = {New Tk.toplevel tkInit(title:    TitleName
						 'class':  'OzTools'
						 delete:   self # off
						 withdraw: true)}
	 {Tk.batch [wm(iconname   self.toplevel IconName)
		    wm(minsize    self.toplevel MinX MinY)
		    wm(maxsize    self.toplevel MaxX MaxY)
		    wm(geometry   self.toplevel ToplevelGeometry)]}

	 Menu,init

	 {ForAll [self.ButtonFrame self.StatusFrame]
	  proc{$ F}
	     F = {New Tk.frame tkInit(parent: self.toplevel
				      bd:     1
				      relief: ridge)}
	  end}

	 {Tk.batch [grid(self.menuBar       row:0 column:0
			 sticky:we columnspan:3)
		    grid(self.ButtonFrame   row:1 column:0
			 sticky:we columnspan:3)
		    grid(self.StatusFrame   row:6 column:0
			 sticky:we columnspan:3)
		   ]}

	 local
	    Bs = {Map [StepButtonBitmap    # StepButtonColor #
		       'step into procedures'
		       NextButtonBitmap    # NextButtonColor #
		       'step over procedures'
		       UnleashButtonBitmap # UnleashButtonColor #
		       'unleash to selected frame'
		       StopButtonBitmap    # StopButtonColor #
		       'stop'
		       DetachButtonBitmap  # DetachButtonColor #
		       'detach'
		       TermButtonBitmap    # TermButtonColor #
		       'terminate and detach']
		  fun {$ S}
		     Bitmap # ForegroundColor # _ = S
		     B = {New Tk.button
			  tkInit(parent: self.ButtonFrame
				 bitmap: {OzcarBitmap Bitmap}
				 fg:     ForegroundColor
				 activeforeground:
				    case UseColors then
				       ForegroundColor
				    else
				       SelectedForeground
				    end
				 relief: raised
				 action: self # action(Bitmap))}
		  in
		     {B tkBind(event:  HelpEvent
			       action: self # help(Bitmap))}
%		     {Tk.send balloonhelp(B BalloonHelpText)}
		     B
		  end}

	    EmacsThreadsLabel = {New Tk.label
				 tkInit(parent: self.ButtonFrame
					text:   EmacsThreadsText
					pady:   3
					font:   ButtonFont)}
	    SubThreadsLabel = {New Tk.label
			       tkInit(parent: self.ButtonFrame
				      text:   SubThreadsText
				      pady:   3
				      font:   ButtonFont)}
	 in

	    self.emacsThreadsMenu =
	    {New TkTools.popupmenu tkInit(parent:   self.ButtonFrame
					  entries:  EmacsThreadsList
					  selected: 2
					  relief:   raised
					  padx:     5
					  pady:     2
					  width:    5
					  font:     ButtonFont
					  action:   proc {$ S}
						       Gui,setEmacsThreads(S)
						    end)}
	    {self.emacsThreadsMenu
	     tkBind(event:  HelpEvent
		    action: self # help(EmacsThreadsText))}

	    self.subThreadsMenu =
	    {New TkTools.popupmenu tkInit(parent:   self.ButtonFrame
					  entries:  SubThreadsList
					  selected: 2
					  relief:   raised
					  padx:     5
					  pady:     2
					  width:    7
					  font:     ButtonFont)}
	    {self.subThreadsMenu
	     tkBind(event:  HelpEvent
		    action: self # help(SubThreadsText))}

	    {Tk.batch [pack(b(Bs) side:left padx:1 pady:1)
		       pack(self.emacsThreadsMenu side:right padx:2)
		       pack(EmacsThreadsLabel side:right padx:0)
		       pack({New Tk.label tkInit(parent:self.ButtonFrame
						 width:0)} side:right padx:2)
		       pack(self.subThreadsMenu side:right padx:2)
		       pack(SubThreadsLabel side:right padx:0)]}

	 end

	 %% border line
	 local
	    F = {New Tk.frame tkInit(parent: self.toplevel
				     height: 1
				     bd:     0
				     relief: flat)}
	 in
	    {Tk.send grid(F row:2 column:0 sticky:we columnspan:3)}
	 end

	 self.StatusText =
	 {New StatusDisplay
	  tkInit(parent: self.StatusFrame
		 state:  disabled
		 height: 1
		 width:  0
		 bd:     0
		 cursor: TextCursor
		 font:   StatusFont
		 wrap:   none)}
	 {self.StatusText tkBind(event:  HelpEvent
				 action: self # help(StatusHelp))}
	 {Tk.send pack(self.StatusText side:left padx:2 fill:x expand:yes)}

	 %% create the thread tree object...
	 self.ThreadTree =
	 {New Tree tkInit(parent: self.toplevel
			  ozcar:  self
			  title:  TreeTitle
			  relief: sunken
			  bd:     1
			  width:  ThreadTreeWidth
			  bg:     DefaultBackground)}
	 {self.ThreadTree tkBind(event:  HelpEvent
				 action: self # help(TreeTitle))}

	 %% ...and the text widgets for stack and environment
	 {ForAll [self.StackText       # StackTitle      # StackTextWidth
		  self.LocalEnvText    # LocalEnvTitle   # EnvTextWidth
		  self.GlobalEnvText   # GlobalEnvTitle  # EnvTextWidth ]
	  proc {$ T}
	     T.1 = {New ScrolledTitleText tkInit(parent: self.toplevel
						 title:  T.2
						 wrap:   none
						 state:  disabled
						 width:  T.3
						 cursor: TextCursor
						 font:   DefaultFont
						 bg:     DefaultBackground)}
	     {T.1 tkBind(event:  HelpEvent
			 action: self # help(T.2))}

	  end}

	 %% the global stack tag, to make Tk happy (and me!! :-))
	 StackTag = 'globalTag'
	 {self.StackText tk(conf state:normal)}
	 {self.StackText tk(insert 'end' "" StackTag)}
	 {self.StackText tk(conf state:disabled)}

	 {Tk.batch [grid(self.ThreadTree.frame    row:3 column:0
			 sticky:nswe rowspan:2)
		    grid(self.StackText.frame     row:3 column:1 sticky:nswe
			 columnspan:2)
		    grid(self.LocalEnvText.frame  row:4 column:1 sticky:nswe)
		    grid(self.GlobalEnvText.frame row:4 column:2 sticky:nswe)
		    grid(rowconfigure    self.toplevel 3 weight:1)
		    grid(rowconfigure    self.toplevel 4 weight:1)
		    grid(columnconfigure self.toplevel 0 weight:1)
		    grid(columnconfigure self.toplevel 1 weight:1)
		    grid(columnconfigure self.toplevel 2 weight:1)
		   ]}
      end

      meth setEmacsThreads(S)
	 case S == AttachText then
	    {EnqueueCompilerQuery setSwitch(runwithdebugger true)}
	 else
	    {EnqueueCompilerQuery setSwitch(runwithdebugger false)}
	 end
      end

      meth checkSubThreads($)
	 {self.subThreadsMenu getCurrent($)}
      end

      meth getEnv(Frame $)
	 NullEnv = v('Y': nil 'G': nil)
      in
	 case @currentStack == unit then
	    NullEnv
	 else
	    F = case Frame == unit then
		   case @LastSelectedFrame > 0 then
		      {@currentStack getFrame(@LastSelectedFrame $)}
		   else
		      {@currentStack getTop($)}
		   end
		else
		   Frame
		end
	 in
	    case F == unit then
	       NullEnv
	    else
	       FrameId   = F.frameID
	       SavedVars = F.vars
	    in
	       case SavedVars \= unit then
		  {OzcarMessage
		   'getEnv: using saved variables of frame ' # F.nr}
		  SavedVars
	       elsecase FrameId \= unit then
		  {OzcarMessage
		   'getEnv: requesting variables for frame ' # F.nr}
		  V = {Debug.getFrameVariables @currentThread FrameId}
	       in
		  case V == unit then %% `FrameId' was an invalid frame id...
		     NullEnv
		  else
		     V
		  end
	       else
		  NullEnv
	       end
	    end
	 end
      end

      meth ProcessClick(V)
	 {Browser.browse V}
	 LastClicked <- V
      end

      meth updateEnv
	 V = Gui,getEnv(unit $)
      in
	 Gui,markEnv(active)
	 Gui,PrintEnv(vars:V)
      end

      meth DoPrintEnv(Widget Vars SV)
	 {ForAll Vars
	  proc{$ V}
	     Name # Value = V
	     PrintName # PrintValue # ClickValue =
	     case {Cget envPrintTypes} then
		Name # {CheckType Value} # Value
	     else
		case {IsDet Value} andthen {IsCell Value} then
		   X = {Access Value}
		in
		   {VirtualString.toAtom
		    '{Access ' # Name # '}'} # {V2VS X} # X
		else
		   Name # {V2VS Value} # Value
		end
	     end
	  in
	     case SV orelse {Atom.toString Name}.1 \= &` then
		T  = {Widget newTag($)}
		Ac = {New Tk.action
		      tkInit(parent: Widget
			     action: self # ProcessClick(ClickValue))}
	     in
		{Widget tk(insert 'end'
			   {PrintF ' ' # PrintName {EnvVarWidth}})}
		{Widget tk(insert 'end' PrintValue # '\n' T)}
		{Widget tk(tag bind T '<1>' Ac)}
		{Widget tk(tag conf T font:BoldFont)}
	     else skip end
	  end}
      end

      meth printEnv(vars:V<=unit)
	 case {Cget updateEnv} then
	    New in
	    EnvSync <- New = unit
	    Gui,markEnv(active)
	    thread
	       {WaitOr New {Alarm {Cget timeoutToUpdateEnv}}}
	       case {IsDet New} then skip else
		  Gui,PrintEnv(vars:V)
	       end
	    end
	 else
	    Gui,markEnv(inactive)
	 end
      end

      meth PrintEnv(vars:V)
	 SV  = {Cget envSystemVariables}
	 Y#G = case V == unit then
		  nil # nil
	       else
		  V.'Y' # V.'G'
	       end
      in
	 {self.LocalEnvText resetTags}
	 Gui,Clear(self.LocalEnvText)
	 case V == unit then skip else
	    Gui,DoPrintEnv(self.LocalEnvText Y SV)
	 end
	 Gui,Disable(self.LocalEnvText)

	 {self.GlobalEnvText resetTags}
	 Gui,Clear(self.GlobalEnvText)
	 case V == unit then skip else
	    Gui,DoPrintEnv(self.GlobalEnvText G SV)
	 end
	 Gui,Disable(self.GlobalEnvText)
      end

      meth FrameClick(frame:F highlight:Highlight<=true)
	 case @currentThread == unit then skip
	 elsecase {Dbg.checkStopped @currentThread} then
	    %% allow switching of stack frames only if thread is stopped
	    Vars = Gui,getEnv(F $)
	 in
	    case Highlight then
	       L = case F.line == unit then unit else {Abs F.line} end
	    in
	       {SendEmacs delayedBar(file:F.file line:L column:F.column
				     state:unchanged)}
	       Gui,SelectStackFrame(F.nr)
	    else
	       Gui,SelectStackFrame(0)
	    end
	    Gui,printEnv(vars:Vars)
	 else %% thread is running
	    Gui,markEnv(inactive)
	 end
      end

      meth previousThread
	 case {IsFree @switchDone} then skip else
	    switchDone <- _
	 end
	 {self.ThreadTree selectPrevious}
      end

      meth nextThread
	 case {IsFree @switchDone} then skip else
	    switchDone <- _
	 end
	 {self.ThreadTree selectNext}
      end

      meth neighbourStackFrame(Delta)
	 Stack = @currentStack
      in
	 case Stack == unit then skip else
	    LSF = @LastSelectedFrame
	    N   = case LSF == 0 then ~1 else LSF + Delta end
	    F   = {Stack getFrame(N $)}
	 in
	    case F == unit then skip else
	       Gui,FrameClick(frame:F)
	    end
	 end
      end

      meth SelectStackFrame(T)
	 LSF = @LastSelectedFrame
      in
	 case LSF \= T then
	    case LSF > 0 then
	       Gui,DeactivateLine(LSF)
	    else skip end
	    case T > 0 then
	       Gui,ActivateLine(T)
	    else skip end
	    LastSelectedFrame <- T
	 else skip end
      end

      meth UnselectStackFrame
	 LSF = @LastSelectedFrame
      in
	 case LSF > 0 then
	    Gui,DeactivateLine(LSF)
	    LastSelectedFrame <- 0
	 else skip end
      end

      meth printStackFrame(frame:Frame delete:Delete<=true)
	 W          = self.StackText
	 FrameNr    = Frame.nr
	 FrameName  = case Frame.name
		      of ''  then '$'
		      [] nil then "nil"
		      [] '#' then "#"
		      else Frame.name end
	 FrameData  = Frame.data
	 FrameArgs  = case Frame.args == unit then unit
		      else {FormatArgs Frame.args}  %% argument list
		      end
	 LineColTag = FrameNr       %% no need to garbage collect this tag
	 LineActTag = act # FrameNr %% dito
	 LineAction = {New Tk.action
		       tkInit(parent: W
			      action: self # FrameClick(frame:Frame))}
	 LineEnd    = p(FrameNr 'end')
	 UpToDate   = 1 > 0 %{Emacs isUpToDate(Frame.time $)}
	 Arrow      = case Frame.dir == entry then ' -> ' else ' <- ' end
      in
	 case Delete then
	    Gui,Enable(W)
	    Gui,DeleteToEnd(W FrameNr+1)
	    Gui,DeleteLine(W FrameNr)
	 else skip end
	 case Frame.kind \= 'call' then
	    {W tk(insert LineEnd Arrow # FrameNr # ' ' # FrameName
		  q(StackTag LineActTag LineColTag))}
	 elsecase {IsDet FrameData} andthen FrameData == unit then
	    {W tk(insert LineEnd Arrow # FrameNr # ' {' # FrameName
		  q(StackTag LineActTag LineColTag))}
	 else
	    ProcTag    = {W newTag($)}
	    ProcAction = {New Tk.action
			  tkInit(parent: W
				 action: self # ProcessClick(FrameData))}
	 in
	    {W tk(insert LineEnd Arrow # FrameNr # ' {'
		  q(StackTag LineActTag LineColTag))}
	    {W tk(insert LineEnd FrameName
		  q(StackTag LineColTag ProcTag))}
	    {W tk(tag bind ProcTag '<1>' ProcAction)}
	    {W tk(tag conf ProcTag font:BoldFont)}
	 end
	 case FrameArgs of unit then
	    case Frame.kind \= 'call' then
	       skip
	    else
	       {W tk(insert LineEnd ' ...' q(StackTag LineActTag LineColTag))}
	    end
	 else
	    {ForAll FrameArgs
	     proc {$ Arg}
		P # V     = Arg
		ArgTag    = {W newTag($)}
		ArgAction = {New Tk.action
			     tkInit(parent: W
				    action: self # ProcessClick(V))}
	     in
		{W tk(insert LineEnd ' ' q(StackTag LineActTag LineColTag))}
		{W tk(insert LineEnd P q(StackTag LineColTag ArgTag))}
		{W tk(tag bind ArgTag '<1>' ArgAction)}
		{W tk(tag conf ArgTag font:BoldFont)}
	     end}
	 end
	 {W tk(insert LineEnd
	       case Frame.kind \= 'call' then
		  '' else '}'
	       end # case UpToDate then nil else
			' (source has changed)' end #
	       case Delete then '\n' else "" end
	       q(StackTag LineActTag LineColTag))}
	 {W tk(tag add  LineActTag LineEnd)} % extend tag to EOL
	 {W tk(tag add  LineColTag LineEnd)} % dito
	 {W tk(tag bind LineActTag '<1>' LineAction)}
	 case Delete then
	    Gui,Disable(W)
	    {W tk(yview 'end')}
	    Gui,FrameClick(frame:Frame highlight:false)
	 else skip end
      end

      meth printStack(id:I frames:Frames depth:Depth last:LastFrame<=nil)
	 W = self.StackText
      in
	 {W resetTags}
	 Gui,Clear(W)
	 case I == 0 then   % clear stack and env windows; reset title
	    {self.StackText title(StackTitle)}
	    Gui,Disable(W)
	    Gui,clearEnv
	 else
	    {self.StackText title(AltStackTitle # I)}
	    case Depth == 0 then
	       case {CheckState @currentThread} == running then
		  Gui,Append(W (' There is no stack available\n as' #
				' the thread is still running.'))
	       else
		  Gui,Append(W ' The stack is empty.')
	       end
	       Gui,Disable(W)
	       Gui,clearEnv
	    else
	       Gui,Append(W {MakeLines Depth}) % Tk is _really_ stupid...
	       {ForAll Frames
		proc{$ Frame}
		   Gui,printStackFrame(frame:Frame delete:false)
		end}
	       Gui,Disable(W)
	       {W tk(yview 'end')}
	       case LastFrame == nil then
		  {OzcarError 'printStack: LastFrame == nil ?!'}
	       else
		  Gui,FrameClick(frame:LastFrame highlight:false)
	       end
	    end
	 end
      end

      meth clearStack
	 Gui,printStack(id:0 frames:nil depth:0)
      end

      meth clearEnv
	 Gui,printEnv
      end

      meth selectNode(I)
	 {self.ThreadTree select(I)}
      end

      meth markNode(I How)
	 {self.ThreadTree mark(I How)}
      end

      meth addNode(I Q)
	 {self.ThreadTree add(I Q)}
      end

      meth killNode(I $)
	 {self.ThreadTree kill(I $)}
      end

      meth getStackText($)
	 self.StackText
      end

      meth status(S M<=clear C<=DefaultForeground)
	 case M == clear then
	    {self.StatusText replace(S C)}
	 else
	    {self.StatusText append(S C)}
	 end
      end

      meth BlockedStatus(T A)
	 Gui,status('Thread ' # {Debug.getId T} # ' is blocked, ' #
		    A # ' has no effect')
      end

      meth TerminatedStatus(T A)
	 Gui,status('Thread ' # {Debug.getId T} # ' is dead, ' #
		    A # ' has no effect')
      end

      meth StoppedStatus(I A)
	 Gui,status('Thread ' # I # ' is not running, ' #
		    A # ' has no effect')
      end

      meth RunningStatus(I A)
	 Gui,status('Thread ' # I # ' is already running, ' #
		    A # ' has no effect')
      end

      meth markStack(How)
	 New in
	 MarkStackSync <- New = unit
	 thread
	    {WaitOr New {Alarm TimeoutToUpdate*10}}
	    case {IsDet New} then skip else
	       Gui,DoMarkStack(How)
	    end
	 end
      end

      meth DoMarkStack(How)
	 case How
	 of active then
	    {self.StackText tk(tag 'raise' StackTag)}
	    {self.StackText tk(tag conf StackTag foreground:DefaultForeground)}
	 [] inactive then
	    {self.StackText tk(tag 'raise' StackTag)}
	    {self.StackText tk(tag conf StackTag foreground:DirtyColor)}
	 end
      end

      meth markEnv(How)
	 New in
	 MarkEnvSync <- New = unit
	 thread
	    {WaitOr New {Alarm TimeoutToUpdate*10}}
	    case {IsDet New} then skip else
	       Gui,DoMarkEnv(How)
	    end
	 end
      end

      meth DoMarkEnv(How)
	 case How
	 of active then
	    {self.LocalEnvText  tk(conf foreground:DefaultForeground)}
	    {self.GlobalEnvText tk(conf foreground:DefaultForeground)}
	 [] inactive then
	    {self.LocalEnvText  tk(conf foreground:DirtyColor)}
	    {self.GlobalEnvText tk(conf foreground:DirtyColor)}
	 end
      end

      meth MarkRunning(T)
	 case {CheckState T} == blocked then skip else
	    Gui,markNode({Debug.getId T} running)
	 end
	 Gui,markStack(inactive)
	 Gui,markEnv(inactive)
	 {SendEmacs configureBar(running)}
      end

      meth ContinueTo(T Frame)
	 Gui,UnselectStackFrame
	 Gui,MarkRunning(T)
	 case Frame.frameID of unit then
	    {Dbg.unleash T 0}
	 elseof FrameID then
	    {Dbg.unleash T FrameID}
	 end
	 {Thread.resume T}
      end

      meth action(A)
	 lock
	    {Wait @detachDone}
	    {Wait @switchDone}
	    case ThreadManager,emptyForest($) then
	       Gui,status(NoThreads)
	    else
	       Gui,DoAction(A)
	    end
	 end
      end

      meth DoAction(A)
	 case {IsName A} then skip else
	    {OzcarMessage 'action(' # A # ')'}
	 end

	 case A == TermAllAction then
	    Gui,status('Terminating all threads...')
	    ThreadManager,termAll
	    Gui,status(' done' append)

	 elsecase A == TermAllButCurAction then
	    Gui,status('Terminating all threads but current...')
	    ThreadManager,termAllButCur
	    Gui,status(' done' append)

	 elsecase A == DetachAllAction then
	    Gui,status('Detaching all threads...')
	    ThreadManager,detachAll
	    Gui,status(' done' append)

	 elsecase A == DetachAllButCurAction then
	    Gui,status('Detaching all threads but current...')
	    ThreadManager,detachAllButCur
	    Gui,status(' done' append)

	 elsecase A == DetachAllDeadAction then
	    Gui,status('Detaching all dead threads...')
	    ThreadManager,detachAllDead
	    Gui,status(' done' append)

	 elsecase A == StepButtonBitmap then T I in
	    T = @currentThread
	    I = {Debug.getId T}
	    case {CheckState T}
	    of running    then Gui,RunningStatus(I StepInto)
	    [] terminated then Gui,TerminatedStatus(T StepInto)
	    else
	       TopFrame = {@currentStack getTop($)}
	    in
	       case TopFrame == unit then skip else
		  case TopFrame.dir == exit then skip else
		     {@currentStack incStep(_)}
		  end
		  Gui,status('')
		  Gui,ContinueTo(T TopFrame)
	       end
	    end

	 elsecase A == NextButtonBitmap then T I in
	    T = @currentThread
	    I = {Debug.getId T}
	    case {CheckState T}
	    of running    then Gui,RunningStatus(I StepOver)
	    [] terminated then Gui,TerminatedStatus(T StepOver)
	    else
	       TopFrame = {@currentStack getTop($)}
	    in
	       case TopFrame == unit then skip else
		  case TopFrame.dir == exit then skip else
		     {@currentStack incNext(_)}
		     {Dbg.step T false}
		  end
		  Gui,status('')
		  Gui,ContinueTo(T TopFrame)
	       end
	    end

	 elsecase A == UnleashButtonBitmap then T I in
	    T = @currentThread
	    I = {Debug.getId T}
	    case {CheckState T}
	    of running    then Gui,RunningStatus(I A)
	    [] terminated then Gui,TerminatedStatus(T A)
	    else
	       Frame
	       LSF = @LastSelectedFrame
	       Stk = @currentStack
	    in
	       {Stk getFrame(LSF Frame)}
	       case Frame == unit then skip
%              elsecase Frame.dir == exit then
%                 Gui,status('Already at end of procedure ' #
%                              'application -- unleash has no effect')
	       else
		  {Dbg.step T false}
		  {Stk rebuild(true)}
		  case Frame.frameID of unit then
		     {Dbg.unleash T 0}
		  elseof FrameID then
		     {Dbg.unleash T FrameID}
		  end

		  Gui,resetLastSelectedFrame
		  Gui,MarkRunning(T)
		  Gui,status('Unleashing thread ' # I #
			     ' to frame ' #
			     case LSF == 0 then 1 else LSF end)
		  {Thread.resume T}
	       end
	    end

	 elsecase A == StopButtonBitmap then T S in
	    T = @currentThread
	    S = {Thread.state T}
	    case S == terminated then Gui,TerminatedStatus(T A) else
	       I         = {Debug.getId T}
	       ThreadDic = ThreadManager,getThreadDic($)
	       Stack     = {Dictionary.condGet ThreadDic I nil}
	    in
	       case
		  Stack == nil then skip
	       elsecase
		  {Dbg.checkStopped T} then Gui,StoppedStatus(I A)
	       else
		  Gui,status('You have stopped thread ' # I)
		  case S == blocked then
		     F L C in
		     {Thread.suspend T}
		     {Stack rebuild(true)}
		     {Stack print}
		     {Stack getPos(file:F line:L column:C)}
		     {SendEmacs bar(file:F line:L column:C state:S)}
		  else
		     {Stack rebuild(true)}
		  end
		  {Dbg.step T true}
	       end
	    end

	 elsecase A == DetachButtonBitmap then T I in
	    T = @currentThread
	    I = {Debug.getId T}
	    lock UserActionLock then ThreadManager,detach(T I) end

	 elsecase A == TermButtonBitmap then T I in
	    T = @currentThread
	    I = {Debug.getId T}
	    ThreadManager,kill(T I)

	 end
      end

      meth toggleEmacs
	 case {Cget useEmacsBar} then
	    Gui,status('Not using Emacs Bar')
	    {Emacs.condSend.interface removeBar}
	 else
	    Gui,status('Using Emacs Bar')
	 end
	 {Ctoggle useEmacsBar}
      end

      meth toggleUpdateEnv
	 case {Cget updateEnv} then
	    Gui,status('Turning auto update off')
	 else
	    Gui,status('Turning auto update on')
	 end
	 {Ctoggle updateEnv}
      end

      meth DeactivateLine(Tag)
	 {self.StackText tk(tag 'raise' Tag)}
	 {self.StackText tk(tag conf Tag
			    relief:flat borderwidth:0
			    background: DefaultBackground
			    foreground: DefaultForeground)}
      end

      meth ActivateLine(Tag)
	 {self.StackText tk(tag 'raise' Tag)}
	 {self.StackText tk(tag conf Tag
			    relief:raised borderwidth:0
			    background: SelectedBackground
			    foreground: SelectedForeground)}
	 {self.StackText tk(see p(Tag 0))}
      end

      meth Enable(W)
	 {W tk(conf state:normal)}
      end

      meth Clear(W)
	 {W tk(conf state:normal)}
	 {W tk(delete p(0 0) 'end')}
      end

      meth Append(W Text Color<=unit)
	 {W tk(insert 'end' Text)}
	 case Color == unit then skip else
	    {W tk(conf fg:Color)}
	 end
      end

      meth DeleteLine(W Nr)
	 {W tk(delete p(Nr 0) p(Nr 'end'))}
      end

      meth DeleteToEnd(W Nr)
	 {W tk(delete p(Nr 0) 'end')}
      end

      meth Disable(W)
	 {W tk(conf state:disabled)}
      end
   end
end
