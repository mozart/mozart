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
	 {OzcarMessage 'ready for next message'}
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
	 detachDone     : unit
	 switchDone     : unit

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
	 {EnqueueCompilerQuery setSwitch(debuginfo false)}
	 {SendEmacs removeBar}
	 {Delay 1000}
	 {self.toplevel tkClose}
      end

      meth checkMe
	 case ThreadManager,emptyForest($) then
	    Gui,doStatus(NoThreads)
	 else
	    T = @currentThread
	    I = {Thread.id T}
	    R = case {Dbg.checkStopped T} then 'stopped' else 'not stopped' end
	    S = {Thread.state T}
	    N = {Length {Dictionary.items self.ThreadDic}}
	 in
	    Gui,doStatus(N # ' attached threads, currently selected: #' #
			 I # '/' # {Thread.parentId T} #
			 ' (' # R # ', ' # S # ')')
	 end
      end

      meth getThreadDic($)
	 self.ThreadDic
      end

      meth AddToSkippedProcs(Name T I FrameId)
	 Key = FrameId # I
      in
	 SkippedProcs <- Key | @SkippedProcs
	 {OzcarMessage 'skipping procedure \'' # Name # '\''}
	 {OzcarShow @SkippedProcs}
	 {Thread.resume T}
      end

      meth readStreamMessage(M)

	 lock UserActionLock then skip end %% don't process messages too fast..

	 case M

	 of breakpoint(thr:T) then
	    I = {Thread.id T}
	 in
	    case ThreadManager,Exists(I $) then %% already attached thread
	       M = 'Thread #' # I # ' has reached a breakpoint'
	       S = {Dictionary.get self.ThreadDic I}
	    in
	       {OzcarMessage 'breakpoint reached by attached thread #' # I}
	       Gui,status(M)
	       {S rebuild(true)}
	    else
	       {OzcarMessage ('`breakpoint\' message of unattached' #
			      ' thread -- ignoring')}
	    end

	 [] entry(thr:T ...) then
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
	       Q = {Thread.parentId T}
	    in
	       case {Cget subThreads} orelse
		  {Not ThreadManager,Exists(Q $)} then
		  case {UnknownFile M.file} then %% don't attach!
		     {OzcarMessage 'ignoring new thread'}
		     {Detach T}
		  else %% yes, do attach!
		     ThreadManager,add(T I Q)
		  end
	       else
		  {OzcarMessage 'ignoring new subthread'}
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
	       {OzcarMessage 'ignoring `exit\' message of ignored application'}
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
	 in
	    case ThreadManager,Exists(I $) then
	       ThreadManager,remove(T I noKill)
	    else
	       {OzcarMessage
		'`term\' message of unattached thread -- ignoring'}
	    end

	 [] blocked(thr:T) then
	    I = {Thread.id T}
	 in
	    case ThreadManager,Exists(I $) then
	       ThreadManager,blocked(thr:T id:I)
	    else
	       {OzcarMessage
		'`blocked\' message of unattached thread -- ignoring'}
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
	       {OzcarMessage
		'`ready\' message of unattached thread -- ignoring'}
	    end

	 [] exception(thr:T exc:X) then
	    I = {Thread.id T}
	 in
	    case ThreadManager,Exists(I $) then
	       {OzcarMessage 'exception of attached thread'}
	       {{Dictionary.get self.ThreadDic I} printException(X)}
	    else
	       Q = {Thread.parentId T}
	    in
	       {OzcarMessage 'exception of unattached thread'}
	       ThreadManager,add(T I Q exc(X))
	    end

	 [] update(thr:T) then
	    I = {Thread.id T}
	 in
	    case ThreadManager,Exists(I $) then
	       Stack = {Dictionary.get self.ThreadDic I}
	    in
	       {Stack rebuild(true)}
	    else
	       {OzcarMessage
		'`update\' message of unattached thread -- ignoring'}
	    end

	 else
	    {OzcarError 'Unknown message on stream'}
	 end
      end

      %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

      meth Exists(I $)
	 {Dictionary.member self.ThreadDic I}
      end

      meth emptyForest($)
	 {Dictionary.keys self.ThreadDic} == nil
      end

      meth removeSkippedProcs(I)
	 SkippedProcs <- {Filter @SkippedProcs
			  fun {$ F} F.2 \= I end}
      end

      meth add(T I Q Exc<=unit)
	 IsFirstThread = ThreadManager,emptyForest($)
	 Stack = {New StackManager init(thr:T id:I)}
      in
	 {Dictionary.put self.ThreadDic I Stack}
	 {OzcarMessage 'adding thread #' # I # '/' # Q}
	 case Exc of exc(X) then   %% exception
	    Gui,addNode(I Q)
	    ThreadManager,switch(I false)
	    {Stack printException(X)}
	 else
	    Gui,addNode(I Q)
	    {Stack rebuild(true)}
	    case Q == 1 orelse       %% all compiler threads have id #1
	       IsFirstThread then    %% there's no other attached thread
	       ThreadManager,switch(I)
	       Gui,status('Selecting new thread #' # I)
	    else skip end
	 end
      end

      meth remove(T I Mode Select<=true)
	 Next in
	 {OzcarMessage 'removing thread #' # I # ' with mode ' # Mode}
	 ThreadManager,removeSkippedProcs(I)
	 case Mode == kill then
	    Gui,killNode(I Next)
	    {OzcarMessage 'next node is #' # Next}
	    {Dictionary.remove self.ThreadDic I}
	    case ThreadManager,emptyForest($) then
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
	    Gui,markNode(I dead)
	 end
	 case T == @currentThread then
	    case Mode == kill then
	       case ThreadManager,emptyForest($) then skip else
		  case Select then
		     detachDone <- _
		     ThreadManager,switch(Next)
		     Gui,status(', new selected thread is #' # Next append)
		  else skip end
	       end
	    else
	       Gui,status('Thread #' # I # ' died')
	       {SendEmacs removeBar}
	       Gui,markStack(active)
	       Gui,printStack(id:I frames:nil depth:0)
	    end
	 else skip end
      end

      meth kill(T I Select<=true)
	 {Dbg.trace T false}
	 {Dbg.step T false}
	 {Thread.terminate T}
	 case Select then
	    Gui,doStatus('Thread #' # I # ' has been terminated')
	 else skip end
	 ThreadManager,remove(T I kill Select)
      end

      meth termAll
	 {ForAll {Dictionary.items self.ThreadDic}
	  proc {$ S}
	     I = {S getId($)}
	     T = {S getThread($)}
	  in
	     ThreadManager,kill(T I false)
	     Gui,doStatus('.' append)
	  end}
      end

      meth termAllButCur
	 {ForAll {Dictionary.items self.ThreadDic}
	  proc {$ S}
	     I = {S getId($)}
	     T = {S getThread($)}
	  in
	     case T == @currentThread then skip else
		ThreadManager,kill(T I false)
		Gui,doStatus('.' append)
	     end
	  end}
      end

      meth detachAll
	 {ForAll {Dictionary.items self.ThreadDic}
	  proc {$ S}
	     I = {S getId($)}
	     T = {S getThread($)}
	  in
	     {Detach T}
	     ThreadManager,remove(T I kill false)
	     Gui,doStatus('.' append)
	  end}
      end

      meth detachAllButCur
	 {ForAll {Dictionary.items self.ThreadDic}
	  proc {$ S}
	     I = {S getId($)}
	     T = {S getThread($)}
	  in
	     case T == @currentThread then skip else
		{Detach T}
		ThreadManager,remove(T I kill false)
		Gui,doStatus('.' append)
	     end
	  end}
      end

      meth detachAllDead
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
	 case @currentThread \= unit andthen
	    {Thread.state @currentThread} == terminated then
	    thread
	       {Delay 1000}   %% give the user a chance to read the ' done' ;)
	       Gui,nextThread %% select next living thread
	    end
	 else skip end
      end

      meth detach(T I)
	 {Detach T}
	 Gui,doStatus('Thread #' # I # ' has been detached')
	 ThreadManager,remove(T I kill)
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
	       {OzcarMessage
		'no position information -- continuing thread #' # I}
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
%	 case {CondSelect {@currentStack getTop($)} dir entry} of exit then
%	    ThreadManager,rebuildCurrentStack
%	 else skip end
      end

      meth rebuildCurrentStack
	 Stack = @currentStack
      in
	 case Stack == unit then
%	    Gui,doStatus(FirstSelectThread)
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

	 case {IsFree @switchDone} then skip else
	    switchDone <- _
	 end

	 Gui,selectNode(I)

	 thread
	    {WaitOr New {Alarm {Cget timeoutToSwitch}}}
	    case {IsDet New} then skip else
	       ThreadManager,DoSwitch(I PrintStack)
	    end
	 end
      end

      meth DoSwitch(I PrintStack)
	 Stack = {Dictionary.get self.ThreadDic I}
	 T     = {Stack getThread($)}
	 S     = {CheckState T}
      in
	 case @currentStack == unit then skip else
	    Gui,resetLastSelectedFrame
	 end

	 {OzcarMessage 'switching to thread #' # I}
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

	 @switchDone = unit
	 @detachDone = unit
      end

      meth toggleEmacsThreads
	 {Ctoggle emacsThreads}
	 case {Cget emacsThreads} then
	    {EnqueueCompilerQuery setSwitch(runwithdebugger true)}
	 else
	    {EnqueueCompilerQuery setSwitch(runwithdebugger false)}
	 end
      end

      meth toggleSubThreads
	 {Ctoggle subThreads}
      end
   end
end
