%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   proc {ReadLoop S}
      case S
      of H|T then
	 {OzcarMessage 'readloop:'}{Show H}
	 {Ozcar readStreamMessage(H)}
	 {ReadLoop T}
      end
   end

   class Thr
      attr
	 Th   : undef
      
	 Loc  : loc( file:undef line:undef)
	 Call : call(name:undef args:undef)
	 Stack: nil

      meth init(T)
	 Th <- T
      end
      
      meth setPos(file:F line:L name:N args:A)
	 Loc  <- loc( file:F line:L)
	 Call <- call(name:N args:A)
      end
      meth getPos(file:?F line:?L name:?N args:?A)
	 F = @Loc.file
	 L = @Loc.line
	 N = @Call.name
	 A = @Call.args
      end

      meth getThr($)
	 @Th
      end
      
      meth setStack(S) Stack <- S end
      meth getStack($) @Stack end
   end

in
   
   class ThreadManager
      feat
	 Stream            %% info stream of the emulator
	 ThreadDic         %% dictionary that holds various information
                           %% about debugged threads
	 ReadLoopThread    %% we need to kill it when closing the manager

      attr
	 Threads : nil     %% list of debugged threads
	 currentThread
      
      meth init
	 self.Stream    = {Dbg.stream}
	 self.ThreadDic = {Dictionary.new}
	 thread
	    self.ReadLoopThread = {Thread.this}
	    {ReadLoop self.Stream}
	 end
      end
      
      meth readStreamMessage(M)
	 case {Label M}
	 of step then
	    T         = M.thr.1
	    I         = M.thr.2
	    File      = M.file
	    Line      = M.line
	    IsBuiltin = M.builtin
	    Name = case {Value.hasFeature M name} then M.name else nil end
	    Args = case {Value.hasFeature M args} then M.args else nil end
	 in
	    case {Thread.is T} then
	       ThreadManager,step(file:File line:Line thr:T id:I
				  name:Name args:Args
				  builtin:IsBuiltin)
	    else
	       {OzcarMessage InvalidThreadID}
	    end
	    
	 elseof thr then
	    T = M.thr.1
	    I = M.thr.2
	    P = M.par.1
	    Q = M.par.2  %% id of parent thread
	    E = {Ozcar exists(T $)}
	 in
	    case E then
	       {OzcarMessage KnownThread # {ID I}}
	    else
	       {OzcarMessage NewThread   # {ID I}}
	       case
		  {Thread.state T} == terminated then
		  {OzcarMessage EarlyThreadDeath}
	       else
		  ThreadManager,add(T I P Q)
	       end
	    end
	    
	 elseof term then
	    T = M.thr.1  %% just terminated thread
	    I = M.thr.2  %% ...with it's id
	    P = M.par.1  %% the sad parent <schnueff>
	    Q = M.par.2  %% ...with it's id
	    E = {Ozcar exists(T $)}
	 in
	    case E then
	       ThreadManager,remove(T I P Q)
	    else
	       {OzcarMessage 'Unknown terminating thread'}
	    end

	 elseof susp then
	    T = M.thr.1  %% just suspending thread
	    I = M.thr.2  %% ...with it's id
	    E = {Ozcar exists(T $)}
	 in
	    case E then
	       case T == @currentThread then
		  Gui,status(I blocked)
	       else skip end
	       Gui,markNode(I blocked)
	    else
	       {OzcarMessage 'Unknown suspending thread'}
	    end
	    
	 elseof cont then
	    T = M.thr.1  %% woken thread
	    I = M.thr.2  %% ...with it's id
	    E = {Ozcar exists(T $)}
	 in
	    case E then
	       case T == @currentThread then
		  Gui,status(I runnable)
	       else skip end
	       Gui,markNode(I runnable)
	    else
	       {OzcarMessage 'Unknown woken thread'}
	    end
	    
	 else
	    {OzcarMessage 'Unknown message on stream'}
	 end
      end

      meth add(T I P Q)
	 Threads <- T | @Threads
	 {Dictionary.put self.ThreadDic I {New Thr init(T)}}
	 Gui,addNode(I Q)
	 ThreadManager,switch(I)
      end
      
      meth remove(T I P Q)  %% Q = <id of parent thread>
	 Threads <- {List.filter @Threads fun {$ X} X\=T end}
	 case @currentThread == T then
	    ThreadManager,switch(Q)
	 else skip end
	 Gui,removeNode(I)
	 Gui,displayTree
      end
      
      meth exists(T $)
	 {List.member T @Threads}
      end

      meth setThrPos(id:I file:F line:L name:N args:A)
	 T = {Dictionary.get self.ThreadDic I}
      in
	 {T setPos(file:F line:L name:N args:A)}
      end
      
      meth getThrPos(id:I file:?F line:?L name:?N args:?A)
	 T = {Dictionary.get self.ThreadDic I}
      in
	 {T getPos(file:F line:L name:N args:A)}
      end

      meth getThrThr(id:I thr:?T state:?S)
	 O = {Dictionary.get self.ThreadDic I}
      in
	 T = {O getThr($)}
	 S = {Thread.state T}
      end
      
      meth step(file:F line:L thr:T id:I name:N args:A builtin:IsBuiltin)
	 SourceManager,scrollbar(file:F line:L color:ScrollbarDefaultColor)
	 ThreadManager,setThrPos(id:I file:F line:L name:N args:A)
	 Gui,printStackFrame(nr:1 name:N args:A)
      end
      
      meth switch(I)
	 F L N A T S
      in
	 case I == 1 then
	    Gui,status(0)
	 else
	    ThreadManager,getThrPos(id:I file:F line:L name:N args:A)
	    ThreadManager,getThrThr(id:I thr:T state:S)
	    currentThread <- T
	    
	    Gui,status(I S)
	    Gui,stackTitle(AltStackTitle # I)
	    Gui,printStackFrame(nr:1 name:N args:A)
	    
	    Gui,selectNode(I)
	    Gui,displayTree
	    
	    SourceManager,scrollbar(file:F line:L color:ScrollbarDefaultColor)
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
