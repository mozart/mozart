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
	  {ArgType X} # X
       end}
   end
   
   fun {ArgType X}
      case {IsDet X} then
	 case     {IsArray X}      then ArrayType
	 elsecase {IsThread X}     then ThreadType
	 elsecase {IsAtom X}       then case X
					of 'nil'         then "'nil'"
					[] '|'           then "'|'"
					[] '#'           then "'#'"
					[] 'unallocated' then UnAllocatedType
					else                  '\'' # X # '\''
					end
	 elsecase {IsBool X}       then case X then
					   'true'
					else
					   'false'
					end
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
      case N < 1 then nil
      else 10 | {MakeLines N-1} end
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
	 tkRunChildren
      
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

	 LastClicked : nil

      meth lastValue($)
	 try @LastClicked catch failure(...) then _ end
      end
      
      meth init
	 %% create the main window, but delay showing it
	 self.toplevel = {New Tk.toplevel tkInit(title:    TitleName
						 delete:   self # off
						 withdraw: true)}
	 {Tk.batch [wm(iconname   self.toplevel IconName)
		    wm(iconbitmap self.toplevel IconBitMap)
		    wm(geometry   self.toplevel ToplevelGeometry)]}
	 
	 Menu,init
	 Dialog,init
	 Help,init

	 {ForAll [self.ButtonFrame self.StatusFrame]
	  proc{$ F}
	     F = {New Tk.frame tkInit(parent: self.toplevel
				      bd:     SmallBorderSize
				      relief: ridge)}
	  end}
      
	 {Tk.batch [grid(self.menuBar       row:0 column:0
			 sticky:we columnspan:3)
		    grid(self.ButtonFrame   row:1 column:0
			 sticky:we columnspan:3)
		    grid(self.StatusFrame   row:6 column:0
			 sticky:we columnspan:3)
		   ]}
      
	 %% the buttons
	 local
	    Bs = {Map [StepButtonBitmap   # StepButtonColor
		       NextButtonBitmap   # NextButtonColor
		       ContButtonBitmap   # ContButtonColor
		       StopButtonBitmap   # StopButtonColor
		       ForgetButtonBitmap # ForgetButtonColor
		       TermButtonBitmap   # TermButtonColor]
		  fun {$ S}
		     Bitmap # ForegroundColor = S
		     B = {New Tk.button
			  tkInit(parent:           self.ButtonFrame
				 bitmap:           LocalBitMapDir # Bitmap #
				                               BitmapExtension
				 fg:               ForegroundColor
				 activeforeground: ForegroundColor
				 padx:             PadXButton
				 pady:             PadYButton
				 relief:           raised
				 borderwidth:      SmallBorderSize
				 action:           self # action(Bitmap))}
		  in
		     {B tkBind(event:  HelpEvent
			       action: self # help(Bitmap))}
		     B
		  end}
	    TkSusp TkRunChildren Susp RunChildren
	 in
	    {ForAll
	     [TkSusp       #Susp       #IgnoreFeedsBitmap  #suspend    #1
	      TkRunChildren#RunChildren#IgnoreThreadsBitmap#runChildren#1]
	     proc {$ B}
		V # C # Xbm # Action # Default = B
	     in
		V = {New Tk.variable tkInit(Default)}
		C = {New Tk.checkbutton
		     tkInit(parent:           self.ButtonFrame
			    %indicatoron:      false
			    %selectcolor:      CheckButtonSelectColor
			    fg:               ButtonForeground
			    activeforeground: ButtonForeground
			    variable:         V
			    bitmap:           LocalBitMapDir # Xbm
			    relief:           flat %raised
			    width:            CheckButtonWidth
			    height:           CheckButtonHeight
			    borderwidth:      SmallBorderSize
			    action:           self # Action(V))}
		{C tkBind(event:  HelpEvent
			  action: self # help(Xbm))}
	     end}
	     /*
	    {RunChildren tk(conf state:disabled)}
	     */
	    self.tkRunChildren = TkRunChildren
	    {Tk.batch [pack(b(Bs) side:left  padx:1)
		       pack(Susp RunChildren side:right padx:0)]}
	 end

	 %% border line
         local
	    F = {New Tk.frame tkInit(parent: self.toplevel
				     height: SmallBorderSize
				     bd:     NoBorderSize
				     relief: flat)}
	 in
	    {Tk.send grid(F row:2 column:0 sticky:we columnspan:3)}
	 end
 
	 %% status line
	 self.StatusText =
	 {New Tk.text tkInit(parent: self.StatusFrame
			     state:  disabled
			     height: 1
			     width:  0
			     bd:     NoBorderSize
			     cursor: TextCursor
			     font:   StatusFont)}
	 {self.StatusText tkBind(event:  HelpEvent
				 action: self # help(StatusHelp))}
	 {Tk.send pack(self.StatusText side:left padx:2 fill:x expand:yes)}

	 %% create the thread tree object...
	 self.ThreadTree =
	 {New Tree tkInit(parent: self.toplevel
			  title:  TreeTitle
			  bd:     SmallBorderSize
			  relief: sunken
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
						 bd:     SmallBorderSize
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

      meth DoPrintEnv(Widget Vars CV CP)
	 {ForAll Vars
	  proc{$ V}
	     AT = {ArgType V.2}
	  in
	     case V.1 == '' then skip
	     elsecase CV orelse {Atom.toString V.1}.1 \= 96 then
		case CP orelse AT \= ProcedureType then
		   case AT == UnAllocatedType then skip else
		      T  = {Widget newTag($)}
		      Ac = {New Tk.action
			    tkInit(parent: Widget
				   action: proc {$}
					      {Browse V.2} LastClicked <- V.2
					   end)}
		   in
		      {ForAll [tk(insert 'end' {PrintF ' ' # V.1 EnvVarWidth})
			       tk(insert 'end' AT # NL T)
			       tk(tag bind T '<1>' Ac)
			       tk(tag conf T font:BoldFont)] Widget}
		   end
		else skip end
	     else skip end
	  end}
      end

      meth printEnv(frame:I vars:V<=nil)
	 New in
	 EnvSync <- New = unit
	 thread
	    {WaitOr New {Alarm TimeoutToUpdateEnv}}
	    case {IsDet New} then skip else
	       {OzcarMessage 'printing environment of frame #' # I}
	       Gui,PrintEnv(frame:I vars:V)
	    end
	 end
      end
      
      meth PrintEnv(frame:I vars:V)
	 CV  = {Not {Cget envSystemVariables}}
	 CP  = {Not {Cget envProcedures}}
	 Y#G = case V == nil then
		  nil # nil
	       else
		  {Reverse V.'Y'} # {Reverse V.'G'}
	       end
      in
	 Gui,Clear(self.LocalEnvText)
	 Gui,Clear(self.GlobalEnvText)
      
	 case V == nil then skip else
	    Gui,DoPrintEnv(self.LocalEnvText  Y CV CP)
	    Gui,DoPrintEnv(self.GlobalEnvText G CV CP)
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
	 case L orelse CurThr == undef then skip
	 elsecase
	    {Dbg.checkStopped CurThr}
	 then     % allow switching of stack frames only if thread is stopped
	    FrameId       = F.id
	    FrameNr       = F.nr
	    SavedVars     = F.vars
	    Vars          = case SavedVars \= nil then
			       {OzcarMessage 'using saved variables'}
			       SavedVars
			    else
			       {OzcarMessage
				'requesting variables for frame id ' # F.id}
			       {Dbg.frameVars CurThr FrameId}
			    end
	 in
	    {OzcarMessage 'Selecting frame #' # FrameNr}
	    case Highlight then
	       SourceManager,delayedBar(file:F.file line:{Abs F.line})
	       Gui,SelectStackFrame(FrameNr)
	    else
	       Gui,SelectStackFrame(0)
	    end
	    Gui,printEnv(frame:FrameNr vars:Vars)
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
	 case Stack == undef then skip else
	    LSF = @LastSelectedFrame
	    N   = case LSF == 0 then ~1 else LSF + Delta end
	    F   = {Stack getFrame(N $)}
	 in
	    case F == nil then skip else
	       Gui,frameClick(frame:F highlight:true delay:false)
	    end
	 end
      end
      
      meth SelectStackFrame(T)
	 W   = self.StackText
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
	 W   = self.StackText
	 LSF = @LastSelectedFrame
      in
	 case LSF > 0 then
	    Gui,DeactivateLine(LSF)
	    LastSelectedFrame <- 0
	 else skip end
      end
      
      meth printStackFrame(frame:Frame delete:Delete<=true)
	 W          = self.StackText
	 FrameNr    = Frame.nr                 % frame number
	 FrameName  = Frame.name               % procedure/builtin name
	 FrameArgs  = {FormatArgs Frame.args}  % argument list
	 FrameFile  = {StripPath  Frame.file}
	 FrameLine  = {Abs Frame.line}
	 LineTag    = FrameNr
	 LineAction =
	 {New Tk.action
	  tkInit(parent: W
		 action: Ozcar # frameClick(frame:Frame))}
	 LineEnd    = FrameNr # DotEnd
	 UpToDate   = true %SourceManager,isUpToDate(Frame.time $)
      in
	 
	 {OzcarMessage '  printing frame #' # FrameNr}

	 lock
	    case Delete then
	       Gui,Enable(W)
	       Gui,DeleteToEnd(W FrameNr+1)
	       Gui,DeleteLine(W FrameNr)
	    else skip end
	    
	    {W tk(insert LineEnd
		  case Frame.dir == enter then
		     ' -> '
		  else
		     ' <- '
		  end # FrameNr #
		  case FrameName == suspension then
		     ' suspending conditional or procedure appl'
		  else
		     ' {' #
		     case FrameName == '' then '$' else FrameName end
		  end
		  q(StackTag LineTag))}
	    
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
	    
	    {ForAll [tk(insert LineEnd
			case FrameName == suspension then '' else '}' end #
			case UpToDate then nil else
			   ' (source has changed)' end #
			case Delete then NL else nil end
			q(StackTag LineTag))
		     tk(tag add  LineTag LineEnd) % extend tag to whole line
		     tk(tag bind LineTag '<1>' LineAction)] W}
	    
	    case Delete then
	       FrameDir = Frame.dir
	    in
	       Gui,Disable(W)
	       case FrameDir == enter then % should also work with 'leave' :-(
		  {W tk(yview 'end')}
		  Gui,frameClick(frame:Frame highlight:false)
	       else skip end
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
	       case Depth == 0 then   % empty stack? let's see...
		  case {Dbg.taskstack @currentThread 1} == nil then
		     Gui,Append(W ' The stack is empty.')  % yes, empty!
		  else                     % no, some fragments are there...
		     Gui,Append(W
				' There is not enough information to\n' #
				' display the stack here.\n\n' #
				' Try \'Browse\' from' #
				' the \'Stack\' menu instead.')
		  end
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
		     {OzcarError 'printStack: LastFrame == nil ??'}
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
	 Gui,printEnv(frame:0)
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
		     tk(tag conf StackTag foreground:OldStackColor)
		    ] self.StackText}
	 else skip end
      end
      
      meth action(A)
	 lock
	    case {IsName A} then skip else
	       {OzcarMessage 'action:' # A}
	    end
	    
	    case A
	    of !ResetAction then
	       N in
	       Gui,doStatus('Resetting...')
	       ThreadManager,killAll(N)
	       {Delay 200} %% just to look nice... ;)
	       Gui,doStatus(case N == 1 then
			       ' 1 thread killed'
			    else
			       ' ' # N # ' threads killed'
			    end append)
	       
	    [] !StepButtonBitmap then
	       T = @currentThread
	    in
	       case T == undef then
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
		     Gui,UnselectStackFrame
		     Gui,markNode({Thread.id T} running)
		     SourceManager,configureBar(running)
		     Gui,markStack(inactive)
		     {Thread.resume @currentThread}
		  end
	       end
	       
	    [] !NextButtonBitmap then
	       T = @currentThread
	    in
	       case T == undef then
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
		     ThreadDic = ThreadManager,getThreadDic($)
		     Stack     = try
				    {Dget ThreadDic I}
				 catch
				    system(kernel(dict ...) ...) then nil
				 end
		     TopFrame Dir
		  in
		     case Stack == nil then skip else
			TopFrame  = {Stack getTop($)}
			Dir       = case TopFrame == nil
				    then enter else TopFrame.dir end
			case Dir == leave then
			   {OzcarMessage NextOnLeave}
			else
			   {Dbg.stepmode T false}
			end
			
			Gui,UnselectStackFrame
			Gui,markNode(I running)
			Gui,markStack(inactive)
			SourceManager,configureBar(running)
			{Thread.resume T}
		     end
		  end
	       end
	       
	    [] !ContButtonBitmap then
	       T = @currentThread
	    in
	       case T == undef then
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
		     ThreadDic = ThreadManager,getThreadDic($)
		     Stack     = try
				    {Dget ThreadDic I}
				 catch
				    system(kernel(dict ...) ...) then nil
				 end
		  in
		     case Stack == nil then skip else
			
			{Dbg.stepmode T false}
			{Dbg.contflag T true}

			%% delete all tags
			{ForAll [resetReservedTags({Stack getSize($)})
				 resetTags] self.StackText}
			
			Gui,markNode({Thread.id T} running)
			Gui,markStack(inactive)
			Gui,doStatus('Continuing thread #' # I)
			SourceManager,configureBar(running)
			{Thread.resume T}
		     end
		  end
	       end

	    [] !StopButtonBitmap then
	       T = @currentThread
	    in
	       case T == undef then skip else
		  S = {Thread.state T}
	       in
		  case S == terminated then Gui,TerminatedStatus(T A) else
		     I         = {Thread.id T}
		     ThreadDic = ThreadManager,getThreadDic($)
		     Stack     = try
				    {Dget ThreadDic I}
				 catch
				    system(kernel(dict ...) ...) then nil
				 end
		  in
		     case
			Stack == nil then skip
		     elsecase
			{Dbg.checkStopped T} then Gui,StoppedStatus(I A)
		     else
			case S == blocked then
			   F L in
			   {Thread.suspend T}
			   {ForAll [rebuild(true) print
				    getPos(file:F line:L)] Stack}
			   SourceManager,bar(file:F line:L state:S)
			else
			   {Stack rebuild(true)}
			end
			{Dbg.contflag T false}
			{Dbg.stepmode T true}
			Gui,doStatus('You have stopped thread #' # I)
		     end
		  end
	       end
	       
	    [] !ForgetButtonBitmap then
	       T = @currentThread
	    in
	       case T == undef then skip else
		  I = {Thread.id T}
	       in
		  ThreadManager,forget(T I)
	       end
	       
	    [] !TermButtonBitmap then
	       T = @currentThread
	    in
	       case T == undef then skip else
		  I = {Thread.id T}
		  S = {CheckState T}
	       in
		  case S
		  of terminated then Gui,TerminatedStatus(T A)
		  else
		     ThreadManager,kill(T I)
		  end
	       end
	       
	    [] !StackAction then
	       T = @currentThread
	    in
	       case T == undef then
		  Gui,doStatus(FirstSelectThread)
	       else
		  S = {Filter {Dbg.taskstack T MaxStackBrowseSize}
		       fun {$ F} {Label F} \= debug end}
	       in
		  {Browse S}
	       end
	    end
	 end
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
	 {Widget tk(delete Nr#'.0' Nr#DotEnd)}
      end

      meth DeleteToEnd(Widget Nr)
	 {Widget tk(delete Nr#'.0' 'end')}
      end
   end
end
