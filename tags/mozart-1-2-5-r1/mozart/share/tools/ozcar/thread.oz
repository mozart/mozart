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
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.

local

   fun {AppOK Name}
      ({Cget stepDotBuiltin}     orelse Name \= 'Value.\'.\'')
      andthen
      ({Cget stepNewNameBuiltin} orelse Name \= 'Name.new')
   end

   proc {Detach T}
      try
	 {Dbg.trace T false}
	 {Dbg.step T false}
	 {Thread.resume T}
      catch
	 error(kernel(deadThread ...) ...)
      then skip end
   end

   proc {OzcarReadEvalLoop S}
      case S
      of H|T then
	 try
	    {Ozcar PrivateSend(readStreamMessage(H))}
	 catch E then
	    {OzcarError 'Whoooops, got an unhandled exception:'}
	    {Error.printException E}
	 end
	 {OzcarMessage 'OzcarReadEvalLoop: waiting for next message...'}
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
	 Gui,status('Destroying myself -- byebye...')
	 {Dbg.off}
	 {Thread.terminate @ReadLoopThread}
	 {EnqueueCompilerQuery setSwitch(debuginfo false)}
	 {SendEmacs removeBar}
	 {Delay 1000}
	 {self.toplevel tkClose}
      end

      meth checkMe
	 if ThreadManager,emptyForest($) then
	    Gui,status(NoThreads)
	 else
	    T = @currentThread
	    I = {Debug.getId T}
	    P = try {Debug.getParentId T} catch
		   error(kernel(deadThread ...) ...) then '?' end
	    R = if {Dbg.checkStopped T} then 'stopped' else 'not stopped' end
	    S = {Thread.state T}
	    N = {Length {Dictionary.items self.ThreadDic}}
	 in
	    Gui,status(N # ' attached thread' #
		       if N > 1 then 's, currently selected: ' else ': ' end
		       # I # '/' # P # ' (' #
		       if S == terminated then S else R # ', ' # S end # ')')
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
%        {OzcarShow @SkippedProcs}
	 {Thread.resume T}
      end

      meth readStreamMessage(M)

	 lock UserActionLock then skip end %% don't process messages too fast..

	 {OzcarMessage 'readStreamMessage: dispatching:'}
	 {OzcarShow M}

	 case M

	 of breakpoint(thr:T) then
	    I = {Debug.getId T}
	 in
	    if ThreadManager,Exists(I $) then %% already attached thread
	       M = 'Thread ' # I # ' has reached a breakpoint'
	       S = {Dictionary.get self.ThreadDic I}
	    in
	       {OzcarMessage 'breakpoint reached by attached thread ' # I}
	       Gui,status(M)
	       {S rebuild(true)}
	       {S setAtBreakpoint(true)}
	    else
	       {OzcarMessage ('`breakpoint\' message of unattached' #
			      ' thread -- ignoring')}
	    end

	 [] entry(thr:T ...) then
	    I = {Debug.getId T}
	 in
	    if ThreadManager,Exists(I $) then %% already attached thread
	       Name = {CondSelect M data unit}
	    in
	       if {Not {IsDet Name}}
		  orelse {AppOK {System.printName Name}}
	       then
		  ThreadManager,M
	       else
		  ThreadManager,AddToSkippedProcs(Name T I M.frameID)
	       end

	    else %% this is a (not yet) attached thread
	       Q = {Debug.getParentId T}
	       S = Gui,checkSubThreads($)
	    in
	       if S == AttachText orelse
		  {Not ThreadManager,Exists(Q $)} then
		  ThreadManager,add(T I Q)
	       else
		  case S
		  of !IgnoreText then
		     {OzcarMessage 'ignoring new subthread'}
		     {Detach T}
		  elseof U then
		     ThreadManager,add(T I Q)
		     {Dbg.step T false}
		     case U
		     of !Unleash0Text then
			{Dbg.unleash T 0}
		     [] !Unleash1Text then
			{Dbg.unleash T 6}
		     end
		     {Thread.resume T}
		  end
	       end
	    end

	 elseof exit(thr:T frameID:FrameId ...) then
	    I     = {Debug.getId T}
	    Key   = FrameId # I
	    Found = {Member Key @SkippedProcs}
	 in
%           {OzcarShow @SkippedProcs # Key # Found}
	    if Found then
	       {OzcarMessage 'ignoring `exit\' message of ignored application'}
	       SkippedProcs  <- {Filter @SkippedProcs fun {$ F} F \= Key end}
	       {Thread.resume T}
	    else
	       Gui,markNode(I stopped)  % thread is not running anymore
	       Gui,markStack(active)    % stack view has up-to-date content
	       if T == @currentThread then
		  Stack = {Dictionary.get self.ThreadDic I}
	       in
		  {Stack exit(M)}
		  {SendEmacs bar(file:{CondSelect M file ''}
				 line:{CondSelect M line unit}
				 column:{CondSelect M column unit}
				 state:runnable)}
		  {Stack printTop}
	       end
	    end

	 elseof term(thr:T) then
	    I = {Debug.getId T}
	 in
	    if ThreadManager,Exists(I $) then
	       ThreadManager,remove(T I noKill)
	    else
	       {OzcarMessage
		'`term\' message of unattached thread -- ignoring'}
	    end

	 [] blocked(thr:T) then
	    I = {Debug.getId T}
	 in
	    if ThreadManager,Exists(I $) then
	       ThreadManager,blocked(thr:T id:I)
	    else
	       {OzcarMessage
		'`blocked\' message of unattached thread -- ignoring'}
	    end

	 [] ready(thr:T) then
	    I = {Debug.getId T}
	 in
	    if ThreadManager,Exists(I $) then
	       Gui,markNode(I runnable)
	       if {Dbg.checkStopped T} then
		  Gui,markNode(I stopped)
		  if T == @currentThread then
		     {SendEmacs configureBar(runnable)}
		  end
	       end
	    else
	       {OzcarMessage
		'`ready\' message of unattached thread -- ignoring'}
	    end

	 [] exception(thr:T exc:X) then
	    I = {Debug.getId T}
	 in
	    case X of system(kernel(terminate) ...) then
	       if ThreadManager,Exists(I $) then
		  ThreadManager,remove(T I noKill)
	       end
	    else
	       if ThreadManager,Exists(I $) then
		  {OzcarMessage 'exception of attached thread'}
		  Gui,markNode(I exc)
		  Gui,markNode(I stopped)
		  {{Dictionary.get self.ThreadDic I} printException(X)}
	       else
		  Q = {Debug.getParentId T}
	       in
		  {OzcarMessage 'exception of unattached thread'}
		  ThreadManager,add(T I Q exc(X))
	       end
	    end

	 [] update(thr:T) then
	    I = {Debug.getId T}
	 in
	    if ThreadManager,Exists(I $) then
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
	 {OzcarMessage 'attaching thread ' # I # '/' # Q}
	 case Exc of exc(X) then   %% exception
	    Gui,addNode(I Q)
	    Gui,markNode(I exc)
	    ThreadManager,switch(I false)
	    {Stack printException(X)}
	 else
	    Gui,addNode(I Q)
	    {Stack rebuild(true)}
	    if Q == 1 orelse         %% all compiler threads have id #1
	       IsFirstThread then    %% there's no other attached thread
	       ThreadManager,switch(I)
	       Gui,status('Selecting new thread ' # I)
	    end
	 end
      end

      meth remove(T I Mode Select<=true)
	 Next in
	 {OzcarMessage 'removing thread ' # I # ' with mode ' # Mode}
	 ThreadManager,removeSkippedProcs(I)
	 if Mode == kill then
	    Gui,killNode(I Next)
	    {OzcarMessage 'next node is ' # Next}
	    {Dictionary.remove self.ThreadDic I}
	    if ThreadManager,emptyForest($) then
	       currentThread <- unit
	       currentStack  <- unit
	       {SendEmacs removeBar}
	       Gui,selectNode(0)
	       Gui,clearStack
	       if Select then
		  Gui,status(', thread tree is now empty' append)
	       end
	    end
	 else
	    Gui,markNode(I dead)
	    Gui,markNode(I stopped)
	 end
	 if T == @currentThread then
	    if Mode == kill then
	       if ThreadManager,emptyForest($) then skip else
		  if Select then
		     detachDone <- _
		     ThreadManager,switch(Next)
		     Gui,status(', new selected thread is ' # Next append)
		  end
	       end
	    else
	       Gui,status('Thread ' # I # ' died')
	       {SendEmacs removeBar}
	       Gui,markStack(active)
	       Gui,printStack(id:I frames:nil depth:0)
	    end
	 end
      end

      meth kill(T I Select<=true)
	 try
	    {Dbg.trace T false}
	    {Dbg.step T false}
	    {Thread.terminate T}
	 catch error(kernel(deadThread ...) ...) then skip end
	 if Select then
	    Gui,status('Thread ' # I # ' has been terminated')
	 end
	 ThreadManager,remove(T I kill Select)
      end

      meth termAll
	 {ForAll {Dictionary.items self.ThreadDic}
	  proc {$ S}
	     I = {S getId($)}
	     T = {S getThread($)}
	  in
	     ThreadManager,kill(T I false)
	     Gui,status('.' append)
	  end}
      end

      meth termAllButCur
	 {ForAll {Dictionary.items self.ThreadDic}
	  proc {$ S}
	     I = {S getId($)}
	     T = {S getThread($)}
	  in
	     if T \= @currentThread then
		ThreadManager,kill(T I false)
		Gui,status('.' append)
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
	     Gui,status('.' append)
	  end}
      end

      meth detachAllButCur
	 {ForAll {Dictionary.items self.ThreadDic}
	  proc {$ S}
	     I = {S getId($)}
	     T = {S getThread($)}
	  in
	     if T \= @currentThread then
		{Detach T}
		ThreadManager,remove(T I kill false)
		Gui,status('.' append)
	     end
	  end}
      end

      meth detachAllDead
	 {ForAll {Dictionary.items self.ThreadDic}
	  proc {$ S}
	     I = {S getId($)}
	     T = {S getThread($)}
	  in
	     if {Thread.state T} == terminated then
		ThreadManager,remove(T I kill false)
		Gui,status('.' append)
	     end
	  end}
	 if @currentThread \= unit andthen
	    {Thread.state @currentThread} == terminated then
	    thread
	       {Delay 1000}   %% give the user a chance to read the ' done' ;)
	       Gui,nextThread %% select next living thread
	    end
	 end
      end

      meth detach(T I)
	 {Detach T}
	 Gui,status('Thread ' # I # ' has been detached')
	 ThreadManager,remove(T I kill)
      end

      meth entry(thr: T ...)=Frame
	 I     = {Debug.getId T}
	 Stack = {Dictionary.get self.ThreadDic I}
      in
	 Gui,markNode(I stopped)  % thread is not running anymore
	 Gui,markStack(active)    % stack view has up-to-date content
	 {Stack entry(Frame)}
	 if T == @currentThread then
	    F = {CondSelect Frame file ''}
	    L = {CondSelect Frame line unit}
	    C = {CondSelect Frame column unit}
	 in
	    if {Stack atBreakpoint($)} then
	       {SendEmacs bar(file:F line:L column:C state:blocked)}
	    else
	       {SendEmacs bar(file:F line:L column:C state:runnable)}
	    end
	    {Stack printTop}
	 end
      end

      meth blocked(thr:T id:I)
	 Gui,markNode(I blocked)
	 Gui,markNode(I running)
%        if {CondSelect {@currentStack getTop($)} dir entry} of exit then
%           ThreadManager,rebuildCurrentStack
%        end
      end

      meth rebuildCurrentStack
	 S = @currentStack
	 T = @currentThread
      in
	 if S == unit then
	    Gui,status(NoThreads)
	 elseif {CheckState T} == running then
	    Gui,status('Cannot recalculate stack while thread is running')
	 else
	    Gui,status('Recalculating stack of thread ' #
		       {Debug.getId T} # '...')
	    {S rebuild(true)}
	    {S print}
	    {S emacsBarToTop}
	    Gui,status(' done' append)
	 end
      end

      meth switch(I PrintStack<=true)
	 New in
	 SwitchSync <- New = unit

	 if {IsFree @switchDone} then skip else
	    switchDone <- _
	 end

	 Gui,selectNode(I)

	 thread
	    {WaitOr New {Alarm {Cget timeoutToSwitch}}}
	    if {IsDet New} then skip else
	       ThreadManager,DoSwitch(I PrintStack)
	    end
	 end
      end

      meth DoSwitch(I PrintStack)
	 Stack = {Dictionary.get self.ThreadDic I}
	 T     = {Stack getThread($)}
	 S     = {CheckState T}
      in
	 if @currentStack \= unit then
	    CurT = @currentThread
	    CurS = @currentStack
	 in
	    Gui,resetLastSelectedFrame
	    if {CheckState CurT} == running then
	       {OzcarMessage 'setting `rebuild\' flag' #
		' of (running) thread ' # {Debug.getId CurT}}
	       {CurS rebuild(true)}
	    end
	 end

	 {OzcarMessage 'switching to thread ' # I}
	 currentThread <- T
	 currentStack  <- Stack

	 if PrintStack then
	    if S == terminated then
	       {SendEmacs removeBar}
	       Gui,printStack(id:I frames:nil depth:0)
	    else
	       F L C Exc = {Stack getException($)}
	    in
	       {Stack print}
	       {Stack getPos(file:?F line:?L column:?C)}
	       if Exc == nil then
		 if {Stack atBreakpoint($)} then
		    {SendEmacs bar(file:F line:L column:C state:blocked)}
		 else
		    {SendEmacs bar(file:F line:L column:C state:S)}
		 end
	       else
		  {SendEmacs bar(file:F line:L column:C state:blocked)}
		  Gui,status(Exc clear ExcThreadColor)
	       end
	    end
	 end

	 @switchDone = unit
	 @detachDone = unit
      end
   end
end
