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
	 {ReadLoop T}
      end
   end

in
   
   class ThreadManager
      feat
	 Stream                %% info stream of the emulator
	 ThreadDic             %% dictionary that holds various information
                               %% about debugged threads
	 ReadLoopThread        %% we need to kill it when closing the manager

      attr
	 currentThread : undef
	 currentStack  : undef
	 SkippedProcs  : nil
	 SkippedThread : nil
	 Breakpoint    : false
      
      meth init
	 self.Stream    = {Dbg.stream}
	 self.ThreadDic = {Dictionary.new}
	 thread
	    self.ReadLoopThread = {Thread.this}
	    {ReadLoop self.Stream}
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
	 case {Label M}
	    
	 of step then
	    T          = M.thr.1
	    I          = M.thr.2
	    File       = M.file
	    Line       = M.line
	    IsBuiltin  = M.builtin
	    Time       = M.time
	    FrameId    = M.frame
	    Name = case {Value.hasFeature M name} then M.name else nil end
	    Args = case {Value.hasFeature M args} then M.args else nil end
	 in
	    case {Thread.is T} then
	       Ok = {AppOK Name}
	    in
	       case ThreadManager,exists(I $) then
		  case Ok then
		     ThreadManager,step(file:File line:Line thr:T id:I
					name:Name args:Args frame:FrameId
					builtin:IsBuiltin time:Time)
		  else
		     ThreadManager,AddToSkippedProcs(Name T I FrameId)
		  end
	       elsecase (File == '' orelse File == 'nofile') then
		  {Dbg.trace T false}
		  {Dbg.stepmode T false}
		  {Thread.resume T}
		  case {HasFeature Args 1} andthen {IsRecord Args.1} andthen
		     (Args.1 == off orelse {Label Args.1} == bpAt) then
		     {OzcarMessage 'message from Emacs detected.'}
		  else
		     Gui,rawStatus(IgnoreNoFileStep)
		  end
	       else
		  {OzcarMessage WaitForThread}
		  {Delay 240} % thread should soon be added
		  case @Breakpoint then
		     Breakpoint <- false
		  else
		     % case Ok then
		     ThreadManager,step(file:File line:Line thr:T id:I
					name:Name args:Args frame:FrameId
					builtin:IsBuiltin time:Time)
		  end
	       end
	    else
	       {OzcarMessage InvalidThreadID}
	    end

	 [] exit then
	    T       = M.thr.1
	    I       = M.thr.2
	    Frame   = M.frame
	    Found   = {Member Frame.1 # I @SkippedProcs}
	 in
	    {OzcarShow @SkippedProcs # (Frame.1 # I) # Found}
	    case Found orelse @SkippedThread == T then
	       {OzcarMessage 'ignoring exit message'}
	       SkippedProcs  <- {Filter @SkippedProcs
				 fun {$ F} F \= Frame.1 # I end}
	       SkippedThread <- nil
	       {Thread.resume T}
	    else
	       F L
	       Stack   = {Dget self.ThreadDic I}
	    in
	       {ForAll [exit(Frame) getPos(file:F line:L)] Stack}
	       SourceManager,scrollbar(file:'' line:0 color:undef what:stack)
	       SourceManager,scrollbar(file:F line:L
				       color:ScrollbarApplColor what:appl)
	       {Stack printTop}
	    end
	    
	 [] thr then
	    T = M.thr.1
	    I = M.thr.2
	    Q = case {Value.hasFeature M par} then
		   M.par.2  %% id of parent thread
		else
		   0        %% parent unknown (threads of tk actions...)
		end
	    E = ThreadManager,exists(I $)
	 in
	    case E then
	       Stack = {Dget self.ThreadDic I}
	    in
	       Gui,rawStatus('Thread #' # I # ' has reached a breakpoint')
	       {OzcarMessage KnownThread # {ID I}}
	       {Stack rebuild(true)}
	    else
	       {OzcarMessage NewThread   # {ID I}}
	       case (Q == 0 orelse Q == 1) andthen
		  {self.tkRunChildren tkReturnInt($)} == 0 then
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
	    
	 [] term then
	    T = M.thr.1  %% just terminated thread
	    I = M.thr.2  %% ...with it's id
	    E = ThreadManager,exists(I $)
	 in
	    case E then
	       ThreadManager,remove(T I noKill)
	    else
	       {OzcarMessage EarlyTermThread}
	    end
	    
	 [] block then
	    T    = M.thr.1  %% just blocking thread
	    I    = M.thr.2  %% ...with it's id
	    F    = M.file
	    L    = M.line
	    N    = M.name
	    A    = M.args
	    B    = M.builtin
	    Time = M.time
	    E    = ThreadManager,exists(I $)
	 in
	    case E then
	       StackObj = {Dget self.ThreadDic I}
	       Ack
	    in
	       thread
		  {OzcarMessage
		   'checking if blocking of thread #'#I#' is persistent...'}
		  {StackObj blockMsg(Ack)}
		  case Ack == ok then
		     lock
			{OzcarMessage '...yes!'}
			{Dbg.contflag T false}
			{Dbg.stepmode T true}
			{Thread.suspend T}
			ThreadManager,block(thr:T id:I file:F line:L name:N
					    args:A builtin:B time:Time)
		     end
		  else
		     {OzcarMessage 'blocking of thread #'#I#' was temporary'}
		     {Thread.resume T}
		  end
	       end
	    else
	       {OzcarError UnknownSuspThread}
	    end
	    
	 [] cont then
	    T = M.thr.1  %% woken thread
	    I = M.thr.2  %% ...with it's id
	    E = ThreadManager,exists(I $)
	 in
	    case E then
	       /*
	       case T == @currentThread andthen
		  {self.tkRunChildren tkReturnInt($)} == 0 then
		  skip
		  %Gui,status(I runnable)
	       else skip end
	       */ 
	       StackObj = {Dget self.ThreadDic I}
	       Ack
	    in
	       lock
		  {OzcarMessage 'serving cont message'}
		  {StackObj contMsg(Ack)}
		  case Ack == ok then
		     Gui,markNode(I runnable)
		     case T == @currentThread then
			F L in
			{StackObj getPos(file:F line:L)}
			SourceManager,scrollbar(file:F line:L what:appl
						color:ScrollbarApplColor)
		     else skip end
		  else skip end
	       end
	    else
	       {OzcarError UnknownWokenThread}
	    end

	 [] exception then
	    T = M.thr.1
	    I = M.thr.2
	    X = M.exc
	 in
	    case ThreadManager,exists(I $) then
	       {{Dget self.ThreadDic I} printException(X)}
	    else
	       thread
		  {Delay 320}
		  case ThreadManager,exists(I $) then
		     {Delay 140}
		     {{Dget self.ThreadDic I} printException(X)}
		  else
		     ThreadManager,add(T I X#_ false)
		  end
	       end
	    end
	    
	 else
	    {OzcarMessage UnknownMessage}
	 end
      end

      %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

      meth exists(I $)
	 {Dmember self.ThreadDic I}
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
	 
	 case {IsTuple Q} then %% exception
	    Gui,addNode(I 0)
	    ThreadManager,switch(I)
	    {Stack printException(Q.1)}
	 else
	    Gui,addNode(I Q)
	    case Q == 0 orelse Q == 1 then 
	       ThreadManager,switch(I)       %% does Gui,displayTree
	       case Q == 1 then
		  Gui,rawStatus('Got new query, selecting thread #' # I)
	       else
		  Gui,rawStatus('Breakpoint reached by thread #' # I #
				', which has been added and selected')
	       end
	    else
	       Gui,displayTree
	    end
	 end
      end

      meth remove(T I Mode)
	 {OzcarMessage 'removing thread #' # I # ' with mode ' # Mode}
	 ThreadManager,removeSkippedProcs(I)
	 case Mode == kill then
	    Gui,killNode(I)
	    {Dremove self.ThreadDic I}
	 else
	    Gui,removeNode(I)
	 end
	 case T == @currentThread then
	    SourceManager,scrollbar(file:'' line:undef color:undef what:both)
	    case Mode == kill then
	       currentThread <- undef
	       currentStack  <- undef
	       Gui,selectNode(0)
	    else skip end
	    Gui,printStack(id:I frames:nil depth:0)
	 else skip end
      end

      meth kill(T I)
	 {Dbg.trace T false}
	 {Thread.terminate T}
	 ThreadManager,remove(T I kill)
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
	     ThreadManager,kill(T I)
	  end}
	 DeleteCount
      end
      
      meth forget(T I)
	 {Dbg.trace T false}      %% thread is not traced anymore
	 {Dbg.stepmode T false}   %% no step mode, run as you like!
	 {Thread.resume T}        %% run, run to freedom!! :-)
	 ThreadManager,remove(T I kill)
      end

      meth step(file:F line:L thr:T id:I name:N args:A
		builtin:B time:Time frame:FrameId)
	 Stack = {Dget self.ThreadDic I}
      in
	 {Stack step(name:N args:A builtin:B file:F line:L
		     time:Time frame:FrameId)}
	 case T == @currentThread then
	    case {UnknownFile F} then
	       {OzcarMessage NoFileInfo # I}
	       SourceManager,scrollbar(file:'' line:0 color:undef what:both)
	       {Thread.resume T}
	    else
	       SourceManager,scrollbar(file:'' line:0 color:undef what:stack)
	       SourceManager,scrollbar(file:F line:L
				       color:ScrollbarApplColor what:appl)
	       {Stack printTop}
	    end
	 else skip end
      end
      
      meth block(thr:T id:I file:F line:L name:N args:A builtin:B time:Time)
	 Stack = {Dget self.ThreadDic I}
      in
	 {Stack rebuild(true)}
	 Gui,markNode(I blocked)
	 case T == @currentThread andthen
	    {self.tkRunChildren tkReturnInt($)} == 0 then
	    case {UnknownFile F} then
	       {OzcarMessage 'Thread #' # I # NoFileBlockInfo}
	       SourceManager,scrollbar(file:'' line:0 color:undef what:both)
	    else
	       SourceManager,scrollbar(file:'' line:0 color:undef what:stack)
	       SourceManager,scrollbar(file:F line:L
				       color:ScrollbarBlockedColor
				       what:appl)
	    end
	    {Stack printTop} 
	 else skip end
      end

      meth rebuildCurrentStack
	 Stack = @currentStack
      in
	 case Stack == undef then
	    Gui,rawStatus(FirstSelectThread)
	 else
	    {ForAll [rebuild(true) print] Stack}
	 end
      end
      
      meth switch(I)
	 F L N A B Time
      in
	 case I == 1 then skip else
	    Stack = {Dget self.ThreadDic I}
	    T     = {Stack getThread($)}
	    S     = {Thread.state T}
	 in
	    currentThread <- T
	    currentStack  <- Stack
	    
	    Gui,selectNode(I)
	    Gui,displayTree
	    
	    case S == terminated then
	       SourceManager,scrollbar(file:'' line:0 color:undef what:appl)
	       Gui,printStack(id:I frames:nil depth:0)
	    else
	       {ForAll [print getPos(file:F line:L)] Stack}
	       SourceManager,
	       scrollbar(file:F line:L
			 color:
			    case S
			    of runnable then ScrollbarApplColor
			    [] blocked  then ScrollbarBlockedColor
			    end
			 what:appl)
	    end
	    SourceManager,scrollbar(file:'' line:0 color:undef what:stack)
	 end
      end

      meth suspend(TkV)
	 Value = {TkV tkReturnInt($)}
	 Arg   = case Value == 0 then false else true end
      in
	 {OzcarMessage 'Dbg.suspend called with argument ' # Value}
	 {Dbg.suspend Arg}
      end

      meth runChildren(TkV)
	 Value = {TkV tkReturnInt($)}
	 Arg   = case Value == 0 then false else true end
      in
	 {OzcarMessage 'Dbg.runChildren called with argument ' # Value}
	 {Dbg.runChildren Arg}
      end
      
      meth close
	 skip
      end
      
   end
end
