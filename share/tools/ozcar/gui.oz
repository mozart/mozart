%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

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
	 case     {IsArray X}      then ArrayType
	 elsecase {IsThread X}     then ThreadType
	 elsecase {IsAtom X}       then {System.valueToVirtualString X 0 0}
	 elsecase {IsBool X}       then {System.valueToVirtualString X 0 0}
	 elsecase {IsCell X}       then CellType
	 elsecase {IsClass X}      then ClassType
	 elsecase {IsDictionary X} then DictionaryType
	 elsecase {IsFloat X}      then case X >= BigFloat then
					   BigFloatType
					else
					   {V2VS X}
					end
	 elsecase {IsInt X}        then case X >= BigInt then
					   BigIntType
					else
					   X
					end
	 elsecase {IsUnit  X}      then UnitType
	 elsecase {IsName X}       then NameType
	 elsecase {IsLock X}       then LockType
	 elsecase {IsObject X}     then ObjectType
	 elsecase {IsPort X}       then PortType
	 elsecase {IsProcedure X}  then ProcedureType
	 elsecase {IsDefList X}    then ListType
	 elsecase {IsTuple X}      then TupleType
	 elsecase {IsRecord X}     then RecordType
	 elsecase {IsChunk X}      then ChunkType
	 else                           UnknownType
	 end
      else                              UnboundType
      end
   end

   fun {MakeLines N}
      case N < 1 then ""
      else &\n | {MakeLines N-1} end
   end

   Lck =
   {New class
	   attr
	      L : false
	   meth init skip end
	   meth set   L <- true  end
	   meth unset L <- false end
	   meth is($) @L end
	end init}

   StackTag

