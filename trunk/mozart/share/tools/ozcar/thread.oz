%%%
%%% Author:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.

local

   fun {AppOK Name}
      ({Cget stepDotBuiltin}     orelse Name \= '.')
      andthen
      ({Cget stepNewNameBuiltin} orelse Name \= 'NewName')
   end

   proc {Detach T}
      {Dbg.trace T false}
      {Dbg.step T false}
      {Thread.resume T}
   end

   proc {OzcarReadEvalLoop S}
      case S
      of H|T then
	 {OzcarMessage 'readloop:'} {OzcarShow H}
	 {Ozcar PrivateSend(readStreamMessage(H))}
	 {OzcarMessage 'preparing for next stream message...'}
	 {OzcarReadEvalLoop T}
      end
   end

in

   class ThreadManager
      feat
	 ThreadDic             %% dictionary that holds various information
			       %% about debugged threads

      attr
	 ReadLoopThread : unit

	 currentThread  : unit
	 currentStack   : unit

	 SkippedProcs   : nil

	 SwitchSync     : _

      meth init
	 self.ThreadDic = {Dictionary.new}
	 thread
	    ReadLoopThread <- {Thread.this}
	    {OzcarReadEvalLoop {Dbg.stream}}
	 end
      end

      meth destroy
	 Gui,doStatus('Destroying myself -- byebye...')
	 {Dbg.off}
	 {Thread.terminate @ReadLoopThread}
	 {Compile '\\switch -debuginfo'}
	 {SendEmacs removeBar}
	 {Delay 1000}
	 {self.toplevel tkClose}
      end

      meth checkMe
	 T = @currentThread
      in
	 case T == unit then
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
	    case ThreadManager,Exists(I $) then %% already attached thread
	       Name = {CondSelect M data unit}
	    in
	       case {Not {IsDet Name}}
		  orelse {AppOK {System.printName Name}}
	       then
		  ThreadManager,M
	       else
		  ThreadManager,AddToSkippedProcs(Name T I M.frameID)
	       end

	    else %% this is a (not yet) attached thread
	       Data = {CondSelect M data unit}
	       Q    = {Thread.parentId T}
	       %% The following is not very nice. A better solution
	       %% will be implemented at some time in the near future...
	       Ignore = case {Not {IsDet Data}} then false
			else Data == Ozcar orelse
			   Data == {Compiler.getOPICompiler}
			end
	    in
	       case {Cget subThreads} orelse
		  {Not ThreadManager,Exists(Q $)} then
		  case Ignore orelse {UnknownFile M.file} then %% don't attach!
		     {OzcarMessage 'Ignoring new thread'}
		     {Detach T}
		  else %% yes, do attach!
		     ThreadManager,add(T I Q)
		  end
	       else
		  {OzcarMessage 'Ignoring new subthread'}
		  {Detach T}
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
	       {Thread.resume T}
	    else
	       Gui,markNode(I runnable) % thread is not running anymore
	       Gui,markStack(active)    % stack view has up-to-date content
	       case T == @currentThread then
		  Stack = {Dictionary.get self.ThreadDic I}
	       in
		  {Stack exit(M)}
		  {SendEmacs bar(file:{CondSelect M file nofile}
				    line:{CondSelect M line unit}
				    column:{CondSelect M column unit}
				    state:runnable)}
		  {Stack printTop}
	       else skip end
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
		     {OzcarError 'Unknown suspending thread'}
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
		     {SendEmacs configureBar(runnable)}
		     %Gui,doStatus('Thread #' # I # ' is runnable again')
		  else skip end
	       else
		  Gui,markNode(I running)
	       end
	    else
	       {OzcarError 'Unknown woken thread'}
	    end

	 [] exception(thr:T exc:X) then
	    I = {Thread.id T}
	 in
	    case ThreadManager,Exists(I $) then
	       {{Dictionary.get self.ThreadDic I} printException(X)}
	    else
	       thread
		  {OzcarMessage 'exception of unknown thread -- waiting...'}
		  {Delay 320}
		  case ThreadManager,Exists(I $) then
		     {OzcarMessage 'ok, got it -- printException'}
		     {Delay 140}
		     {{Dictionary.get self.ThreadDic I} printException(X)}
		  else
		     {OzcarMessage 'still not known -- adding...'}
		     ThreadManager,add(T I exc(X))
		  end
	       end
	    end

	 [] update(thr:T) then
	    I = {Thread.id T}
	 in
	    case ThreadManager,Exists(I $) then
	       Stack = {Dictionary.get self.ThreadDic I}
	    in
	       {Stack rebuild(true)}
	    else
	       {OzcarMessage 'ignoring update of unknown thread'}
	    end

	 else
	    {OzcarError 'Unknown message on stream'}
	 end
      end

      %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

      meth Exists(I $)
	 {Dictionary.member self.ThreadDic I}
      end

      meth EmptyTree($)
	 {Dictionary.keys self.ThreadDic} == nil
      end

      meth removeSkippedProcs(I)
	 SkippedProcs <- {Filter @SkippedProcs
			  fun {$ F} F.2 \= I end}
      end

      meth add(T I Q)
	 Stack = {New StackManager init(thr:T id:I)}
      in
	 {Dictionary.put self.ThreadDic I Stack}
	 case Q of exc(X) then     %% exception
	    Gui,addNode(I 0)
	    ThreadManager,switch(I false)
	    {Stack printException(X)}
	 else                      %% Q is the ID of the parent thread
	    {OzcarMessage 'add ' # I # '/' # Q}
	    Gui,addNode(I Q)
	    {Stack rebuild(true)}
	    case Q == 1 then       %% all compiler threads have id #1
	       ThreadManager,switch(I)
	       Gui,status('Got new query, selecting thread #' # I)
	    else skip end
	 end
      end

      meth remove(T I Mode Select<=true)
	 Next in
	 {OzcarMessage 'removing thread #' # I # ' with mode ' # Mode}
	 ThreadManager,removeSkippedProcs(I)
	 case Mode == kill then
	    Gui,killNode(I Next)
	    {Dictionary.remove self.ThreadDic I}
	    case ThreadManager,EmptyTree($) then
	       currentThread <- unit
	       currentStack  <- unit
	       {SendEmacs removeBar}
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
		     currentThread <- unit % to ignore Gui actions temporarily
		     ThreadManager,switch(Next)
		     Gui,status(', new selected thread is #' # Next append)
		  else skip end
	       end
	    else
	       Gui,status('Thread #' # I # ' died')
	       {SendEmacs removeBar}
	       Gui,printStack(id:I frames:nil depth:0)
	    end
	 else skip end
      end

      meth kill(T I Select<=true)
	 lock
	    {Dbg.trace T false}
	    {Dbg.step T false}
	    {Thread.terminate T}
	    case Select then
	       Gui,doStatus('Thread #' # I # ' has been terminated')
	    else skip end
	    ThreadManager,remove(T I kill Select)
	 end
      end

      meth killAll($)
	 E = {Dictionary.items self.ThreadDic}
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

      meth removeAllDead
	 {ForAll {Dictionary.items self.ThreadDic}
	  proc {$ S}
	     I = {S getId($)}
	     T = {S getThread($)}
	  in
	     case {Thread.state T} == terminated then
		ThreadManager,remove(T I kill false)
		Gui,doStatus('.' append)
	     else skip end
	  end}
	 {Delay 1000}   %% give the user a chance to read the dots... ;)
	 Gui,nextThread %% select next living thread
      end

      meth detach(T I)
	 lock
	    {Dbg.trace T false}      %% thread is not traced anymore
	    {Dbg.step T false}       %% no step mode, run as you like!
	    {Thread.resume T}        %% run, run to freedom!! :-)
	    Gui,doStatus('Thread #' # I # ' is not traced anymore')
	    ThreadManager,remove(T I kill)
	 end
      end

      meth entry(thr: T ...)=Frame
	 I     = {Thread.id T}
	 Stack = {Dictionary.get self.ThreadDic I}
      in
	 Gui,markNode(I runnable) % thread is not running anymore
	 Gui,markStack(active)    % stack view has up-to-date content
	 {Stack entry(Frame)}
	 case T == @currentThread then
	    F = {CondSelect Frame file nofile}
	 in
	    case {UnknownFile F} then
	       {OzcarMessage NoFileInfo # I}
	       {SendEmacs removeBar}
	       {Thread.resume T}
	    else
	       L = {CondSelect Frame line unit}
	       C = {CondSelect Frame column unit}
	    in
	       {SendEmacs bar(file:F line:L column:C state:runnable)}
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
	 case Stack == unit then
	    %Gui,doStatus(FirstSelectThread)
	    skip
	 else
%	    Gui,doStatus('Re-calculating stack of thread #' #
%			 {Thread.id @currentThread} # '...')
	    {Stack rebuild(true)}
	    {Stack print}
	    {Stack emacsBarToTop}
%	    Gui,doStatus(' done' append)
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
	    Stack = {Dictionary.get self.ThreadDic I}
	    T     = {Stack getThread($)}
	    S     = {CheckState T}
	 in

	    case @currentStack == unit then skip else
	       Gui,resetReservedTags({@currentStack getSize($)})
	    end

	    currentThread <- T
	    currentStack  <- Stack

	    case PrintStack then
	       case S == terminated then
		  {SendEmacs removeBar}
		  Gui,printStack(id:I frames:nil depth:0)
	       else
		  F L C Exc = {Stack getException($)}
	       in
		  {Stack print}
		  {Stack getPos(file:?F line:?L column:?C)}
		  case Exc == nil then
		     {SendEmacs bar(file:F line:L column:C state:S)}
		  else
		     {SendEmacs bar(file:F line:L column:C state:blocked)}
		     Gui,doStatus(Exc clear BlockedThreadColor)
		  end
	       end
	    else skip end
	 end
      end

      meth toggleEmacsThreads
	 {Ctoggle emacsThreads}
	 case {Cget emacsThreads} then
	    {Compile '\\switch +runwithdebugger'}
	 else
	    {Compile '\\switch -runwithdebugger'}
	 end
      end

      meth toggleSubThreads
	 {Ctoggle subThreads}
      end
   end
end
