%%%
%%% Authors:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
%%%   Leif Kornstaedt, 2001
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

   fun {AppOK Data}
      {Not {IsDet Data}} orelse
      case {System.printName Data}
      of 'Value.\'.\'' then {Cget stepDotBuiltin}
      [] 'Name.new'    then {Cget stepNewNameBuiltin}
      else true
      end
   end

   fun {FindEntry Ss T}
      case Ss of S|Sr then
	 if {S getThread($)} == T then S
	 else {FindEntry Sr T}
	 end
      [] nil then unit
      end
   end

   fun {RemoveEntry Ss T}
      case Ss of S|Sr then
	 if {S getThread($)} == T then Sr
	 else S|{RemoveEntry Sr T}
	 end
      end
   end

   class ThreadDict
      feat Dict
      meth init()
	 self.Dict = {NewDictionary}
      end
      meth put(T Stack) I in
	 I = {Primitives.getThreadId T}
	 {Dictionary.put self.Dict I
	  Stack|{Dictionary.condGet self.Dict I nil}}
      end
      meth get(T $)
	 case {Dictionary.condGet self.Dict {Primitives.getThreadId T} unit}
	 of unit then unit
	 elseof Ss then {FindEntry Ss T}
	 end
      end
      meth remove(T) I in
	 I = {Primitives.getThreadId T}
	 case {RemoveEntry {Dictionary.get self.Dict I} T} of nil then
	    {Dictionary.remove self.Dict I}
	 elseof Ss then
	    {Dictionary.put self.Dict I Ss}
	 end
      end
      meth size($)
	 {FoldR {Dictionary.items self.Dict}
	  fun {$ Ss In} {Length Ss} + In end 0}
      end
      meth isEmpty($)
	 {Dictionary.isEmpty self.Dict}
      end
      meth items($)
	 {FoldR {Dictionary.items self.Dict} Append nil}
      end
      meth isKnownI(I $) %--** this causes problems
	 {Dictionary.member self.Dict I}
      end
   end

