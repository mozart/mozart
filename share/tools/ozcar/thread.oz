%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   fun {AppOK Name}
      ({Cget stepRecordBuiltin}  orelse Name \= 'record')
      andthen
      ({Cget stepDotBuiltin}     orelse Name \= '.')
      andthen
      ({Cget stepWidthBuiltin}   orelse Name \= 'Width')
      andthen
      ({Cget stepNewNameBuiltin} orelse Name \= 'NewName')
      andthen
      ({Cget stepSetSelfBuiltin} orelse Name \= 'setSelf')
      andthen
      ({Cget stepSystemProcedures} orelse
       Name == '`,`'      orelse
       Name == '`send`'   orelse
       Name == '`ooSend`' orelse
       Name \= '' andthen {Atom.toString Name}.1 \= &`)
   end

   proc {OzcarReadEvalLoop S}
      case S
      of H|T then
	 {OzcarMessage 'readloop:'} {OzcarShow H}
	 {Ozcar readStreamMessage(H)}
	 {OzcarMessage 'preparing for next stream message...'}
	 {OzcarReadEvalLoop T}
      end
   end

in

   class ThreadManager
      feat
	 ThreadDic             %% dictionary that holds various information
			       %% about debugged threads
	 ReadLoopThread

      attr
	 currentThread : undef
	 currentStack  : undef
	 SkippedProcs  : nil
	 SkippedThread : nil
	 Breakpoint    : false

	 SwitchSync    : _

      meth init
	 self.ThreadDic = {Dnew}
	 thread
	    self.ReadLoopThread = {Thread.this}
	    {OzcarReadEvalLoop {Dbg.stream}}
	 end
      end

      meth checkMe
	 T = @currentThread
      in
	 case T == undef then
	    Gui,doStatus('There is no thread selected')
	 else
	    I = {Thread.id T}
	    R = case {Dbg.checkStopped T} then stopped else running end
	    S = {Thread.state T}
	 in
	    Gui,doStatus('Currently selected thread: #' # I #
			 ' (' # R # ' / ' # S # ')')
	 end
      end

      meth getThreadDic($)
	 self.ThreadDic
      end

      meth AddToSkippedProcs(Name T I FrameId)
	 Key = FrameId # I
      in
	 SkippedProcs <- Key | @SkippedProcs
	 {OzcarMessage 'Skipping procedure \'' # Name # '\''}
	 {OzcarShow @SkippedProcs}
	 {Thread.resume T}
      end

      meth readStreamMessage(M)

	 case M

	 of entry(thr:T ...) then
	    I = {Thread.id T}
	 in
	    case ThreadManager,Exists(I $) then
	       Name = {CondSelect M data unit}
	    in
	       case {Not {IsDet Name}}
		  orelse {AppOK {System.printName Name}}
	       then
		  ThreadManager,M
	       else
		  ThreadManager,AddToSkippedProcs(Name T I M.frameID)
	       end

	    else
	       ForMe = case {CondSelect M data unit} \= `ooSend` then false
		       elsecase {CondSelect M args nil} of [_ _ Obj] then
			  {IsDet Obj} andthen Obj == self
		       else false
		       end
	    in
	       case ForMe then
		  {Dbg.trace T false}
		  {Dbg.stepmode T false}
		  {Thread.resume T}
		  {OzcarMessage 'message for Ozcar detected.'}

	       elsecase {UnknownFile M.file} then
		  {Dbg.trace T false}
		  {Dbg.stepmode T false}
		  {Thread.resume T}
		  Gui,status(IgnoreNoFileStep)

	       else
		  {OzcarMessage WaitForThread}
		  {Delay 240} % thread should soon be added
		  case @Breakpoint then
		     Breakpoint <- false
		  else
		     ThreadManager,M
		  end
	       end
	    end

	 elseof exit(thr:T frameID:FrameId ...) then
	    I     = {Thread.id T}
	    Key   = FrameId # I
	    Found = {Member Key @SkippedProcs}
	 in
	    {OzcarShow @SkippedProcs # Key # Found}
	    case Found then
	       {OzcarMessage 'ignoring exit message for ignored application'}
	       SkippedProcs  <- {Filter @SkippedProcs fun {$ F} F \= Key end}
	       SkippedThread <- nil
	       {Thread.resume T}
	    elsecase @SkippedThread == T then
	       {OzcarMessage 'ignoring exit message for ignored thread'}
	       SkippedThread <- nil
	       {Thread.resume T}
	    else
	       Gui,markNode(I runnable) % thread is not running anymore
	       Gui,markStack(active)    % stack view has up-to-date content
	       case T == @currentThread then
		  Stack = {Dget self.ThreadDic I}
	       in
		  {Stack exit(M)}
		  SourceManager,bar(file:{CondSelect M file nofile}
				    line:{CondSelect M line unit}
				    column:{CondSelect M column unit}
				    state:runnable)
		  {Stack printTop}
	       else skip end
	    end

	 elseof thr(thr:T ...) then   % thread creation
	    I = {Thread.id T}
	    Q = case {CondSelect M par unit} of unit then 0
		elseof T then {Thread.id T}
		end
	    E = ThreadManager,Exists(I $)
	 in
	    case E then
	       Stack = {Dget self.ThreadDic I}
	    in
	       Gui,status('Thread #' # I # ' has reached a breakpoint') % #
			  %' or woke up another thread')
	       {OzcarMessage KnownThread # {ID I}}
	       {Stack rebuild(true)}
	    else
	       {OzcarMessage NewThread   # {ID I}}
	       case Q == 0 orelse Q == 1 then   % unknown or root thread
		  thread
		     case Q == 1 then   % root thread
			SkippedThread <- T
			{Thread.resume T}
		     else skip end

		     {Delay 170}

		     case Q == 1 then
			SkippedThread <- nil
		     else skip end

		     case {Thread.state T} == terminated then
			{OzcarMessage EarlyThreadDeath # I}
		     else
			ThreadManager,add(T I Q (Q == 0))
		     end
		  end
	       else
		  ThreadManager,add(T I Q false)
	       end
	    end

	 elseof term(thr:T) then
	    I = {Thread.id T}
	    E = ThreadManager,Exists(I $)
	 in
	    case E then
	       ThreadManager,remove(T I noKill)
	    else
	       {OzcarMessage EarlyTermThread}
	    end

	 [] blocked(thr:T) then
	    I = {Thread.id T}
	 in
	    case ThreadManager,Exists(I $) then
	       ThreadManager,blocked(thr:T id:I)
	    else
	       thread
		  {OzcarMessage WaitForThread}
		  {Delay TimeoutToBlock} % thread should soon be added
		  case ThreadManager,Exists(I $) then
		     ThreadManager,blocked(thr:T id:I)
		  else
		     {OzcarError UnknownSuspThread}
		  end
	       end
	    end

	 [] ready(thr:T) then
	    I = {Thread.id T}
	 in
	    case ThreadManager,Exists(I $) then
	       case {Dbg.checkStopped T} then
		  Gui,markNode(I runnable)
		  case T == @currentThread then
		     SourceManager,configureBar(runnable)
		     %Gui,doStatus('Thread #' # I # ' is runnable again')
		  else skip end
	       else
		  Gui,markNode(I running)
	       end
	    else
	       {OzcarError UnknownWokenThread}
	    end

	 [] exception(thr:T exc:X) then
	    I = {Thread.id T}
	 in
	    case ThreadManager,Exists(I $) then
	       {{Dget self.ThreadDic I} printException(X)}
	    else
	       thread
		  {OzcarMessage 'exception of unknown thread -- waiting...'}
		  {Delay 320}
		  case ThreadManager,Exists(I $) then
		     {OzcarMessage 'ok, got it -- printException'}
		     {Delay 140}
		     {{Dget self.ThreadDic I} printException(X)}
		  else
		     {OzcarMessage 'still not known -- adding...'}
		     ThreadManager,add(T I exc(X) false)
		  end
	       end
	    end

	 else
	    {OzcarMessage UnknownMessage}
	 end
      end

      %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

      meth Exists(I $)
	 {Dmember self.ThreadDic I}
      end

      meth EmptyTree($)
	 {Dkeys self.ThreadDic} == nil
      end

      meth removeSkippedProcs(I)
	 SkippedProcs <- {Filter @SkippedProcs
			  fun {$ F} F.2 \= I end}
      end

      meth add(T I Q R)
	 Stack = {New StackManager init(thr:T id:I)}
      in
	 {Dput self.ThreadDic I Stack}
	 case R then
	    {Stack rebuild(true)}
	    Breakpoint <- true
	 else skip end

	 case Q of exc(X) then  %% exception
	    Gui,addNode(I 0)
	    {Stack checkNew(_)} %% _don't_ do a first step here!
	    ThreadManager,switch(I false)
	    {Stack printException(X)}
	 elsecase {IsInt Q} then   %% Q is the ID of the parent thread
	    Gui,addNode(I Q)
	    case Q == 0 orelse Q == 1 then
	       {Stack checkNew(_)}  % don't force a first `step'
	       ThreadManager,switch(I)
	       case Q == 1 then
		  Gui,status('Got new query, selecting thread #' # I)
	       else
		  Gui,status('Breakpoint reached by thread #' # I #
				', which has been added and selected')
	       end
	    else skip end
	 end
      end

      meth remove(T I Mode Select<=true)
	 Next in
	 {OzcarMessage 'removing thread #' # I # ' with mode ' # Mode}
	 ThreadManager,removeSkippedProcs(I)
	 case Mode == kill then
	    Gui,killNode(I Next)
	    {Dremove self.ThreadDic I}
	    case ThreadManager,EmptyTree($) then
	       currentThread <- undef
	       currentStack  <- undef
	       SourceManager,removeBar
	       Gui,selectNode(0)
	       Gui,clearStack
	       case Select then
		  Gui,status(', thread tree is now empty' append)
	       else skip end
	    else skip end
	 else
	    Gui,removeNode(I)
	 end
	 case T == @currentThread then
	    case Mode == kill then
	       case ThreadManager,EmptyTree($) then skip else
		  case Select then
		     currentThread <- undef % to ignore Gui actions temporarily
		     ThreadManager,switch(Next)
		     Gui,status(', new selected thread is #' # Next append)
		  else skip end
	       end
	    else
	       Gui,status('Thread #' # I # ' died')
	       SourceManager,removeBar
	       Gui,printStack(id:I frames:nil depth:0)
	    end
	 else skip end
      end

      meth kill(T I Select<=true)
	 lock
	    {Dbg.trace T false}
	    {Thread.terminate T}
	    case Select then
	       Gui,doStatus(TerminateMessage # I # TerminateMessage2)
	    else skip end
	    ThreadManager,remove(T I kill Select)
	 end
      end

      meth killAll($)
	 E = {Ditems self.ThreadDic}
	 DeleteCount = {Length E}
      in
	 {ForAll E
	  proc {$ S}
	     I = {S getId($)}
	     T = {S getThread($)}
	  in
	     ThreadManager,kill(T I false)
	     Gui,doStatus('.' append)
	  end}
	 DeleteCount
      end

      meth forget(T I)
	 lock
	    {Dbg.trace T false}      %% thread is not traced anymore
	    {Dbg.stepmode T false}   %% no step mode, run as you like!
	    {Thread.resume T}        %% run, run to freedom!! :-)
	    Gui,doStatus(ForgetMessage # I # ForgetMessage2)
	    ThreadManager,remove(T I kill)
	 end
      end

      meth entry(thr: T ...)=Frame
	 I     = {Thread.id T}
	 Stack = {Dget self.ThreadDic I}
      in
	 Gui,markNode(I runnable) % thread is not running anymore
	 Gui,markStack(active)    % stack view has up-to-date content
	 {Stack entry(Frame)}
	 case T == @currentThread then
	    F = {CondSelect Frame file nofile}
	 in
	    case {UnknownFile F} then
	       {OzcarMessage NoFileInfo # I}
	       SourceManager,removeBar
	       {Thread.resume T}
	    else
	       L = {CondSelect Frame line unit}
	       C = {CondSelect Frame column unit}
	    in
	       SourceManager,bar(file:F line:L column:C state:runnable)
	       {Stack printTop}
	    end
	 else skip end
      end

      meth blocked(thr:T id:I)
	 Gui,markNode(I blocked)
      end

      meth rebuildCurrentStack
	 Stack = @currentStack
      in
	 case Stack == undef then
	    Gui,doStatus(FirstSelectThread)
	 else
	    Gui,doStatus(RebuildMessage # {Thread.id @currentThread} # '...')
	    {Stack rebuild(true)}
	    {Stack print}
	    Gui,doStatus(DoneMessage append)
	 end
      end

      meth switch(I PrintStack<=true)
	 New in
	 SwitchSync <- New = unit

	 Gui,selectNode(I)

	 thread
	    lock
	       {WaitOr New {Alarm TimeoutToSwitch}}
	       case {IsDet New} then skip else
		  ThreadManager,DoSwitch(I PrintStack)
	       end
	    end
	 end
      end

      meth DoSwitch(I PrintStack)
	 case I == 1 then skip else
	    Stack = {Dget self.ThreadDic I}
	    T     = {Stack getThread($)}
	    S     = {CheckState T}
	 in

	    case @currentStack == undef then skip else
	       Gui,resetReservedTags({@currentStack getSize($)})
	    end

	    currentThread <- T
	    currentStack  <- Stack

	    case {Stack checkNew($)} then
	       {OzcarMessage 'Let thread #' # I # ' make its first step'}
	       {Thread.resume T}
	    else
	       case PrintStack then
		  case S == terminated then
		     SourceManager,removeBar
		     Gui,printStack(id:I frames:nil depth:0)
		  else
		     F L C Exc = {Stack getException($)}
		  in
		     {Stack print}
		     {Stack getPos(file:?F line:?L column:?C)}
		     case Exc == nil then
			SourceManager,bar(file:F line:L column:C state:S)
		     else
			SourceManager,bar(file:F line:L column:C state:blocked)
			Gui,doStatus(Exc clear BlockedThreadColor)
		     end
		  end
	       else skip end
	    end
	 end
      end

      meth toggleEmacsThreads(TkV)
	 Value = case {TkV tkReturnInt($)} == 0 then false else true end
      in
	 case {NewCompiler} then
	    case Value then
	       {Compile '\\switch +runwithdebugger'}
	    else
	       {Compile '\\switch -runwithdebugger'}
	    end
	 else
	    {Dbg.emacsThreads Value}
	 end
      end

      meth toggleSubThreads(TkV)
	 Value = {TkV tkReturnInt($)} \= 0
      in
	 {Dbg.subThreads Value}
      end

   end
end
