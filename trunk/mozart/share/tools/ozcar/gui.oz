%%%
%%% Authors:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
%%%   Leif Kornstaedt, 2001
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

local

   fun {IsDefList Xs}
      {IsDet Xs} andthen case Xs of _|_ then true else false end
   end

   Approx = {NewUniqueName 'Approximation'}

   fun {IsApprox X}
      case X of Approx(...) then true else false end
   end

   fun {DisplayApprox X}
      case X
      of Approx(procedure A)  then A
      [] Approx(cell Y)       then '('#{DisplayName Y}#')'
      [] Approx(dictionary _) then DictionaryType
      [] Approx(fdvar)        then FDVarType
      [] Approx(fsvar)        then FSVarType
      [] Approx(recordc)      then KindedRecordType
      [] Approx(future)       then FutureType
      [] Approx(failed)       then FailedType
      [] Approx(free)         then UnboundType
      [] Approx(A)            then '<'#A#'>'
      else                         UnknownType
      end
   end

   fun {DisplayName X}
      case {Value.status X}
      of det(int)   andthen X >= BigInt   then BigIntType
      [] det(int)                         then X
      [] det(float) andthen X >= BigFloat then BigFloatType
      [] det(float)                       then {V2VS X}
      [] det(tuple) andthen {IsApprox X}  then {DisplayApprox X}
      [] det(tuple) andthen {IsDefList X} then ListType
      [] det(tuple)                       then TupleType
      [] det(record)                      then RecordType
      [] det(atom)                        then {Value.toVirtualString X 0 0}
      [] det(name)  andthen X == Approx   then UnknownType
      [] det(name)  andthen {IsBool X}    then {System.printName X}
      [] det(name)  andthen {IsUnit X}    then UnitType
      [] det(name)                        then NameType
      [] det(procedure)                   then ProcedureType
      [] det(cell)                        then '('#{DisplayName {Access X}}#')'
      [] det(chunk)                       then ChunkType
      [] det(array)                       then ArrayType
      [] det(dictionary)                  then DictionaryType
      [] det('class')                     then ClassType
      [] det(object)                      then ObjectType
      [] det('lock')                      then LockType
      [] det(port)                        then PortType
      [] det(space)                       then SpaceType
      [] det('thread')                    then ThreadType
      [] det(Y)                           then '<'#Y#'>'
      [] kinded(int)                      then FDVarType
      [] kinded(fset)                     then FSVarType
      [] kinded(record)                   then KindedRecordType
      [] future                           then FutureType
      [] failed                           then FailedType
      [] free                             then UnboundType
      else                                     UnknownType
      end
   end

   fun {FormatArgs A}
      {Map A
       fun {$ X}
	  if {Cget envPrintTypes} then
	     {DisplayName X} # X
	  else
	     {V2VS X} # X
	  end
       end}
   end

   fun {MakeLines N}
      if N < 1 then ""
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

      meth lastClickedValue($)
	 @LastClicked
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
		    wm(minsize    self.toplevel MinX MinY)
		    wm(geometry   self.toplevel ToplevelGeometry)]}

	 Menu,init

	 for F in [self.ButtonFrame self.StatusFrame] do
	    F = {New Tk.frame tkInit(parent: self.toplevel
				     bd:     1
				     relief: ridge)}
	 end

	 {Tk.batch [grid(self.menuBar       row:0 column:0
			 sticky:we columnspan:3)
		    grid(self.ButtonFrame   row:1 column:0
			 sticky:we columnspan:3)
		    grid(self.StatusFrame   row:6 column:0
			 sticky:we columnspan:3)
		   ]}

	 local
	    Bs = {Map [StepButtonBitmap    # StepButtonColor
		       NextButtonBitmap    # NextButtonColor
		       UnleashButtonBitmap # UnleashButtonColor
		       StopButtonBitmap    # StopButtonColor
		       DetachButtonBitmap  # DetachButtonColor
		       TermButtonBitmap    # TermButtonColor]
		  fun {$ Bitmap # ForegroundColor}
		     B = {New Tk.button
			  tkInit(parent: self.ButtonFrame
				 bitmap: {OzcarBitmap Bitmap}
				 fg:     ForegroundColor
				 activeforeground:
				    if UseColors then ForegroundColor
				    else SelectedForeground
				    end
				 relief: raised
				 action: self # action(Bitmap))}
		  in
		     {B tkBind(event:  HelpEvent
			       action: self # help(Bitmap))}
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
						       Gui,SetEmacsThreads(S)
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
			  title:  TreeTitle
			  relief: sunken
			  bd:     1
			  width:  ThreadTreeWidth
			  bg:     DefaultBackground)}
	 {self.ThreadTree tkBind(event:  HelpEvent
				 action: self # help(TreeTitle))}

	 %% ...and the text widgets for stack and environment
	 for W#Title#Width in
	    [self.StackText       # StackTitle      # StackTextWidth
	     self.LocalEnvText    # LocalEnvTitle   # EnvTextWidth
	     self.GlobalEnvText   # GlobalEnvTitle  # EnvTextWidth]
	 do
	    W = {New ScrolledTitleText tkInit(parent: self.toplevel
					      title:  Title
					      wrap:   none
					      state:  disabled
					      width:  Width
					      cursor: TextCursor
					      font:   DefaultFont
					      bg:     DefaultBackground)}
	    {W tkBind(event:  HelpEvent
		      action: self # help(Title))}
	 end

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

      meth SetEmacsThreads(S)
	 {EnqueueCompilerQuery setSwitch(runwithdebugger (S == AttachText))}
      end

      meth checkSubThreads($)
	 {self.subThreadsMenu getCurrent($)}
      end

      meth getEnv(Frame $)
	 NullEnv = v('Y': nil 'G': nil)
      in
	 if @currentStack == unit then NullEnv
	 else
	    F = case Frame of unit then
		   case @LastSelectedFrame of 0 then {@currentStack getTop($)}
		   elseof LSF then {@currentStack getFrame(LSF $)}
		   end
		else Frame
		end
	 in
	    case F of unit then NullEnv
	    else
	       FrameId   = F.frameID
	       SavedVars = F.vars
	    in
	       if SavedVars \= unit then
		  {OzcarMessage 'getEnv: frame '#F.nr#' has saved variables'}
		  SavedVars
	       elseif FrameId \= unit then
		  {OzcarMessage 'getEnv: requesting variables for frame '#F.nr}
		  case {Primitives.threadState @currentThread}
		  of terminated then NullEnv
		  elsecase {Primitives.getEnvironment @currentThread FrameId}
		  of unit then NullEnv % `FrameId' was an invalid frame id...
		  elseof V then V
		  end
	       else NullEnv
	       end
	    end
	 end
      end

      meth ProcessClick(V)
	 {Inspect V}
	 LastClicked <- V
      end

      meth updateEnv
	 Gui,MarkEnv(active)
	 Gui,PrintEnv(Gui,getEnv(unit $))
      end

      meth printEnv(V)
	 if {Cget updateEnv} then New in
	    EnvSync <- New = unit
	    Gui,MarkEnv(active)
	    thread
	       {WaitOr New {Alarm {Cget timeoutToUpdateEnv}}}
	       if {IsFree New} then
		  Gui,PrintEnv(V)
	       end
	    end
	 else
	    Gui,MarkEnv(inactive)
	 end
      end

      meth PrintEnv(V)
	 SV = {Cget envSystemVariables}
      in
	 {self.LocalEnvText resetTags}
	 Gui,Clear(self.LocalEnvText)
	 if V \= unit then
	    Gui,DoPrintEnv(self.LocalEnvText V.'Y' SV)
	 end
	 Gui,Disable(self.LocalEnvText)

	 {self.GlobalEnvText resetTags}
	 Gui,Clear(self.GlobalEnvText)
	 if V \= unit then
	    Gui,DoPrintEnv(self.GlobalEnvText V.'G' SV)
	 end
	 Gui,Disable(self.GlobalEnvText)
      end

      meth DoPrintEnv(Widget Vars SV)
	 for V in Vars do
	    Name0 # Value = V
	    Name = if {IsName Name0} then
		      {VirtualString.toAtom {V2VS Name0}}
		   else Name0 end
	    PrintName # PrintValue # ClickValue =
	    if {Cget envPrintTypes} then
	       Name # {DisplayName Value} # Value
	    else
	       if {IsDet Value} andthen {IsCell Value} then
		  X = {Access Value}
	       in
		  {VirtualString.toAtom
		   '{Access ' # Name # '}'} # {V2VS X} # X
	       else
		  Name # {V2VS Value} # Value
	       end
	    end
	 in
	    if SV orelse {Atom.toString Name}.1 \= &` then
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
	    end
	 end
      end

      meth FrameClick(frame:F highlight:Highlight<=true)
	 if @currentThread \= unit then
	    Stopped = case {Primitives.threadState @currentThread}
		      of stoppedRunnable then true
		      [] stoppedBlocked  then true
		      else false
		      end
	 in
	    if Stopped then
	       %% allow switching of stack frames only if thread is stopped
	       if Highlight then
		  L = case F.line of unit then unit else {Abs F.line} end
	       in
		  {SendEmacs delayedBar(file:F.file line:L column:F.column)}
		  Gui,SelectStackFrame(F.nr)
	       else
		  Gui,SelectStackFrame(0)
	       end
	       Gui,printEnv(Gui,getEnv(F $))
	    else %% thread is running
	       Gui,MarkEnv(inactive)
	    end
	 end
      end

      meth previousThread
	 case {self.ThreadTree previous($)} of unit then skip
	 elseof T then
	    Gui,status('Selected thread ' # {Primitives.getThreadName T})
	    ThreadManager,switch(T)
	 end
      end

      meth nextThread
	 case {self.ThreadTree next($)} of unit then skip
	 elseof T then
	    Gui,status('Selected thread ' # {Primitives.getThreadName T})
	    ThreadManager,switch(T)
	 end
      end

      meth neighbourStackFrame(Delta)
	 case @currentStack of unit then skip
	 elseof Stack then
	    LSF = @LastSelectedFrame
	    N   = if LSF == 0 then ~1 else LSF + Delta end
	    F   = {Stack getFrame(N $)}
	 in
	    if F \= unit then
	       Gui,FrameClick(frame:F)
	    end
	 end
      end

      meth SelectStackFrame(T)
	 LSF = @LastSelectedFrame
      in
	 if LSF \= T then
	    if LSF \= 0 then
	       Gui,DeactivateLine(LSF)
	    end
	    if T \= 0 then
	       Gui,ActivateLine(T)
	    end
	    LastSelectedFrame <- T
	 end
      end

      meth unselectStackFrame
	 LSF = @LastSelectedFrame
      in
	 if LSF \= 0 then
	    Gui,DeactivateLine(LSF)
	 end
	 LastSelectedFrame <- 0
      end

      meth printStackFrame(frame:Frame delete:Delete)
	 W          = self.StackText
	 FrameNr    = Frame.nr
	 FrameName  = case Frame.name of ''  then '$'
		      elseof N then N
		      end
	 FrameData  = Frame.data
	 FrameArgs  = case Frame.args of unit then unit
		      else {FormatArgs Frame.args}  %% argument list
		      end
	 LineColTag = FrameNr       %% no need to garbage collect this tag
	 LineActTag = act # FrameNr %% dito
	 LineAction = {New Tk.action
		       tkInit(parent: W
			      action: self # FrameClick(frame:Frame))}
	 LineEnd    = p(FrameNr 'end')
	 Arrow      = if Frame.dir == entry then ' -> ' else ' <- ' end
      in
	 if Delete then
	    Gui,Enable(W)
	    Gui,DeleteToEnd(W FrameNr+1)
	    Gui,DeleteLine(W FrameNr)
	 end
	 if Frame.kind \= 'call' then
	    {W tk(insert LineEnd Arrow # FrameNr # ' ' # FrameName
		  q(StackTag LineActTag LineColTag))}
	 elseif {IsDet FrameData} andthen FrameData == unit then
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
	    if Frame.kind == 'call' then
	       {W tk(insert LineEnd ' ...' q(StackTag LineActTag LineColTag))}
	    end
	 else
	    for Arg in FrameArgs do
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
	    end
	 end
	 {W tk(insert LineEnd
	       if Frame.kind \= 'call' then '' else '}' end #
	       if Delete then '\n' else "" end
	       q(StackTag LineActTag LineColTag))}
	 {W tk(tag add  LineActTag LineEnd)} % extend tag to EOL
	 {W tk(tag add  LineColTag LineEnd)} % dito
	 {W tk(tag bind LineActTag '<1>' LineAction)}
	 if Delete then
	    Gui,Disable(W)
	    {W tk(yview 'end')}
	    Gui,FrameClick(frame:Frame highlight:false)
	 end
      end

      meth printStack(thr:T frames:Frames depth:Depth<=0 last:LastFrame<=unit)
	 W = self.StackText
      in
	 {W resetTags}
	 Gui,Clear(W)
	 {self.StackText title(AltStackTitle # {Primitives.getThreadName T})}
	 if Depth == 0 then
	    case @currentThread of unit then
	       {Delay {Cget timeoutToSwitch}} % timing analysis is tricky...
	    else skip
	    end
	    case @currentThread of unit then
	       {OzcarError 'Gui,printStack: @currentThread == unit'}
	    elseof T then
	       Empty = case {Primitives.threadState T}
		       of terminated then true
		       [] runnable   then true
		       [] blocked    then true
		       else false
		       end
	    in
	       if Empty then
		  Gui,Append(W ' The stack is empty.')
	       else
		  Gui,Append(W (' No stack computed yet;\n' #
				' stop thread to compute one'))
	       end
	       Gui,Disable(W)
	       Gui,ClearEnv
	    end
	 else
	    Gui,Append(W {MakeLines Depth}) % Tk is _really_ stupid...
	    for Frame in Frames do
	       Gui,printStackFrame(frame:Frame delete:false)
	    end
	    Gui,Disable(W)
	    {W tk(yview 'end')}
	    case LastFrame of unit then
	       {OzcarError 'Gui,printStack: LastFrame == unit'}
	    else
	       Gui,FrameClick(frame:LastFrame highlight:false)
	    end
	 end
      end

      meth clearStack
	 W = self.StackText
      in
	 {W resetTags}
	 Gui,Clear(W)
	 {self.StackText title(StackTitle)}
	 Gui,Disable(W)
	 Gui,ClearEnv
      end

      meth ClearEnv
	 Gui,printEnv(unit)
      end

      meth selectNode(T)
	 {self.ThreadTree select(T)}
      end

      meth markNode(T S)
	 {self.ThreadTree mark(T S)}
      end

      meth addNode(T S)
	 {self.ThreadTree add(T S)}
      end

      meth removeNode(T $)
	 {self.ThreadTree remove(T $)}
      end

      meth status(S M<=clear C<=DefaultForeground)
	 case M of clear then
	    {self.StatusText replace(S C)}
	 [] append then
	    {self.StatusText append(S C)}
	 end
      end

      meth AssertRunning(B A $) T in
	 T = @currentThread
	 case {@currentStack getException($)} of unit then
	    case {Primitives.threadState T} of terminated then
	       Gui,status('Thread ' # {Primitives.getThreadName T} #
			  ' is dead, ' # A # ' has no effect')
	       false
	    elseof State then
	       Running = case State
			 of runnable        then true
			 [] blocked         then true
			 [] stoppedRunnable then false
			 [] stoppedBlocked  then false
			 end
	    in
	       case B#Running
	       of true#false then
		  Gui,status('Thread ' # {Primitives.getThreadName T} #
			     ' is not running, ' # A # ' has no effect')
		  false
	       [] false#true then
		  Gui,status('Thread ' # {Primitives.getThreadName T} #
			     ' is already running, ' # A # ' has no effect')
		  false
	       else true
	       end
	    end
	 else
	    Gui,status('Thread ' # {Primitives.getThreadName T} #
		       ' got an unhandled exception, ' # A # ' has no effect')
	    false
	 end
      end

      meth markStack(How) New in
	 MarkStackSync <- New = unit
	 thread
	    {WaitOr New {Alarm TimeoutToUpdate*10}}
	    if {IsFree New} then
	       Gui,DoMarkStack(How)
	    end
	 end
      end

      meth DoMarkStack(How)
	 {self.StackText tk(tag 'raise' StackTag)}
	 {self.StackText tk(tag conf StackTag
			    foreground:case How
				       of active   then DefaultForeground
				       [] inactive then DirtyColor
				       end)}
      end

      meth MarkEnv(How) New in
	 MarkEnvSync <- New = unit
	 thread
	    {WaitOr New {Alarm TimeoutToUpdate*10}}
	    if {IsFree New} then
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

      meth ContinueTo(T Frame DoStep)
	 {@currentStack setAtBreakpoint(false)}
	 Gui,unselectStackFrame
	 Gui,markStack(inactive)
	 Gui,MarkEnv(inactive)
	 {SendEmacs configureBar(running)}
	 case Frame.frameID of unit then
	    {Primitives.unleash T 0 DoStep}
	 elseof FrameID then
	    {Primitives.unleash T FrameID DoStep}
	 end
	 Gui,markNode(T {Primitives.threadState T})
      end

      meth action(A)
	 lock
	    {Wait @switchDone}
	    if ThreadManager,emptyForest($) then
	       Gui,status(NoThreads)
	    else
	       Gui,DoAction(A)
	    end
	 end
      end

      meth DoAction(A)
	 {OzcarMessage 'action: '#{System.printName A}}

	 case A of !TermAllAction then
	    Gui,status('Terminating all threads...')
	    ThreadManager,termAll
	    Gui,status(' done' append)

	 [] !TermAllButCurAction then
	    Gui,status('Terminating all threads but current...')
	    ThreadManager,termAllButCur
	    Gui,status(' done' append)

	 [] !DetachAllAction then
	    Gui,status('Detaching all threads...')
	    ThreadManager,detachAll
	    Gui,status(' done' append)

	 [] !DetachAllButCurAction then
	    Gui,status('Detaching all threads but current...')
	    ThreadManager,detachAllButCur
	    Gui,status(' done' append)

	 [] !DetachAllDeadAction then
	    Gui,status('Detaching all dead threads...')
	    ThreadManager,detachAllDead
	    Gui,status(' done' append)

	 [] !StepButtonBitmap then
	    if Gui,AssertRunning(false StepInto $) then
	       case {@currentStack getTop($)} of unit then skip
	       elseof TopFrame then
		  Gui,status('')
		  Gui,ContinueTo(@currentThread TopFrame true)
	       end
	    end

	 [] !NextButtonBitmap then
	    if Gui,AssertRunning(false StepOver $) then
	       case {@currentStack getTop($)} of unit then skip
	       elseof TopFrame then
		  Gui,status('')
		  Gui,ContinueTo(@currentThread TopFrame TopFrame.dir \= exit)
	       end
	    end

	 [] !UnleashButtonBitmap then
	    if Gui,AssertRunning(false A $) then
	       Stk = @currentStack
	       LSF = @LastSelectedFrame
	    in
	       case {Stk getFrame(LSF $)} of unit then skip
	       elseof Frame then T in
		  T = @currentThread
		  {Stk rebuild(true)}
		  Gui,status('Unleashing thread ' #
			     {Primitives.getThreadName T} #
			     ' to frame ' # {Max 1 LSF})
		  Gui,ContinueTo(T Frame false)
	       end
	    end

	 [] !StopButtonBitmap then
	    if Gui,AssertRunning(true A $) then T in
	       T = @currentThread
	       {Primitives.suspend T}
	       case {Primitives.threadState T} of stoppedBlocked then F L C in
		  Gui,status('You have stopped thread ' #
			     {Primitives.getThreadName T})
		  Gui,markNode(T stoppedBlocked)
		  {@currentStack rebuild(true)}
		  {@currentStack print}
		  {@currentStack getPos(file:?F line:?L column:?C)}
		  {SendEmacsBar F L C stoppedBlocked}
	       else
		  Gui,status('')
		  {Primitives.unleash T 0xFFFFFF true}
	       end
	    end

	 [] !DetachButtonBitmap then
	    lock UserActionLock then ThreadManager,detach(@currentThread) end

	 [] !TermButtonBitmap then
	    ThreadManager,kill(@currentThread true)

	 end
      end

      meth toggleEmacs
	 if {Cget useEmacsBar} then
	    Gui,status('Not using Emacs Bar')
	    {SendEmacs removeBar}
	 else
	    Gui,status('Using Emacs Bar')
	 end
	 {Ctoggle useEmacsBar}
      end

      meth toggleUpdateEnv
	 if {Cget updateEnv} then
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
	 if Color \= unit then
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