in

   class ThreadManager
      feat
	 ThreadDic             %% dictionary that maps ids of attached threads
			       %% to StackManager instances
      attr
	 ReadLoopThread : unit

	 currentThread  : unit
	 currentStack   : unit

	 SwitchSync     : _
	 switchDone     : unit

      meth init
	 self.ThreadDic = {New ThreadDict init()}
	 thread
	    ReadLoopThread <- {Thread.this}
	    for M in {Primitives.getEventStream} do
	       try
		  ThreadManager,ReadStreamMessage(M)
	       catch E then
		  {OzcarError 'Whoooops, got an unhandled exception:'}
		  {Error.printException E}
	       end
	    end
	 end
      end

      meth destroy
	 Gui,status('Destroying myself -- byebye...')
	 {Primitives.setMode false}
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
	    I = {Primitives.getThreadId T}
	    P = {Primitives.getParentId T}
	    N = {self.ThreadDic size($)}
	 in
	    Gui,status(N # ' attached thread' #
		       if N > 1 then 's, currently selected: ' else ': ' end
		       # I # '/' # {Value.toVirtualString P 0 0} # ' (' #
		       case {Primitives.threadState T}
		       of terminated      then 'terminated'
		       [] runnable        then 'not stopped, runnable'
		       [] blocked         then 'not stopped, blocked'
		       [] stoppedRunnable then 'stopped, runnable'
		       [] stoppedBlocked  then 'stopped, blocked'
		       end # ')')
	 end
      end

      meth getThreadDic($)   %--**
	 self.ThreadDic
      end

      meth ReadStreamMessage(M)
	 lock UserActionLock then skip end %% don't process messages too fast..
	 {OzcarMessage 'message: '#{Value.toVirtualString M 5 5}}
	 case M of breakpoint(thr:T) then
	    case {self.ThreadDic get(T $)} of unit then skip
	    elseof S then
	       N = {Primitives.getThreadName T}
	    in
	       {OzcarMessage 'breakpoint reached by attached thread ' # N}
	       Gui,status('Thread ' # N # ' has reached a breakpoint')
	       {S rebuild(true)}
	       {S setAtBreakpoint(true)}
	    end
	 [] entry(thr:T frameID:FrameID ...) then
	    case {self.ThreadDic get(T $)} of unit then
	       %% this thread is not (yet) attached
	       if {Not {self.ThreadDic isKnownI({Primitives.getParentId T} $)}}
	       then
		  ThreadManager,Add(T)
	       elsecase Gui,checkSubThreads($) of !IgnoreText then
		  {OzcarMessage 'ignoring new subthread'}
		  {Primitives.detach T}
	       elseof S then
		  ThreadManager,Add(T)
		  case S of !AttachText then skip
		  [] !Unleash0Text then {Primitives.unleash T 0 false}
		  [] !Unleash1Text then {Primitives.unleash T 6 false}
		  end
	       end
	    elseof S then Data in
	       %% thread already attached
	       Data = {CondSelect M data unit}
	       if {AppOK Data} then
		  ThreadManager,M
	       else
		  {OzcarMessage ('skipping procedure \'' #
				 {System.printName Data} # '\'')}
		  {S addToSkippedProcs(FrameID)}
		  {Primitives.resume T}
	       end
	    end
	 [] exit(thr:T frameID:FrameID ...) then
	    case {self.ThreadDic get(T $)} of unit then skip
	    elseof S then
	       if {S isSkippedProc(FrameID $)} then
		  {OzcarMessage
		   'ignoring `exit\' message of ignored application'}
		  {S removeSkippedProc(FrameID)}
		  {Primitives.resume T}
	       else
		  Gui,markNode(T stoppedRunnable)
		  Gui,markStack(active)
		  if T == @currentThread then
		     {S exit(M)}
		     {SendEmacsBar
		      {CondSelect M file ''}
		      {CondSelect M line unit}
		      {CondSelect M column unit} stoppedRunnable}
		     {S printTop}
		  end
	       end
	    end
	 [] term(thr:T) then
	    case {self.ThreadDic get(T $)} of unit then skip
	    else
	       ThreadManager,MarkDead(T)
	    end
	 [] blocked(thr:T) then
	    case {self.ThreadDic get(T $)} of unit then skip
	    else
	       Gui,markNode(T blocked)
	    end
	 [] ready(thr:T) then
	    case {self.ThreadDic get(T $)} of unit then skip
	    else
	       S = {Primitives.threadState T}
	       Stopped = case S
			 of stoppedRunnable then true
			 [] stoppedBlocked  then true
			 else false
			 end
	    in
	       Gui,markNode(T S)
	       if Stopped andthen T == @currentThread then
		  {SendEmacs configureBar(runnable)}
	       end
	    end
	 [] exception(thr:T exc:system(kernel(terminate) ...)) then
	    case {self.ThreadDic get(T $)} of unit then skip
	    else
	       ThreadManager,MarkDead(T)
	    end
	 [] exception(thr:T exc:X) then
	    case {self.ThreadDic get(T $)} of unit then
	       {OzcarMessage 'exception of unattached thread'}
	       ThreadManager,Add(T exc(X))
	    elseof S then
	       {OzcarMessage 'exception of attached thread'}
	       Gui,markNode(T exc)
	       {S setException(X)}
	       Gui,status({S getException($)} clear ExcThreadColor)
	    end
	 [] update(thr:T) then
	    case {self.ThreadDic get(T $)} of unit then skip
	    elseof S then
	       {S rebuild(true)}
	    end
	 end
      end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

      meth emptyForest($)
	 {self.ThreadDic isEmpty($)}
      end

      meth Add(T Exc<=unit)
	 Q = {Primitives.getParentId T}
	 IsFirstThread = ThreadManager,emptyForest($)
	 Stack = {New StackManager init(T)}
      in
	 {self.ThreadDic put(T Stack)}
	 {OzcarMessage
	  'attaching thread ' # {Primitives.getThreadName T} # '/' #
	  {Value.toVirtualString Q 0 0}}
	 case Exc of exc(X) then   %% exception
	    Gui,addNode(T exc)
	    ThreadManager,switch(T false)
	    {Stack setException(X)}
	    Gui,status({Stack getException($)} clear ExcThreadColor)
	 else
	    Gui,addNode(T stoppedRunnable)
	    {Stack rebuild(true)}
	    if Q == 1                 %% all compiler threads have id #1
	       orelse IsFirstThread   %% there's no other attached thread
	    then
	       ThreadManager,switch(T)
	       Gui,status('Selecting new thread ' #
			  {Primitives.getThreadName T})
	    end
	 end
      end

      meth Remove(T Select) NextT in
	 {OzcarMessage 'removing thread ' # {Primitives.getThreadName T}}
	 Gui,removeNode(T ?NextT)
	 {self.ThreadDic remove(T)}
	 if ThreadManager,emptyForest($) then
	    currentThread <- unit
	    currentStack  <- unit
	    {SendEmacs removeBar}
	    Gui,selectNode(unit)
	    Gui,clearStack
	    if Select then
	       Gui,status(', thread forest now empty' append)
	    end
	 elseif T == @currentThread andthen Select then
	    ThreadManager,switch(NextT)
	    Gui,status(', selected thread ' # {Primitives.getThreadName NextT}
		       append)
	 end
      end

      meth MarkDead(T)
	 {OzcarMessage 'thread ' # {Primitives.getThreadName T} # ' died'}
	 Gui,markNode(T terminated)
	 if T == @currentThread then
	    Gui,status('Thread ' # {Primitives.getThreadName T} # ' died')
	    {SendEmacs removeBar}
	    Gui,markStack(active)
	    Gui,printStack(thr:T frames:nil)
	 end
      end

      meth kill(T Select)
	 {Primitives.terminate T}
	 if Select then
	    Gui,status('Thread ' # {Primitives.getThreadName T} #
		       ' has been terminated')
	 end
	 ThreadManager,Remove(T Select)
      end

      meth termAll
	 for S in {self.ThreadDic items($)} do
	    T = {S getThread($)}
	 in
	    ThreadManager,kill(T false)
	    Gui,status('.' append)
	 end
      end

      meth termAllButCur
	 for S in {self.ThreadDic items($)} do
	    T = {S getThread($)}
	 in
	    if T \= @currentThread then
	       ThreadManager,kill(T false)
	       Gui,status('.' append)
	    end
	 end
      end

      meth detachAll
	 for S in {self.ThreadDic items($)} do
	    T = {S getThread($)}
	 in
	    {Primitives.detach T}
	    ThreadManager,Remove(T false)
	    Gui,status('.' append)
	 end
      end

      meth detachAllButCur
	 for S in {self.ThreadDic items($)} do
	    T = {S getThread($)}
	 in
	    if T \= @currentThread then
	       {Primitives.detach T}
	       ThreadManager,Remove(T false)
	       Gui,status('.' append)
	    end
	 end
      end

      meth detachAllDead
	 for S in {self.ThreadDic items($)} do
	    T = {S getThread($)}
	 in
	    case {Primitives.threadState T} of terminated then
	       ThreadManager,Remove(T false)
	       Gui,status('.' append)
	    end
	 end
	 if @currentThread \= unit
	    andthen {Primitives.threadState @currentThread} == terminated
	 then
	    thread
	       {Delay 1000}   %% give the user a chance to read the ' done' ;)
	       Gui,nextThread %% select next living thread
	    end
	 end
      end

      meth detach(T)
	 {Primitives.detach T}
	 Gui,status('Thread ' # {Primitives.getThreadName T} #
		    ' has been detached')
	 ThreadManager,Remove(T true)
      end

      meth entry(thr: T ...)=Frame
	 Stack = {self.ThreadDic get(T $)}
      in
	 Gui,markNode(T stoppedRunnable)
	 Gui,markStack(active)
	 {Stack entry(Frame)}
	 if T == @currentThread then
	    {SendEmacsBar
	     {CondSelect Frame file ''}
	     {CondSelect Frame line unit}
	     {CondSelect Frame column unit}
	     if {Stack atBreakpoint($)} then stoppedBlocked
	     else stoppedRunnable
	     end}
	    {Stack printTop}
	 end
      end

      meth rebuildCurrentStack
	 S = @currentStack
	 T = @currentThread
      in
	 if S == unit then
	    Gui,status(NoThreads)
	 elsecase {Primitives.threadState T}
	 of runnable then
	    Gui,status('Cannot recalculate stack while thread is running')
	 [] blocked  then
	    Gui,status('Cannot recalculate stack while thread is running')
	 else
	    Gui,status('Recalculating stack of thread ' #
		       {Primitives.getThreadName T} # '...')
	    {S rebuild(true)}
	    {S print}
	    {S emacsBarToTop}
	    Gui,status(' done' append)
	 end
      end

      meth switch(T PrintStack<=true) New in
	 SwitchSync <- New = unit
	 if {IsDet @switchDone} then
	    switchDone <- _
	 end
	 thread
	    {WaitOr New {Alarm {Cget timeoutToSwitch}}}
	    if {IsFree New} then
	       Gui,selectNode(T)
	       ThreadManager,DoSwitch(T PrintStack)
	    end
	 end
      end

      meth DoSwitch(T PrintStack)
	 Stack = {self.ThreadDic get(T $)}
      in
	 if @currentStack \= unit then
	    CurT = @currentThread
	    CurS = @currentStack
	    Running = case {Primitives.threadState CurT}
		      of runnable then true
		      [] blocked  then true
		      else false
		      end
	 in
	    Gui,unselectStackFrame
	    if Running then
	       {CurS rebuild(true)}
	    end
	 end

	 {OzcarMessage 'switching to thread ' # {Primitives.getThreadName T}}
	 currentThread <- T
	 currentStack  <- Stack

	 if PrintStack then
	    case {Primitives.threadState T} of terminated then
	       {SendEmacs removeBar}
	       Gui,printStack(thr:T frames:nil)
	    elseof S then F L C in
	       {Stack print}
	       {Stack getPos(file:?F line:?L column:?C)}
	       case {Stack getException($)} of unit then
		  {SendEmacsBar F L C
		   if {Stack atBreakpoint($)} then stoppedBlocked
		   else S
		   end}
	       elseof Exc then
		  {SendEmacsBar F L C stoppedBlocked}
		  Gui,status(Exc clear ExcThreadColor)
	       end
	    end
	 end

	 @switchDone = unit
      end
   end
end
