%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   proc {ReadLoop S}
      case S
      of H|T then
	 {Show 'readloop:'}{Show H}
	 {Ozcar message(H)}
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
      
      meth message(M)
	 case {Label M}
	 of step then
	    T         = M.thr.1
	    File      = M.file
	    Line      = M.line
	    IsBuiltin = M.builtin
	    Name = case {Value.hasFeature M name} then M.name else nil end
	    Args = case {Value.hasFeature M args} then M.args else nil end
	 in
	    case {Thread.is T} then
	       ThreadManager,step(file:File line:Line thr:T
				  name:Name args:Args
				  builtin:IsBuiltin)
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
		  ThreadManager,add(T I)
	       end
	    end
	    
	 elseof term then
	    T = M.thr.1
	    P = M.par.1
	    I = M.par.2  %% id of parent thread
	    E = {Ozcar exists(T $)}
	 in
	    case E then
	       ThreadManager,remove(T P I)
	    else
	       skip
	    end
	    
	 else skip end
      end

      meth add(T I)
	 Threads <- T | @Threads
	 Gui,addNode(T I)
	 ThreadManager,switch(T I)
	 Gui,displayTree
      end
      
      meth remove(T P I)  %% I = <id of parent thread>
	 Threads <- {List.filter @Threads fun {$ X} X\=T end}
	 case @currentThread == T then
	    ThreadManager,switch(P I)
	 else skip end
	 Gui,removeNode(T)
	 Gui,displayTree
      end
      
      meth exists(T $)
	 {List.member T @Threads}
      end

      meth step(file:F line:L thr:T name:N args:A builtin:IsBuiltin)
	 SourceManager,scrollbar(file:F line:L color:ScrollbarDefaultColor)
	 Gui,stackTitle('Stack of  #' # {Thread.id T})
	 local
	    W    = Gui,stackText($)
	    Args = {FormatArgs A}
	    File = {Str.rchr {Atom.toString F} &/}.2
	 in
	    {ForAll [tk(conf state:normal)
		     tk(insert 'end' '{' # N)
		     tk(conf state:disabled)] W}
	     
	     {ForAll Args
	      proc {$ A}
		 T = {TagCounter get($)}
		 Ac = {New Tk.action
		       tkInit(parent:{W w($)}
			      action:proc{$}
					S = A.3
				     in
					{Browse S}
				     end)}
	      in
		 {ForAll [tk(conf state:normal)
			  tk(insert 'end' ' ')
			  tk(insert 'end' A.2 T)
			  tk(conf state:disabled)
			  tk(tag bind T '<1>' Ac)
			  tk(tag conf T font:BoldFont)] W}
	      end}
	     
	     {ForAll [tk(conf state:normal)
		      tk(insert 'end' '}\n') 
	      tk(conf state:disabled)
	      tk(yview 'end')] W}
	 end
      end
      
      meth switch(T I)
         %TS = {Dbg.taskstack T}
	 %% todo: adapt highlighted lines in SourceWindows
      %in
	 currentThread <- T
	 Gui,status("Current Thread:  #" # I #
		    "  (" # {Thread.state T} # ")")
	 Gui,selectNode(T)
	 Gui,displayTree
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
