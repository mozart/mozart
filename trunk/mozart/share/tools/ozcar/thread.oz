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

   class Thr
      feat
	 Th
	 Stack
      
      attr
	 Loc  : loc( file:undef line:undef)
	 Call : call(builtin:false name:undef args:undef)
	 Time : 0
      
      meth init(T I)
	 self.Th    = T
	 self.Stack = {New StackManager init(thr:T id:I
					     output:{Ozcar getStackText($)})}
      end
      
      meth setPos(name:N args:A<=nil file:F<=undef line:L<=0
		  builtin:B<=false time:T<=0)
	 Loc  <- loc( file:F line:L)
	 Call <- call(builtin:B name:N args:A)
	 Time <- T
      end
      meth getPos(file:?F line:?L name:?N args:?A builtin:?B time:?T)
	 F = @Loc.file
	 L = @Loc.line
	 N = @Call.name
	 A = @Call.args
	 B = @Call.builtin
	 T = @Time
      end

      meth isBuiltin($)
	 @Call.builtin
      end
      
      meth getThr($)
	 self.Th
      end
      
      meth getStack($) self.Stack end
   end

in
   
   class ThreadManager
      feat
	 Stream                %% info stream of the emulator
	 ThreadDic             %% dictionary that holds various information
                               %% about debugged threads
	 ReadLoopThread        %% we need to kill it when closing the manager

      attr
	 Threads : nil         %% list of debugged threads
	 currentThread : undef
      
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
      
      meth readStreamMessage(M)
	 case {Label M}
	    
	 of step then
	    T          = M.thr.1
	    I          = M.thr.2
	    File       = M.file
	    Line       = M.line
	    IsBuiltin  = M.builtin
	    Time       = M.time
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
				     name:Name args:Args
				     builtin:IsBuiltin time:Time)
	       else
		  {OzcarMessage 'Skipping procedure \'' # Name # '\''}
		  {Thread.resume T}
	       end
	    else
	       {OzcarMessage InvalidThreadID}
	    end
	    
	 [] thr then
	    T = M.thr.1
	    I = M.thr.2
	    Q = case {Value.hasFeature M par} then
		   M.par.2  %% id of parent thread
		else
		   0        %% parent unknown (threads of tk actions...)
		end
	    E = {Ozcar exists(T $)}
	 in
	    case E then
	       {OzcarMessage KnownThread # {ID I}}
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
	    E = {Ozcar exists(T $)}
	 in
	    case E then
	       ThreadManager,remove(T I noKill)
	    else
	       %{OzcarMessage UnknownTermThread}
	       skip
	    end
	    
	 [] susp then
	    T    = M.thr.1  %% just suspending thread
	    I    = M.thr.2  %% ...with it's id
	    F    = M.file
	    L    = M.line
	    N    = M.name
	    A    = M.args
	    B    = M.builtin
	    Time = M.time
	    E = {Ozcar exists(T $)}
	 in
	    case E then
	       ThreadManager,block(T I F L N A B Time)
	    else
	       {OzcarMessage UnknownSuspThread}
	    end
	    
	 [] cont then
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
	       {OzcarMessage UnknownWokenThread}
	    end
	    
	 else
	    {OzcarMessage UnknownMessage}
	 end
      end

      %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

      meth exists(T $)
	 {List.member T @Threads}
      end

      meth add(T I Q)
	 Threads <- T | @Threads
	 {Dictionary.put self.ThreadDic I {New Thr init(T I)}}
	 Gui,addNode(I Q)
	 case Q == 0 orelse Q == 1 then 
	    ThreadManager,switch(I)       %% does Gui,displayTree
	 else
	    Gui,displayTree
	 end
      end

      meth remove(T I Mode)
	 O = {Dget self.ThreadDic I}
      in
	 Threads <- {List.filter @Threads fun {$ X} X\=T end}
	 {O setPos(name:undef)}
	 SourceManager,scrollbar(file:'' line:undef color:undef what:both)
	 Gui,printAppl(id:I name:undef args:undef)
	 {{O getStack($)} clear}
	 case Mode == kill then
	    currentThread <- undef
	    Gui,killNode(I)
	    Gui,status(0)
	 else
	    Gui,removeNode(I)
	    Gui,status(I terminated)
	 end
	 case @Threads == nil then
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
		builtin:IsBuiltin time:Time)
	 O = {Dget self.ThreadDic I}
      in
	 {O setPos(file:F line:L time:Time name:N args:A builtin:IsBuiltin)}
	 case T == @currentThread then
	    Ack
	 in
	    %{{O getStack($)} update}
	    {{O getStack($)} print}
	    case F == '' orelse F == 'nofile' orelse F == 'noDebugInfo' then
	       {OzcarMessage NoFileInfo # I}
	       SourceManager,scrollbar(file:'' line:undef color:undef
				       what:both)
	       {Thread.resume @currentThread}
	    else
	       SourceManager,scrollbar(file:'' line:undef color:undef
				       what:stack)
	       thread
		  SourceManager,scrollbar(file:F line:L ack:Ack
					  color:ScrollbarApplColor what:appl)
	       end
	       thread Gui,loadStatus(F Ack) end
	    end
	    Gui,printAppl(id:I name:N args:A builtin:IsBuiltin time:Time
			  file:F line:L)
	 else skip end
      end

      meth block(T I F L N A B Time)
	 O = {Dget self.ThreadDic I}
      in
	 {O setPos(file:F line:L time:Time name:N args:A builtin:B)}
	 Gui,markNode(I blocked)
	 case T == @currentThread then
	    {{O getStack($)} print}
	    case F == '' orelse F == 'nofile' orelse F == 'noDebugInfo' then
	       {OzcarMessage 'Thread #' # I # NoFileBlockInfo}
	       SourceManager,scrollbar(file:'' line:undef
				       color:undef what:both)
	    else
	       SourceManager,scrollbar(file:F line:L
				       color:ScrollbarBlockedColor what:appl)
	       SourceManager,scrollbar(file:'' line:undef
				       color:undef what:stack)
	    end
	    Gui,printAppl(id:I name:N args:A builtin:B
			  file:F line:L time:Time)
	    Gui,status(I blocked)
	 else skip end
      end
      
      meth switch(I)
	 F L N A B Time
      in
	 case I == 1 then
	    Gui,status(0)
	 else
	    O = {Dget self.ThreadDic I}
	    T = {O getThr($)}
	    S = {Thread.state T}
	 in
	    currentThread <- T
	    {O getPos(file:F line:L name:N args:A builtin:B time:Time)}

	    {{O getStack($)} print}
	    
	    Gui,status(I S)
	    Gui,selectNode(I)
	    Gui,displayTree

	    case S \= terminated then
	       SourceManager,
	       scrollbar(file:F line:L
			 color:
			    case S
			    of runnable then ScrollbarApplColor
			    [] blocked  then ScrollbarBlockedColor
			    end
			 what:appl)
	    else
	       SourceManager,scrollbar(file:undef line:undef
				       color:undef what:appl)
	    end
	    SourceManager,scrollbar(file:undef line:undef
				    color:undef what:stack)
	    Gui,printAppl(id:I name:N args:A builtin:B file:F line:L time:Time)
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