in

   class Gui from Menu Dialog Help

      prop
	 locking

      feat
	 toplevel
	 menuBar
	 emacsThreadsButton
	 subThreadsButton

	 ButtonFrame

	 ThreadTree
	 StackText
	 GlobalEnvText
	 LocalEnvText

	 StatusFrame
	 StatusText

      attr
	 LastSelectedFrame : 0
	 EnvSync    : _
	 StatusSync : _

	 LastClicked : unit

      meth lastClickedValue(?V)
	 try
	    V = @LastClicked
	 catch failure(...) then skip
	 end
      end

      meth init
	 MinX # MinY = ToplevelMinSize
      in
	 %% create the main window, but delay showing it
	 self.toplevel = {New Tk.toplevel tkInit(title:    TitleName
						 'class':  'OzTools'
						 delete:   self # off
						 withdraw: true)}
	 {Tk.batch [wm(iconname   self.toplevel IconName)
		    wm(iconbitmap self.toplevel IconBitMap)
		    wm(minsize    self.toplevel MinX MinY)
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
	    Bs = {Map [StepButtonBitmap   # StepButtonColor
		       NextButtonBitmap   # NextButtonColor
		       UnleashButtonBitmap# UnleashButtonColor
		       StopButtonBitmap   # StopButtonColor
		       DetachButtonBitmap # DetachButtonColor
		       TermButtonBitmap   # TermButtonColor]
		  fun {$ S}
		     Bitmap # ForegroundColor = S
		     B = {New Tk.button
			  tkInit(parent:           self.ButtonFrame
				 bitmap:           (LocalBitMapDir # Bitmap #
						    BitmapExtension)
				 fg:               ForegroundColor
				 activeforeground: case UseColors then
						      ForegroundColor
						   else
						      SelectedForeground
						   end
				 relief:           raised
				 action:           self # action(Bitmap))}
		  in
		     {B tkBind(event:  HelpEvent
			       action: self # help(Bitmap))}
		     B
		  end}
	 in
	    {ForAll
	     [{Cget emacsThreads} #
	      self.emacsThreadsButton #
	      AddQueriesBitmap #
	      toggleEmacsThreads #
	      ConfigEmacsThreads

	      {Cget subThreads} #
	      self.subThreadsButton #
	      AddSubThreadsBitmap #
	      toggleSubThreads #
	      ConfigSubThreads]
	     proc {$ B}
		V # C # Xbm # Action # Default = B
	     in
		C = {New Tk.checkbutton
		     tkInit(parent:           self.ButtonFrame
			    %indicatoron:      false
			    %selectcolor:      CheckButtonSelectColor
			    fg:               ButtonForeground
			    activeforeground: case UseColors then
						 ButtonForeground
					      else
						 SelectedForeground
					      end
			    variable:         V
			    bitmap:           LocalBitMapDir # Xbm
			    relief:           flat %raised
			    width:            CheckButtonWidth
			    height:           CheckButtonHeight
			    action:           self # Action(V))}
		{C tkBind(event:  HelpEvent
			  action: self # help(Xbm))}
	     end}
	    {Tk.batch [pack(b(Bs) side:left padx:1 pady:1)
		       pack(self.emacsThreadsButton self.subThreadsButton
			    side:right padx:0)]}
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
	 {New Tk.text tkInit(parent: self.StatusFrame
			     state:  disabled
			     height: 1
			     width:  0
			     bd:     0
			     cursor: TextCursor
			     font:   StatusFont)}
	 {self.StatusText tkBind(event:  HelpEvent
				 action: self # help(StatusHelp))}
	 {Tk.send pack(self.StatusText side:left padx:2 fill:x expand:yes)}

	 %% create the thread tree object...
	 self.ThreadTree =
	 {New Tree tkInit(parent: self.toplevel
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
	 StackTag = 4711 * 4711
	 {ForAll [tk(conf state:normal)
		  tk(insert 'end' "" StackTag)
		  tk(conf state:disabled)] self.StackText}

	 {Tk.batch [grid(self.ThreadTree    row:3 column:0
			 sticky:nswe rowspan:2)
		    grid(self.StackText     row:3 column:1 sticky:nswe
			 columnspan:2)
		    grid(self.LocalEnvText  row:4 column:1 sticky:nswe)
		    grid(self.GlobalEnvText row:4 column:2 sticky:nswe)
		    grid(rowconfigure       self.toplevel 3 weight:1)
		    grid(rowconfigure       self.toplevel 4 weight:1)
		    grid(columnconfigure    self.toplevel 0 weight:1)
		    grid(columnconfigure    self.toplevel 1 weight:1)
		    grid(columnconfigure    self.toplevel 2 weight:1)
		   ]}
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
		  {OzcarMessage 'using saved variables'}
		  SavedVars
	       elsecase FrameId \= unit then
		  {OzcarMessage
		   'requesting variables for frame id ' # FrameId}
		  V = {Thread.frameVariables @currentThread FrameId}
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

      meth updateEnv
	 V = Gui,getEnv(unit $)
      in
	 Gui,PrintEnv(vars:V)
      end

      meth DoPrintEnv(Widget Vars SV)
	 {ForAll Vars
	  proc{$ V}
	     Name # Value = V
	     Print        = case {Cget envPrintTypes} then
			       {CheckType Value}
			    else
			       {V2VS Value}
			    end
	  in
	     case SV orelse {Atom.toString Name}.1 \= &` then
		T  = {Widget newTag($)}
		Ac = {New Tk.action
		      tkInit(parent: Widget
			     action: proc {$}
					{Browse Value} LastClicked <- Value
				     end)}
	     in
		{ForAll [tk(insert 'end' {PrintF ' ' # Name {EnvVarWidth}})
			 tk(insert 'end' Print # '\n' T)
			 tk(tag bind T '<1>' Ac)
			 tk(tag conf T font:BoldFont)] Widget}
	     else skip end
	  end}
      end

      meth printEnv(vars:V<=unit)
	 case {Cget updateEnv} then
	    New in
	    EnvSync <- New = unit
	    thread
	       {WaitOr New {Alarm TimeoutToUpdateEnv}}
	       case {IsDet New} then skip else
		  Gui,PrintEnv(vars:V)
	       end
	    end
	 else
	    {self.LocalEnvText  tk(conf foreground:DirtyColor)}
	    {self.GlobalEnvText tk(conf foreground:DirtyColor)}
	 end
      end

      meth PrintEnv(vars:V)
	 SV  = {Cget envSystemVariables}
	 Y#G = case V == unit then
		  nil # nil
	       else
		  {Reverse V.'Y'} # {Reverse V.'G'}
	       end
      in
	 Gui,Clear(self.LocalEnvText)
	 Gui,Clear(self.GlobalEnvText)

	 {self.LocalEnvText  tk(conf foreground:DefaultForeground)}
	 {self.GlobalEnvText tk(conf foreground:DefaultForeground)}

	 case V == unit then skip else
	    Gui,DoPrintEnv(self.LocalEnvText  Y SV)
	    Gui,DoPrintEnv(self.GlobalEnvText G SV)
	 end

	 Gui,Disable(self.LocalEnvText)
	 Gui,Disable(self.GlobalEnvText)
      end

      meth frameClick(frame:F highlight:Highlight<=true delay:D<=true)
	 L CurThr
      in
	 case D then
	    {Delay 70} % > TIME_SLICE
	    L = {Lck is($)}
	 else
	    L = false
	 end
	 CurThr = @currentThread
	 case L orelse CurThr == unit then skip
	 elsecase
	    {Dbg.checkStopped CurThr}
	 then    % allow switching of stack frames only if thread is stopped
	    Vars = Gui,getEnv(F $)
	 in
	    {OzcarMessage 'selecting frame #' # F.nr}
	    case Highlight then
	       L = case F.line == unit then unit else {Abs F.line} end
	    in
	       {SendEmacs delayedBar(file:F.file line:L column:F.column)}
	       Gui,SelectStackFrame(F.nr)
	    else
	       Gui,SelectStackFrame(0)
	    end
	    Gui,printEnv(vars:Vars)
	 else %% thread is running -- do nothing
	    skip
	 end
      end

      meth previousThread
	 {self.ThreadTree selectPrevious}
      end

      meth nextThread
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
	       Gui,frameClick(frame:F highlight:true delay:false)
	    end
	 end
      end

      meth SelectStackFrame(T)
	 W   = self.StackText
	 LSF = @LastSelectedFrame
      in
	 {OzcarMessage 'SelectStackFrame: LSF == ' # LSF # ', T == ' # T}
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
	 W   = self.StackText
	 LSF = @LastSelectedFrame
      in
	 {OzcarMessage 'UnselectStackFrame: LSF == ' # LSF}
	 case LSF > 0 then
	    Gui,DeactivateLine(LSF)
	    LastSelectedFrame <- 0
	 else skip end
      end

      meth printStackFrame(frame:Frame delete:Delete<=true)
	 W           = self.StackText
	 FrameNr     = Frame.nr                 % frame number
	 FrameName   = Frame.name               % procedure/builtin name
	 FrameArgs   = case Frame.args == unit then unit
		       else {FormatArgs Frame.args}  % argument list
		       end
	 FrameFile   = {StripPath  Frame.file}
	 FrameLine   = Frame.line
	 FrameColumn = Frame.column
	 LineTag     = FrameNr
	 LineAction  =
	 {New Tk.action
	  tkInit(parent: W
		 action: Ozcar # PrivateSend(frameClick(frame:Frame)))}
	 LineEnd     = FrameNr # '.end'
	 UpToDate    = 1 > 0 % {Emacs isUpToDate(Frame.time $)}
      in

	 {OzcarMessage '  printing frame #' # FrameNr}

	 lock
	    case Delete then
	       Gui,Enable(W)
	       Gui,DeleteToEnd(W FrameNr+1)
	       Gui,DeleteLine(W FrameNr)
	    else skip end

	    {W tk(insert LineEnd
		  case Frame.dir == entry then
		     ' -> '
		  else
		     ' <- '
		  end # FrameNr #
		  case {IsSpecialFrameName FrameName} then
		     ' '#FrameName
		  else
		     ' {' #
		     case FrameName == '' then '$' else FrameName end
		  end
		  q(StackTag LineTag))}

	    case FrameArgs of unit then
	       case {IsSpecialFrameName FrameName} then skip
	       else {W tk(insert LineEnd ' ...' q(StackTag LineTag))}
	       end
	    else
	       {ForAll FrameArgs
		proc {$ Arg}
		   ArgTag    = {W newTag($)}
		   ArgAction =
		   {New Tk.action
		    tkInit(parent: W
			   action: proc {$}
				      {Lck set}
				      {Browse Arg.2} LastClicked <- Arg.2
				      {Delay 150}
				      {Lck unset}
				   end)}
		in
		   {ForAll [tk(insert LineEnd ' ' q(StackTag LineTag))
			    tk(insert LineEnd Arg.1 q(StackTag LineTag ArgTag))
			    tk(tag bind ArgTag '<1>' ArgAction)
			    tk(tag conf ArgTag font:BoldFont)] W}
		end}
	    end

	    {ForAll [tk(insert LineEnd
			case {IsSpecialFrameName FrameName} then ''
			else '}'
			end #
			case UpToDate then nil else
			   ' (source has changed)' end #
			case Delete then '\n' else "" end
			q(StackTag LineTag))
		     tk(tag add  LineTag LineEnd) % extend tag to whole line
		     tk(tag bind LineTag '<1>' LineAction)] W}

	    case Delete then
	       FrameDir = Frame.dir
	    in
	       Gui,Disable(W)
	       {W tk(yview 'end')}
	       Gui,frameClick(frame:Frame highlight:false)
	    else skip end
	 end
      end

      meth printStack(id:I frames:Frames depth:Depth last:LastFrame<=nil)
	 W = self.StackText
      in
	 {OzcarMessage 'printing complete stack (#' # I # '/' # Depth # ')'}
	 case I == 0 then   % clear stack and env windows; reset title
	    {W title(StackTitle)}
	    Gui,Clear(W)
	    Gui,Disable(W)
	    Gui,clearEnv
	 else
	    {W title(AltStackTitle # I)}
	    lock
	       Gui,Clear(W)
	       case Depth == 0 then
		  Gui,Append(W ' The stack is empty.')
		  Gui,Disable(W)
		  Gui,clearEnv
	       else
		  Gui,Append(W {MakeLines Depth})  % Tk is _really_ stupid...
		  {ForAll Frames
		   proc{$ Frame}
		      Gui,printStackFrame(frame:Frame delete:false)
		   end}
		  {W tk(yview 'end')}
		  Gui,Disable(W)
		  case LastFrame == nil then
		     {OzcarError 'printStack: LastFrame == nil ?!'}
		  else
		     Gui,frameClick(frame:LastFrame highlight:false)
		  end
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

      meth removeNode(I)
	 {self.ThreadTree remove(I)}
      end

      meth killNode(I $)
	 {self.ThreadTree kill(I $)}
      end

      meth getStackText($)
	 self.StackText
      end

      meth status(S M<=clear C<=DefaultForeground)
	 New in
	 StatusSync <- New = unit
	 thread
	    {WaitOr New {Alarm TimeoutToStatus}}
	    case {IsDet New} then skip else
	       Gui,doStatus(S M C)
	    end
	 end
      end

      meth doStatus(S M<=clear C<=DefaultForeground)
	 W = self.StatusText
      in
	 case M == clear then
	    Gui,ClearNoTags(W)
	 else
	    Gui,Enable(W)
	 end
	 Gui,Append(W S C)
	 Gui,Disable(W)
      end

      meth BlockedStatus(T A)
	 Gui,doStatus('Thread #' # {Thread.id T} # ' is blocked, ' #
		      A # ' has no effect')
      end

      meth TerminatedStatus(T A)
	 Gui,doStatus('Thread #' # {Thread.id T} # ' is dead, ' #
		      A # ' has no effect')
      end

      meth StoppedStatus(I A)
	 Gui,doStatus('Thread #' # I # ' is not running, ' #
		      A # ' has no effect')
      end

      meth RunningStatus(I A)
	 Gui,doStatus('Thread #' # I # ' is already running, ' #
		      A # ' has no effect')
      end

      meth markStack(How)
	 case How
	 of active   then
	    {OzcarMessage 'activating stack'}
	    {ForAll [tk(tag 'raise' StackTag)
		     tk(tag conf StackTag foreground:DefaultForeground)
		    ] self.StackText}
	 [] inactive then
	    {OzcarMessage 'deactivating stack'}
	    {ForAll [tk(tag 'raise' StackTag)
		     tk(tag conf StackTag foreground:DirtyColor)
		    ] self.StackText}
	 else skip end
      end

      meth ContinueTo(T Frame)
	 Gui,UnselectStackFrame
	 Gui,markNode({Thread.id T} running)
	 Gui,markStack(inactive)
	 {SendEmacs configureBar(running)}
	 case Frame.frameID of unit then
	    {Dbg.unleash T 0}
	 elseof FrameID then
	    {Dbg.unleash T FrameID}
	 end
	 {Thread.resume T}
      end

      meth action(A)
	 lock
	    case {IsName A} then skip else
	       {OzcarMessage 'action:' # A}
	    end

	    case A == RemoveAllAction then
	       N in
	       Gui,doStatus('Killing and removing all threads...')
	       ThreadManager,killAll(N)
	       {Delay 200} %% just to look nice... ;)
	       Gui,doStatus(case N == 1 then
			       ' done  (1 thread collected)'
			    else
			       ' done  (' # N # ' threads collected)'
			    end append)

	    elsecase A == RemoveAllDeadAction then
	       Gui,doStatus('Removing all dead threads...')
	       thread
		  {Delay 200}
		  Gui,doStatus(' done' append)
	       end
	       ThreadManager,removeAllDead

	    elsecase A == StepButtonBitmap then
	       T = @currentThread
	    in
	       case T == unit then
		  Gui,doStatus(FirstSelectThread)
	       else
		  I = {Thread.id T}
		  S = {CheckState T}
	       in
		  case S
		  of running    then Gui,RunningStatus(I A)
		  [] blocked    then Gui,BlockedStatus(T A)
		  [] terminated then Gui,TerminatedStatus(T A)
		  else
		     TopFrame = {@currentStack getTop($)}
		  in
		     case TopFrame == unit then skip else
			Gui,ContinueTo(T TopFrame)
		     end
		  end
	       end

	    elsecase A == NextButtonBitmap then
	       T = @currentThread
	    in
	       case T == unit then
		  Gui,doStatus(FirstSelectThread)
	       else
		  I = {Thread.id T}
		  S = {CheckState T}
	       in
		  case S
		  of running    then Gui,RunningStatus(I A)
		  [] blocked    then Gui,BlockedStatus(T A)
		  [] terminated then Gui,TerminatedStatus(T A)
		  else
		     TopFrame = {@currentStack getTop($)}
		  in
		     case TopFrame == unit then skip else
			case TopFrame.dir == exit then
			   {OzcarMessage '`next\' on exit --> doing a `step\''}
			else
			   {Dbg.step T false}
			end
			Gui,ContinueTo(T TopFrame)
		     end
		  end
	       end

	    elsecase A == UnleashButtonBitmap then
	       T = @currentThread
	    in
	       case T == unit then
		  Gui,doStatus(FirstSelectThread)
	       else
		  I = {Thread.id T}
		  S = {CheckState T}
	       in
		  case S
		  of running    then Gui,RunningStatus(I A)
		  [] blocked    then Gui,BlockedStatus(T A)
		  [] terminated then Gui,TerminatedStatus(T A)
		  else
		     Frame
		     LSF = @LastSelectedFrame
		     Stk = @currentStack
		  in
		     {Stk getFrame(LSF Frame)}
		     case Frame == unit then skip
%		     elsecase Frame.dir == exit then
%			Gui,doStatus('Already at end of procedure ' #
%				     'application -- unleash has no effect')
		     else
			{Dbg.step T false}
			{Stk rebuild(true)}
			case Frame.frameID of unit then
			   {Dbg.unleash T 0}
			elseof FrameID then
			   {Dbg.unleash T FrameID}
			end

			%% delete all tags
			{self.StackText resetReservedTags({Stk getSize($)})}
			{self.StackText resetTags}

			Gui,markNode({Thread.id T} running)
			Gui,markStack(inactive)
			Gui,doStatus('Unleashing thread #' # I #
				     ' to frame ' #
				     case LSF == 0 then 1 else LSF end)
			{SendEmacs configureBar(running)}
			{Thread.resume T}
		     end
		  end
	       end

	    elsecase A == StopButtonBitmap then
	       T = @currentThread
	    in
	       case T == unit then skip else
		  S = {Thread.state T}
	       in
		  case S == terminated then Gui,TerminatedStatus(T A) else
		     I         = {Thread.id T}
		     ThreadDic = ThreadManager,getThreadDic($)
		     Stack     = {Dictionary.condGet ThreadDic I nil}
		  in
		     case
			Stack == nil then skip
		     elsecase
			{Dbg.checkStopped T} then Gui,StoppedStatus(I A)
		     else
			case S == blocked then
			   F L C in
			   {Thread.suspend T}
			   {ForAll [rebuild(true) print
				    getPos(file:F line:L column:C)] Stack}
			   {SendEmacs bar(file:F line:L column:C state:S)}
			else
			   {Stack rebuild(true)}
			end
			{Dbg.step T true}
			Gui,doStatus('You have stopped thread #' # I)
		     end
		  end
	       end

	    elsecase A == DetachButtonBitmap then
	       T = @currentThread
	    in
	       case T == unit then skip else
		  I = {Thread.id T}
	       in
		  ThreadManager,detach(T I)
	       end

	    elsecase A == TermButtonBitmap then
	       T = @currentThread
	    in
	       case T == unit then skip else
		  I = {Thread.id T}
		  S = {CheckState T}
	       in
		  case S
		  of terminated then Gui,TerminatedStatus(T A)
		  else
		     ThreadManager,kill(T I)
		  end
	       end

	    elsecase A == StackAction then
	       T = @currentThread
	    in
	       case T == unit then
		  Gui,doStatus(FirstSelectThread)
	       else
		  P = {System.get errors}
	       in
		  {Browse {Thread.taskStack T P.'thread' false}}
	       end
	    end
	 end
      end

      meth toggleEmacs
	 case {Cget useEmacsBar} then
	    Gui,doStatus('Not using Emacs Bar')
	    {Emacs removeBar}
	 else
	    Gui,doStatus('Using Emacs Bar')
	 end
	 {Ctoggle useEmacsBar}
      end

      meth toggleUpdateEnv
	 case {Cget updateEnv} then
	    Gui,doStatus('Turning auto update off')
	 else
	    Gui,doStatus('Turning auto update on')
	 end
	 {Ctoggle updateEnv}
      end

      meth Clear(Widget)
	 {ForAll [resetTags
		  tk(conf state:normal)
		  tk(delete '0.0' 'end')] Widget}
      end

      meth ClearNoTags(Widget)
	 {ForAll [tk(conf state:normal)
		  tk(delete '0.0' 'end')] Widget}
      end

      meth resetReservedTags(Size)
	 {self.StackText resetReservedTags(Size)}
	 LastSelectedFrame <- 0
      end

      meth DeactivateLine(Tag)
	 {ForAll [tk(tag 'raise' Tag)
		  tk(tag conf Tag
		     relief:flat borderwidth:0
		     background: DefaultBackground
		     foreground: DefaultForeground)
		 ] self.StackText}
      end

      meth ActivateLine(Tag)
	 {ForAll [tk(tag 'raise' Tag)
		  tk(tag conf Tag
		     relief:raised borderwidth:0
		     background: SelectedBackground
		     foreground: SelectedForeground)
		  tk(see p(Tag 0))
		 ] self.StackText}
      end

      meth Enable(Widget)
	 {Widget tk(conf state:normal)}
      end

      meth Append(Widget Text Color<=DefaultForeground)
	 {ForAll [tk(insert 'end' Text)
		  tk(conf fg:Color)] Widget}
      end

      meth Disable(Widget)
	 {Widget tk(conf state:disabled)}
      end

      meth DeleteLine(Widget Nr)
	 {Widget tk(delete Nr#'.0' Nr#'.end')}
      end

      meth DeleteToEnd(Widget Nr)
	 {Widget tk(delete Nr#'.0' 'end')}
      end
   end
end
