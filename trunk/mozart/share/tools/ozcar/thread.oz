%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

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
      
      meth init
	 self.Stream    = {Dbg.stream}
	 self.ThreadDic = {Dictionary.new}
	 thread
	    self.ReadLoopThread = {Thread.this}
	    {ReadLoop self.Stream}
	 end
      end

      meth getCurrentThread($)
	 @currentThread
      end
      
      meth getThreadDic($)
	 self.ThreadDic
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
	       Ok = ({Cget stepRecordBuiltin} orelse Name \= 'record')
	            andthen
	            ({Cget stepDotBuiltin}    orelse Name \= '.')
	            andthen
	            ({Cget stepWidthBuiltin}  orelse Name \= 'Width')
	            andthen
		    ({Cget stepSystemProcedures} orelse
		     Name == ''       orelse
		     Name == '`,`'    orelse
		     Name == '`send`' orelse
		     {Atom.toString Name}.1 \= 96)
	    in
	       case Ok then
		  ThreadManager,step(file:File line:Line thr:T id:I
				     name:Name args:Args frame:FrameId
				     builtin:IsBuiltin time:Time)
	       else
		  {OzcarMessage 'Skipping procedure \'' # Name # '\''}
		  {Thread.resume T}
	       end
	    else
	       {OzcarMessage InvalidThreadID}
	    end

	 [] exit then
	    I       = M.thr.2
	    FrameId = M.frame
	    Stack   = {Dget self.ThreadDic I}
	    F L
	 in
	    {ForAll [exit(FrameId) printTop getPos(file:F line:L)] Stack}
	    SourceManager,scrollbar(file:'' line:0 color:undef what:stack)
	    SourceManager,scrollbar(file:F line:L
				    color:ScrollbarApplColor what:appl)

	 [] thr then
	    T = M.thr.1
	    I = M.thr.2
	    Q = case {Value.hasFeature M par} then
		   M.par.2  %% id of parent thread
		else
		   0        %% parent unknown (threads of tk actions...)
		end
	    E = {Ozcar exists(I $)}
	 in
	    case E then
	       Stack = {Dget self.ThreadDic I}
	    in
	       {OzcarMessage KnownThread # {ID I}}
	       {Stack rebuild(true)}
	    else
	       {OzcarMessage NewThread   # {ID I}}
	       case Q == 1 then      %% toplevel query?
		  {Thread.resume T}  %% yes, so we want T to make
	                             %% the first step automatically
		  {Delay 200}        %% short living threads which produce
	                             %% no step messages are uninteresting...
	       else skip end
	       case
		  {Thread.state T} == terminated then
		  {OzcarMessage EarlyThreadDeath}
	       else
		  ThreadManager,add(T I Q)
	       end
	    end
	    
	 [] term then
	    T = M.thr.1  %% just terminated thread
	    I = M.thr.2  %% ...with it's id
	    E = {Ozcar exists(I $)}
	 in
	    case E then
	       ThreadManager,remove(T I noKill)
	    else
	       %{OzcarMessage UnknownTermThread}
	       skip
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
	    E = {Ozcar exists(I $)}
	 in
	    case E then
	       ThreadManager,block(thr:T id:I file:F line:L name:N args:A
				   builtin:B time:Time)
	    else
	       {OzcarMessage UnknownSuspThread}
	    end
	    
	 [] cont then
	    T = M.thr.1  %% woken thread
	    I = M.thr.2  %% ...with it's id
	    E = {Ozcar exists(I $)}
	 in
	    case E then
	       case T == @currentThread then
		  Gui,status(I runnable)
	       else skip end
	       Gui,markNode(I runnable)
	    else
	       {OzcarMessage UnknownWokenThread}
	    end
	    
	 else
	    {OzcarMessage UnknownMessage}
	 end
      end

      %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

      meth exists(I $)
	 {Dmember self.ThreadDic I}
      end
      
      meth add(T I Q)
	 {Dput self.ThreadDic I {New StackManager init(thr:T id:I)}}
	 Gui,addNode(I Q)
	 case Q == 0 orelse Q == 1 then 
	    ThreadManager,switch(I)       %% does Gui,displayTree
	 else
	    Gui,displayTree
	 end
      end

      meth remove(T I Mode)
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
	       Gui,status(0)
	    else
	       Gui,status(I terminated)
	    end
	    Gui,printStack(id:0 frames:nil depth:0)
	 else skip end
	 case {Dkeys self.ThreadDic} == nil then
	    {OzcarMessage 'no more threads to debug.'}
	    currentThread <- undef
	    Gui,status(0)
	    Gui,selectNode(0)
	    Gui,displayTree
	 else skip end
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
	 case T == @currentThread then Ack in
	    case {UnknownFile F} then
	       {OzcarMessage NoFileInfo # I}
	       SourceManager,scrollbar(file:'' line:0 color:undef what:both)
	       {Thread.resume @currentThread}
	    else
	       SourceManager,scrollbar(file:'' line:0 color:undef what:stack)
	       thread
		  SourceManager,scrollbar(file:F line:L ack:Ack
					  color:ScrollbarApplColor what:appl)
	       end
	       thread Gui,loadStatus(F Ack) end
	       thread {Stack printTop} end
	    end
	 else skip end
      end

      meth block(thr:T id:I file:F line:L name:N args:A builtin:B time:Time)
	 Stack = {Dget self.ThreadDic I}
      in
	 Gui,markNode(I blocked)
	 case T == @currentThread then
	    Gui,status(I blocked)
	    case {UnknownFile F} then
	       {OzcarMessage 'Thread #' # I # NoFileBlockInfo}
	       SourceManager,scrollbar(file:'' line:0 color:undef what:both)
	    else
	       SourceManager,scrollbar(file:F line:L
				       color:ScrollbarBlockedColor what:appl)
	       SourceManager,scrollbar(file:'' line:0 color:undef what:stack)
	    end
	    {ForAll [rebuild(true) printTop] Stack}
	 else skip end
      end
      
      meth switch(I)
	 F L N A B Time
      in
	 case I == 1 then
	    Gui,status(0)
	 else
	    Stack = {Dget self.ThreadDic I}
	    T     = {Stack getThread($)}
	    S     = {Thread.state T}
	 in
	    currentThread <- T
	    {Stack getPos(file:F line:L)}
	    
	    Gui,status(I S)
	    Gui,selectNode(I)
	    Gui,displayTree
	    Gui,printEnv(frame:0)  % clear environment windows
	    
	    case S == terminated then
	       SourceManager,scrollbar(file:'' line:0 color:undef what:appl)
	       Gui,printStack(id:0 frames:nil depth:0)
	    else Ack in
	       thread
		  SourceManager,
		  scrollbar(file:F line:L ack:Ack
			    color:
			       case S
			       of runnable then ScrollbarApplColor
			       [] blocked  then ScrollbarBlockedColor
			       end
			    what:appl)
	       end
	       thread Gui,loadStatus(F Ack) end
	       thread {Stack print} end
	    end
	    SourceManager,scrollbar(file:'' line:0 color:undef what:stack)
	 end
      end

      meth close
	 %% actually, we should kill this damned thread, but then we get this:
	 %% board.icc:21
	 %% Internal Error:  assertion '!isCommitted() && !isFailed()' failed
	 %% (DFKI Oz Emulator 1.9.16 (sunos-sparc) of Mon Oct, 07, 1996) 
	 {Thread.suspend self.ReadLoopThread}
      end
      
   end
end
