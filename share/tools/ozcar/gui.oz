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
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
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
	 elsecase {IsSpace X}      then SpaceType
	 else                           UnknownType
	 end
      else                              UnboundType
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
		    wm(iconbitmap self.toplevel IconBitMap)
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
	     [self.emacsThreadsButton #
	      AddQueriesBitmap #
	      toggleEmacsThreads #
	      ConfigEmacsThreads

	      self.subThreadsButton #
	      AddSubThreadsBitmap #
	      toggleSubThreads #
	      ConfigSubThreads]
	     proc {$ B}
		C # Xbm # Action # Default = B
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
			    bitmap:           LocalBitMapDir # Xbm
			    relief:           flat %raised
			    width:            CheckButtonWidth
			    height:           CheckButtonHeight
			    variable:         {New Tk.variable tkInit(Default)}
			    action:           self # Action)}
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
	 ThisThread = {Thread.this}
      in
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
		{Widget tk(insert 'end' {PrintF ' ' # Name {EnvVarWidth}})}
		{Widget tk(insert 'end' Print # '\n' T)}
		{Widget tk(tag bind T '<1>' Ac)}
		{Widget tk(tag conf T font:BoldFont)}
	     else skip end

	     %% enable "interleaved" filling of both environment windows
	     %% (keeps time short while one window is completely empty,
	     %%  thus avoiding too much flickering)
	     {Thread.preempt ThisThread}
	  end}
      end

      meth printEnv(vars:V<=unit)
	 case {Cget updateEnv} then
	    New in
	    EnvSync <- New = unit
	    thread
	       {WaitOr New {Alarm {Cget timeoutToUpdateEnv}}}
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
	 %% use two threads to reduce flickering
	 thread
	    Gui,Clear(self.LocalEnvText)
	    {self.LocalEnvText tk(conf foreground:DefaultForeground)}
	    case V == unit then skip else
	       Gui,DoPrintEnv(self.LocalEnvText Y SV)
	    end
	    Gui,Disable(self.LocalEnvText)
	 end
	 thread
	    Gui,Clear(self.GlobalEnvText)
	    {self.GlobalEnvText tk(conf foreground:DefaultForeground)}
	    case V == unit then skip else
	       Gui,DoPrintEnv(self.GlobalEnvText G SV)
	    end
	    Gui,Disable(self.GlobalEnvText)
	 end
      end

      meth frameClick(frame:F highlight:Highlight<=true)
	 case @currentThread == unit then skip
	 elsecase {Dbg.checkStopped @currentThread} then
	    %% allow switching of stack frames only if thread is stopped
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
	       Gui,frameClick(frame:F highlight:true)
	    end
	 end
      end

      meth SelectStackFrame(T)
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
	 LSF = @LastSelectedFrame
      in
	 {OzcarMessage 'UnselectStackFrame: LSF == ' # LSF}
	 case LSF > 0 then
	    Gui,DeactivateLine(LSF)
	    LastSelectedFrame <- 0
	 else skip end
      end

      meth printStackFrame(frame:Frame delete:Delete<=true)
	 W          = self.StackText
	 FrameNr    = Frame.nr    %% frame number
	 FrameName  = Frame.name  %% procedure/builtin name
	 FrameArgs  = case Frame.args == unit then unit
		      else {FormatArgs Frame.args}  %% argument list
		      end
	 LineColTag = FrameNr       %% no need to garbage collect this tag
	 LineActTag = act # FrameNr %% dito
	 LineAction = {New Tk.action
		       tkInit(parent: W
			      action: self # frameClick(frame:Frame))}
	 LineEnd     = p(FrameNr 'end')
	 UpToDate    = 1 > 0 %{Emacs isUpToDate(Frame.time $)}
      in
	 case Delete then
	    Gui,Enable(W)
	    Gui,DeleteToEnd(W FrameNr+1)
	    Gui,DeleteLine(W FrameNr)
	 else skip end
	 {W tk(insert LineEnd
	       case Frame.dir == entry then ' -> ' else ' <- ' end #
	       FrameNr #
	       case {IsSpecialFrameName FrameName} then
		  ' ' # FrameName
	       else
		  ' {' # case FrameName
			 of ''  then '$'
			 [] nil then "nil"
			 [] '#' then "#"
			 else FrameName end
	       end
	       q(StackTag LineActTag LineColTag))}
	 case FrameArgs of unit then
	    case {IsSpecialFrameName FrameName} then skip
	    else {W tk(insert LineEnd ' ...'
		       q(StackTag LineActTag LineColTag))}
	    end
	 else
	    {ForAll FrameArgs
	     proc {$ Arg}
		P # V     = Arg
		ArgTag    = {W newTag($)}
		ArgAction = {New Tk.action
			     tkInit(parent: W
				    action: proc {$}
					       {Browse V}
					       LastClicked <- V
					    end)}
	     in
		{W tk(insert LineEnd ' '
		      q(StackTag LineActTag LineColTag))}
		{W tk(insert LineEnd P
		      q(StackTag LineColTag ArgTag))}
		{W tk(tag bind ArgTag '<1>' ArgAction)}
		{W tk(tag conf ArgTag font:BoldFont)}
	     end}
	 end
	 {W tk(insert LineEnd
	       case {IsSpecialFrameName FrameName} then '' else '}' end #
	       case UpToDate then nil else ' (source has changed)' end #
	       case Delete then '\n' else "" end
	       q(StackTag LineActTag LineColTag))}
	 {W tk(tag add  LineActTag LineEnd)} % extend tag to whole line
	 {W tk(tag add  LineColTag LineEnd)} % dito
	 {W tk(tag bind LineActTag '<1>' LineAction)}
	 case Delete then
	    Gui,Disable(W)
	    {W tk(yview 'end')}
	    Gui,frameClick(frame:Frame highlight:false)
	 else skip end
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
	 of active then
	    {OzcarMessage 'activating stack'}
	    {self.StackText tk(tag 'raise' StackTag)}
	    {self.StackText tk(tag conf StackTag foreground:DefaultForeground)}
	 [] inactive then
	    {OzcarMessage 'deactivating stack'}
	    {self.StackText tk(tag 'raise' StackTag)}
	    {self.StackText tk(tag conf StackTag foreground:DirtyColor)}
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

	    case A == TermAllAction then
	       Gui,doStatus('Terminating all threads...')
	       ThreadManager,termAll
	       {Delay 200} %% just to look nice... ;)
	       Gui,doStatus(' done' append)

	    elsecase A == TermAllButCurAction then
	       Gui,doStatus('Terminating all threads but current...')
	       ThreadManager,termAllButCur
	       {Delay 200} %% just to look nice... ;)
	       Gui,doStatus(' done' append)

	    elsecase A == DetachAllAction then
	       Gui,doStatus('Detaching all threads...')
	       ThreadManager,detachAll
	       {Delay 200}
	       Gui,doStatus(' done' append)

	    elsecase A == DetachAllButCurAction then
	       Gui,doStatus('Detaching all threads but current...')
	       ThreadManager,detachAllButCur
	       {Delay 200}
	       Gui,doStatus(' done' append)

	    elsecase A == DetachAllDeadAction then
	       Gui,doStatus('Detaching all dead threads...')
	       ThreadManager,detachAllDead
	       {Delay 200}
	       Gui,doStatus(' done' append)

	    elsecase A == StepButtonBitmap then
	       T = @currentThread
	    in
	       case T == unit then skip else
		  I = {Thread.id T}
		  S = {CheckState T}
	       in
		  case S
		  of running    then Gui,RunningStatus(I StepInto)
		  [] blocked    then Gui,BlockedStatus(T StepInto)
		  [] terminated then Gui,TerminatedStatus(T StepInto)
		  else
		     TopFrame = {@currentStack getTop($)}
		  in
		     case TopFrame == unit then skip else
			case TopFrame.dir == exit then skip else
			   N = {@currentStack incStep($)} in
			   Gui,status(N # ' step' #
				      case N == 1 then '' else 's' end #
				      ' into')
			end
			Gui,ContinueTo(T TopFrame)
		     end
		  end
	       end

	    elsecase A == NextButtonBitmap then
	       T = @currentThread
	    in
	       case T == unit then skip else
		  I = {Thread.id T}
		  S = {CheckState T}
	       in
		  case S
		  of running    then Gui,RunningStatus(I StepOver)
		  [] blocked    then Gui,BlockedStatus(T StepOver)
		  [] terminated then Gui,TerminatedStatus(T StepOver)
		  else
		     TopFrame = {@currentStack getTop($)}
		  in
		     case TopFrame == unit then skip else
			case TopFrame.dir == exit then skip else
			   N = {@currentStack incNext($)} in
			   Gui,status(N # ' step' #
				      case N == 1 then '' else 's' end #
				      ' over')
			   {Dbg.step T false}
			end
			Gui,ContinueTo(T TopFrame)
		     end
		  end
	       end

	    elsecase A == UnleashButtonBitmap then
	       T = @currentThread
	    in
	       case T == unit then skip else
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
			{self.StackText resetTags}
			Gui,resetLastSelectedFrame

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
			   {Stack rebuild(true)}
			   {Stack print}
			   {Stack getPos(file:F line:L column:C)}
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
		  lock UserActionLock then ThreadManager,detach(T I) end
	       end

	    elsecase A == TermButtonBitmap then
	       T = @currentThread
	    in
	       case T == unit then skip else
		  I = {Thread.id T}
	       in
		  ThreadManager,kill(T I)
	       end

	    elsecase A == BrowseStackAction then
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
	 {Widget resetTags}
	 {Widget tk(conf state:normal)}
	 {Widget tk(delete p(0 0) 'end')}
      end

      meth ClearNoTags(Widget)
	 {Widget tk(conf state:normal)}
	 {Widget tk(delete p(0 0) 'end')}
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

      meth Enable(Widget)
	 {Widget tk(conf state:normal)}
      end

      meth Append(Widget Text Color<=DefaultForeground)
	 {Widget tk(insert 'end' Text)}
	 {Widget tk(conf fg:Color)}
      end

      meth Disable(Widget)
	 {Widget tk(conf state:disabled)}
      end

      meth DeleteLine(Widget Nr)
	 {Widget tk(delete p(Nr 0) p(Nr 'end'))}
      end

      meth DeleteToEnd(Widget Nr)
	 {Widget tk(delete p(Nr 0) 'end')}
      end
   end
end
