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
	 case     {IsArray X}            then ArrayType
	 elsecase {IsThread X}           then ThreadType
	 elsecase {IsAtom X}             then {System.valueToVirtualString X 0 0}
	 elsecase {IsBool X}             then {System.valueToVirtualString X 0 0}
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
	 elsecase {IsUnit  X}            then UnitType
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
	 elsecase {Foreign.pointer.is X} then ForeignPointerType
	 else                                 UnknownType
	 end
      elsecase {IsKinded X} then
	 case     {FD.is X}              then FDVarType
	 elsecase {FS.var.is X}          then FSVarType
	 elsecase {IsRecordC X}          then KindedRecordType
	 else                                 UnknownType
	 end
      elsecase {IsLazy X} then                LazyVarType
      else                                    UnboundType
      end
   end

   fun {MakeLines N}
      case N < 1 then ""
      else &\n | {MakeLines N-1} end
   end

   StackTag

   QueueLock = {NewLock}

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
	 EnvSync           : _
	 StatusSync        : _
	 MarkStackSync     : _
	 MarkEnvSync       : _

	 LastClicked       : unit

	 StackMsgList      : nil
	 EnvMsgList        : nil
	 StatusMsgList     : nil

	 StackMsgListTl    : nil
	 EnvMsgListTl      : nil
	 StatusMsgListTl   : nil

	 StackMsgListLn    : 0
	 EnvMsgListLn      : 0
	 StatusMsgListLn   : 0

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
			  tkInit(parent:           self.ButtonFrame
				 bitmap:           (OzcarBitmapDir # Bitmap #
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
%		     {Tk.send balloonhelp(B BalloonHelpText)}
		     B
		  end}
	 in
	    {ForAll
	     [self.emacsThreadsButton #
	      AddQueriesBitmap #
	      toggleEmacsThreads #
	      ConfigEmacsThreads #
	      'attach queries'

	      self.subThreadsButton #
	      AddSubThreadsBitmap #
	      toggleSubThreads #
	      ConfigSubThreads #
	      'attach subthreads']
	     proc {$ B}
		C # Xbm # Action # Default # _ = B
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
			    bitmap:           OzcarBitmapDir # Xbm
			    relief:           flat %raised
			    width:            CheckButtonWidth
			    height:           CheckButtonHeight
			    variable:         {New Tk.variable tkInit(Default)}
			    action:           self # Action)}
		{C tkBind(event:  HelpEvent
			  action: self # help(Xbm))}
%		{Tk.send balloonhelp(C BalloonHelpText)}
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
		  {OzcarMessage
		   'getEnv: using saved variables of frame #' # FrameId}
		  SavedVars
	       elsecase FrameId \= unit then
		  {OzcarMessage
		   'getEnv: requesting variables for frame #' # FrameId}
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

      meth ProcessClick(V)
	 {Browse V}
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
		W  = {Widget w($)}
		Ac = {New Tk.action
		      tkInit(parent: W
			     action: self # ProcessClick(ClickValue))}
	     in
		Gui,Enqueue(env o(W insert 'end'
				  {PrintF ' ' # PrintName {EnvVarWidth}}))
		Gui,Enqueue(env o(W insert 'end' PrintValue # '\n' T))
		Gui,Enqueue(env o(W tag bind T '<1>' Ac))
		Gui,Enqueue(env o(W tag conf T font:BoldFont))
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
	 WL  = {self.LocalEnvText  w($)}
	 WG  = {self.GlobalEnvText w($)}
	 SV  = {Cget envSystemVariables}
	 Y#G = case V == unit then
		  nil # nil
	       else
		  {Reverse V.'Y'} # {Reverse V.'G'}
	       end
      in
	 Gui,Enqueue(env {self.LocalEnvText resetTags($)})
	 Gui,Clear(env WL)
	 case V == unit then skip else
	    Gui,DoPrintEnv(self.LocalEnvText Y SV)
	 end
	 Gui,Disable(env WL)

	 Gui,Enqueue(env {self.GlobalEnvText resetTags($)})
	 Gui,Clear(env WG)
	 case V == unit then skip else
	    Gui,DoPrintEnv(self.GlobalEnvText G SV)
	 end
	 Gui,Disable(env WG)

	 Gui,ClearQueue(env)
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
	       {SendEmacs bar(file:F.file line:L column:F.column
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
	 W          = {self.StackText w($)}
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
	    Gui,Enable(stack W)
	    Gui,DeleteToEnd(stack W FrameNr+1)
	    Gui,DeleteLine(stack W FrameNr)
	 else skip end
	 case {IsSpecialFrameName FrameName} then
	    Gui,Enqueue(stack
			o(W insert LineEnd Arrow # FrameNr # ' ' # FrameName
			  q(StackTag LineActTag LineColTag)))
	 elsecase {IsDet FrameData} andthen FrameData == unit then
	    Gui,Enqueue(stack
			o(W insert LineEnd Arrow # FrameNr # ' {' # FrameName
			  q(StackTag LineActTag LineColTag)))
	 else
	    ProcTag    = {self.StackText newTag($)}
	    ProcAction = {New Tk.action
			  tkInit(parent: W
				 action: self # ProcessClick(FrameData))}
	 in
	    Gui,Enqueue(stack
			o(W insert LineEnd Arrow # FrameNr # ' {'
			  q(StackTag LineActTag LineColTag)))
	    Gui,Enqueue(stack
			o(W insert LineEnd FrameName
			  q(StackTag LineColTag ProcTag)))
	    Gui,Enqueue(stack o(W tag bind ProcTag '<1>' ProcAction))
	    Gui,Enqueue(stack o(W tag conf ProcTag font:BoldFont))
	 end
	 case FrameArgs of unit then
	    case {IsSpecialFrameName FrameName} then skip
	    else Gui,Enqueue(stack
			     o(W insert LineEnd ' ...'
			       q(StackTag LineActTag LineColTag)))
	    end
	 else
	    {ForAll FrameArgs
	     proc {$ Arg}
		P # V     = Arg
		ArgTag    = {self.StackText newTag($)}
		ArgAction = {New Tk.action
			     tkInit(parent: W
				    action: self # ProcessClick(V))}
	     in
		Gui,Enqueue(stack o(W insert LineEnd ' '
				    q(StackTag LineActTag LineColTag)))
		Gui,Enqueue(stack o(W insert LineEnd P
				    q(StackTag LineColTag ArgTag)))
		Gui,Enqueue(stack o(W tag bind ArgTag '<1>' ArgAction))
		Gui,Enqueue(stack o(W tag conf ArgTag font:BoldFont))
	     end}
	 end
	 Gui,Enqueue(stack
		     o(W insert LineEnd
		       case {IsSpecialFrameName FrameName} then
			  '' else '}'
		       end # case UpToDate then nil else
				' (source has changed)' end #
		       case Delete then '\n' else "" end
		       q(StackTag LineActTag LineColTag)))
	 Gui,Enqueue(stack o(W tag add  LineActTag LineEnd))% extend tag to EOL
	 Gui,Enqueue(stack o(W tag add  LineColTag LineEnd))% dito
	 Gui,Enqueue(stack o(W tag bind LineActTag '<1>' LineAction))
	 case Delete then
	    Gui,Disable(stack W)
	    Gui,Enqueue(stack o(W yview 'end'))
	    Gui,ClearQueue(stack)
	    Gui,FrameClick(frame:Frame highlight:false)
	 else skip end
      end

      meth printStack(id:I frames:Frames depth:Depth last:LastFrame<=nil)
	 W = {self.StackText w($)}
      in
	 Gui,Enqueue(stack {self.StackText resetTags($)})
	 Gui,Clear(stack W)
	 case I == 0 then   % clear stack and env windows; reset title
	    {self.StackText title(StackTitle)}
	    Gui,Disable(stack W)
	    Gui,ClearQueue(stack)
	    Gui,clearEnv
	 else
	    {self.StackText title(AltStackTitle # I)}
	    case Depth == 0 then
	       Gui,Append(stack W ' The stack is empty.')
	       Gui,Disable(stack W)
	       Gui,ClearQueue(stack)
	       Gui,clearEnv
	    else
	       Gui,Append(stack W {MakeLines Depth}) % Tk is _really_ stupid...
	       {ForAll Frames
		proc{$ Frame}
		   Gui,printStackFrame(frame:Frame delete:false)
		end}
	       Gui,Disable(stack W)
	       Gui,Enqueue(stack o(W yview 'end'))
	       Gui,ClearQueue(stack)
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
	 StatusSync <- _ = unit
	 case M == clear then
	    Gui,Clear(status W)
	 else
	    Gui,Enable(status W)
	 end
	 Gui,Append(status W S C)
	 Gui,Disable(status W)
	 Gui,ClearQueue(status)
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
	 New in
	 MarkStackSync <- New = unit
	 thread
	    {WaitOr New {Alarm TimeoutToMark}}
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
	    {WaitOr New {Alarm TimeoutToMark}}
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
	    Gui,markNode({Thread.id T} running)
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
	       Gui,doStatus(NoThreads)
	    else
	       Gui,DoAction(A)
	    end
	 end
      end

      meth DoAction(A)
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

	 elsecase A == StepButtonBitmap then T I in
	    T = @currentThread
	    I = {Thread.id T}
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
	    I = {Thread.id T}
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
	    I = {Thread.id T}
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
%                 Gui,doStatus('Already at end of procedure ' #
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
		  Gui,status('Unleashing thread #' # I #
			       ' to frame ' #
			       case LSF == 0 then 1 else LSF end)
		  {Thread.resume T}
	       end
	    end

	 elsecase A == StopButtonBitmap then T S in
	    T = @currentThread
	    S = {Thread.state T}
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
		  Gui,status('You have stopped thread #' # I)
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
	    I = {Thread.id T}
	    lock UserActionLock then ThreadManager,detach(T I) end

	 elsecase A == TermButtonBitmap then T I in
	    T = @currentThread
	    I = {Thread.id T}
	    ThreadManager,kill(T I)

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

      meth GetQueue(W $)
	 case W
	 of stack  then StackMsgList  # StackMsgListTl  # StackMsgListLn
	 [] env    then EnvMsgList    # EnvMsgListTl    # EnvMsgListLn
	 [] status then StatusMsgList # StatusMsgListTl # StatusMsgListLn
	 end
      end

      meth Enqueue(W Ticklet)
	 lock QueueLock then
	    case Ticklet
	    of nil  then skip
	    [] T|Tr then
	       Gui,Enqueue(W T)
	       Gui,Enqueue(W Tr)
	    else
	       NewTl
	       MsgList # MsgListTl # MsgListLn = Gui,GetQueue(W $)
	    in
	       case {IsDet @MsgListTl} then
		  MsgList <- Ticklet|NewTl
	       else
		  @MsgListTl = Ticklet|NewTl
	       end
	       MsgListTl <- NewTl

	       case @MsgListLn > 250 then % avoid overflow
		  {OzcarMessage W #
		   ' queue has become very big -- clearing...'}
		  Gui,ClearQueue(W)
	       else
		  MsgListLn <- @MsgListLn + 1
	       end
	    end
	 end
      end

      meth ClearQueue(W)
	 lock QueueLock then
	    MsgList # MsgListTl # MsgListLn = Gui,GetQueue(W $)
	 in
	    @MsgListTl = nil
	    {Tk.batch @MsgList}
	    MsgList   <- nil
	    MsgListLn <- 0
	 end
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

      meth Enable(W Widget)
	 Gui,Enqueue(W o(Widget conf state:normal))
      end

      meth Clear(W Widget)
	 Gui,Enqueue(W o(Widget conf state:normal))
	 Gui,Enqueue(W o(Widget delete p(0 0) 'end'))
      end

      meth Append(W Widget Text Color<=unit)
	 Gui,Enqueue(W o(Widget insert 'end' Text))
	 case Color == unit then skip else
	    Gui,Enqueue(W o(Widget conf fg:Color))
	 end
      end

      meth DeleteLine(W Widget Nr)
	 Gui,Enqueue(W o(Widget delete p(Nr 0) p(Nr 'end')))
      end

      meth DeleteToEnd(W Widget Nr)
	 Gui,Enqueue(W o(Widget delete p(Nr 0) 'end'))
      end

      meth Disable(W Widget)
	 Gui,Enqueue(W o(Widget conf state:disabled))
      end

      meth toTop
	 %% I am looking for a better method...
	 {Tk.batch [wm(iconify   self.toplevel)
		    wm(deiconify self.toplevel)]}
      end
   end
end
