%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   proc {ReadLoop S O}
      case S
      of H|T then
	 {Show 'readloop:'}
	 {DebugMessage H}
	 {O read(H)}
	 {ReadLoop T O}
      end
   end
   
in
   
   create Manager from UrObject with init
      feat
	 Stream            %% info stream of the emulator
	 ReadLoopThread    %% we need to kill it when closing the manager
      
      attr
	 Threads : nil     %% list of debugged threads
      
      meth init
	 self.Stream = {Dbg.stream}
	 thread
	    self.ReadLoopThread = {Thread.this}
	    {ReadLoop self.Stream self}
	 end
      end

      meth list($)
	 @Threads
      end
      
      meth add(T)
	 Threads <- T | @Threads
	 {Thread.setName T {New ThreadDebugger init(thr:T)}}
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
	 in
	    case {Thread.is T} then
	       {{Thread.getName T} displayLine(file:File line:Line)}
	    else
	       {Message "Invalid Thread ID in step message of stream"}
	    end

	 elseof thr then
	    T = M.thr.1
	    E = self,exists(T $)
	 in
	    case E then
	       {Message "Got known thread (id " # {Thread.id T} # ")"}
	    else
	       {Message "Got new thread (id " # {Thread.id T} # ")"}
	       self,add(T)
	       {Ozcar newThread(T)}
	    end
	    
	 else skip end
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
