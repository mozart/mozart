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
   
   Manager =
   {New
    class
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
	  in
	     case {Thread.is T} then
		{Ozcar stepThread(file:File line:Line thr:T)}
	     else
		{Message "Invalid Thread ID in step message of stream"}
	     end
	     
	  elseof thr then
	     T = M.thr.1
	     I = M.thr.2
	     E = {self exists(T $)}
	  in
	     case E then
		{Message "Got known thread (id " # I # ")"}
	     else
		{Message "Got new thread (id " # I # ")"}
		{self add(T I)}
	     end
	     
	  elseof term then
	     T = M.thr.1
	     E = {self exists(T $)}
	  in
	     case E then
		{self remove(T)}
		{Ozcar removeThread(T M.par.1 M.par.2)}
	     else
		skip
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
    end init}

end
