%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   proc {ReadLoop S}
      case S
      of H|T then
	 {Show 'readloop:'}{Show H}
	 {Ozcar read(H)}
	 {ReadLoop T}
      end
   end
   
in
   
   class ThreadManager
      feat
	 Stream            %% info stream of the emulator
	 ReadLoopThread    %% we need to kill it when closing the manager
      
      attr
	 Threads : nil     %% list of debugged threads
	 currentThread
      
      meth init
	 self.Stream = {Dbg.stream}
	 thread
	    self.ReadLoopThread = {Thread.this}
	    {ReadLoop self.Stream}
	 end
      end
      
      meth list($)
	 @Threads
      end
       
      meth add(T I)
	 Threads <- T | @Threads
	 {Ozcar newThread(T I)}
      end
      
      meth remove(T)
	 Threads <- {List.filter @Threads fun {$ X} X\=T end}
      end
      
      meth exists(T $)
	 {List.member T @Threads}
      end
      
      meth read(M)
	 case {Label M}
	 of step then
	    T    = M.thr.1
	    File = M.file
	    Line = M.line
	    Name = case {Value.hasFeature M name} then M.name else nil end
	    Args = case {Value.hasFeature M args} then M.args else nil end
	 in
	    case {Thread.is T} then
	       {Ozcar stepThread(file:File line:Line thr:T
				 name:Name args:Args)}
	    else
	       {Message "Invalid Thread ID in step message of stream"}
	    end
	    
	 elseof thr then
	    T = M.thr.1
	    I = M.thr.2
	    E = {Ozcar exists(T $)}
	 in
	    case E then
	       {Message "Got known thread (id " # I # ")"}
	    else
	       {Message "Got new thread (id " # I # ")"}
	       case
		  {Thread.state T} == terminated then
		  {Message "...hm, but it has died already?!"}
	       else
		  {Ozcar add(T I)}
	       end
	    end
	    
	 elseof term then
	    T = M.thr.1
	    E = {Ozcar exists(T $)}
	 in
	    case E then
	       {Ozcar remove(T)}
	       {Ozcar removeThread(T M.par.1 M.par.2)}
	    else
	       skip
	    end
	    
	 else skip end
      end

      meth step(T)
	 {Thread.resume T}
      end
      
      meth cont(T)
	 {Dbg.stepmode T off}
	 {Thread.resume T}
      end
      
      meth stack(T)
	 {Ozcar stackThread(T)}
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
