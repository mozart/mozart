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
      ({Cget stepWaitForArbiterBuiltin} orelse Name \= 'waitForArbiter')
      andthen
      ({Cget stepSystemProcedures} orelse
       Name == ''         orelse
       Name == '`,`'      orelse
       Name == '`send`'   orelse
       Name == '`ooSend`' orelse
       {Atom.toString Name}.1 \= 96)
   end
   
   proc {ReadLoop S}
      case S
      of H|T then
	 {OzcarMessage 'readloop:'} {OzcarShow H}
	 {Ozcar readStreamMessage(H)}
	 {OzcarMessage 'preparing for next stream message...'}
	 {ReadLoop T}
      end
   end

in
   
   class ThreadManager
      feat
	 Stream                %% info stream of the emulator
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
	 self.Stream    = {Dbg.stream}
	 self.ThreadDic = {Dictionary.new}
	 thread
	    self.ReadLoopThread = {Thread.this}
	    {ReadLoop self.Stream}
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
	 SkippedProcs <- FrameId # I | @SkippedProcs
	 {OzcarMessage 'Skipping procedure \'' # Name # '\''}
	 {OzcarShow @SkippedProcs}
	 {Thread.resume T}
      end
      
      meth readStreamMessage(M)
	 
	 case M
	    
	 of step(thr:T#I file:File line:Line builtin:IsBuiltin
		 time:Time frame:FrameId ...) then
	    Name = {CondSelect M name nil}
	    Args = {CondSelect M args nil}
	    Ok   = {AppOK Name}
	 in
	    case ThreadManager,Exists(I $) then
	       case Ok then
		  ThreadManager,step(file:File line:Line thr:T id:I
				     name:Name args:Args frame:FrameId
				     builtin:IsBuiltin time:Time)
	       else
		  ThreadManager,AddToSkippedProcs(Name T I FrameId)
	       end

	    else
	       Obj   = try {Nth Args 3}
		       catch failure(...) then nil end
	       ForMe = {IsDet Obj} andthen Obj == self
	    in
	      case ForMe then
		 {Dbg.trace T false}
		 {Dbg.stepmode T false}
		 {Thread.resume T}
		 {OzcarMessage 'message for Ozcar detected.'}

	      elsecase (File == '' orelse File == 'nofile') then
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
		    %% case Ok then
		    ThreadManager,step(file:File line:Line thr:T id:I
				       name:Name args:Args frame:FrameId
				       builtin:IsBuiltin time:Time)
		 end
	      end
	    end

	 [] exit(thr:T#I frame:Frame) then
	    Found = {Member Frame.1 # I @SkippedProcs}
	 in
	    {OzcarShow @SkippedProcs # (Frame.1 # I) # Found}
	    case Found orelse @SkippedThread == T then
	       {OzcarMessage 'ignoring exit message'}
	       SkippedProcs  <- {Filter @SkippedProcs
				 fun {$ F} F \= Frame.1 # I end}
	       SkippedThread <- nil
	       {Thread.resume T}
	    else
	       Gui,markNode(I runnable) % thread is not running anymore
	       Gui,markStack(active)    % stack view has up-to-date content
	       case T == @currentThread then
		  F L
		  Stack = {Dget self.ThreadDic I}
	       in
		  {ForAll [exit(Frame) getPos(file:F line:L)] Stack}
		  SourceManager,bar(file:F line:L state:runnable)
		  {Stack printTop}
	       else skip end
	    end
	    
	 [] thr(thr:T#I ...) then
	    Q = case {Value.hasFeature M par} then
		   M.par.2  %% id of parent thread
		else
		   0        %% parent unknown (threads of tk actions...)
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
	       case Q == 0 orelse Q == 1 then
		  thread
		     case Q == 1 then
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
			case Q == 0 then
			   ThreadManager,add(T I Q true)
			else
			   ThreadManager,add(T I Q false)
			end
		     end
		  end
	       else
		  ThreadManager,add(T I Q false)
	       end
	    end
	    
	 [] term(thr:T#I) then
	    E = ThreadManager,Exists(I $)
	 in
	    case E then
	       ThreadManager,remove(T I noKill)
	    else
	       {OzcarMessage EarlyTermThread}
	    end
	    
	 [] block(thr:T#I file:F line:L name:N args:A builtin:B time:Time) then
	    case ThreadManager,Exists(I $) then
	       ThreadManager,block(thr:T id:I file:F line:L)
	    else
	       thread
		  {OzcarMessage WaitForThread}
		  {Delay TimeoutToBlock} % thread should soon be added
		  case ThreadManager,Exists(I $) then
		     ThreadManager,block(thr:T id:I file:F line:L)
		  else
		     {OzcarError UnknownSuspThread}
		  end
	       end
	    end
	    
	 [] cont(thr:T#I) then
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
	    
	 [] exception(thr:T#I exc:X) then
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
		     ThreadManager,add(T I X#_ false)
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
	 
	 case {IsTuple Q} then  %% exception
	    Gui,addNode(I 0)
	    {Stack checkNew(_)} %% _don't_ do a first step here!
	    ThreadManager,switch(I false)
	    {Stack printException(Q.1)}
	 else
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

      meth step(file:F line:L thr:T id:I name:N args:A
		builtin:B time:Time frame:FrameId)
	 Stack = {Dget self.ThreadDic I}
      in
	 Gui,markNode(I runnable) % thread is not running anymore
	 Gui,markStack(active)    % stack view has up-to-date content
	 {Stack step(name:N args:A builtin:B file:F line:L
		     time:Time frame:FrameId)}
	 case T == @currentThread then
	    case {UnknownFile F} then
	       {OzcarMessage NoFileInfo # I}
	       SourceManager,removeBar
	       {Thread.resume T}
	    else
	       SourceManager,bar(file:F line:L state:runnable)
	       {Stack printTop}
	    end
	 else skip end
      end
      
      meth block(thr:T id:I file:F line:L)
	 Gui,markNode(I blocked)
	 case T == @currentThread then
	    case {UnknownFile F} then
	       {OzcarMessage 'Thread #' # I # NoFileBlockInfo}
	       SourceManager,removeBar
	    else
	       SourceManager,bar(file:F line:L state:running)
	    end
	    %Gui,doStatus('Thread #' # I # ' is blocked')
	 else skip end
      end
      
      meth rebuildCurrentStack
	 Stack = @currentStack
      in
	 case Stack == undef then
	    Gui,doStatus(FirstSelectThread)
	 else
	    Gui,doStatus(RebuildMessage # {Thread.id @currentThread} # '...')
	    {ForAll [rebuild(true) print] Stack}
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
	    S     = local X = {Thread.state T} in
		       case     {Dbg.checkStopped T} then X
		       elsecase X == terminated      then X
		       else                               running end
		    end
	 in
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
		     F L Exc = {Stack getException($)}
		  in
		     {ForAll [print getPos(file:F line:L)] Stack}
		     case Exc == nil then
			case S == running then
			   Gui,markStack(inactive)
			else skip end
			SourceManager,bar(file:F line:L state:S)
		     else
			SourceManager,bar(file:F line:L state:blocked)
			Gui,doStatus(Exc clear BlockedThreadColor)
		     end
		  end
	       else skip end
	    end
	 end
      end

      meth suspend(TkV)
	 Value = {TkV tkReturnInt($)}
	 Arg   = case Value == 0 then true else false end
      in
	 {OzcarMessage 'Dbg.suspend called with argument ' # Value}
	 {Dbg.suspend Arg}
      end

      meth runChildren(TkV)
	 Value = {TkV tkReturnInt($)}
	 Arg   = case Value == 0 then true else false end
      in
	 {OzcarMessage 'Dbg.runChildren called with argument ' # Value}
	 {Dbg.runChildren Arg}
      end
      
      meth close
	 skip
      end
      
   end
end
