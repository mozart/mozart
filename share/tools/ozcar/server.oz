%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 2001
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   Connection(offerUnlimited)
   Property(get)
   Primitives at 'OzcarPrimitives.ozf'
export
   Start
   %% Global Operations
   SetMode
   GetEventStream
   BreakpointAt
   %% Thread Operations
   GetThreadId
   GetThreadName
   GetParentId
   GetStack
   GetEnvironment
   ThreadState
   Suspend
   Resume
   Unleash
   Detach
   Terminate
define
   class Server
      prop locking
      attr
	 Ticket: unit
	 CommonEventPort: unit
	 StreamSnapshot: unit
	 Connections: nil
	 Mode: false
      meth init() EventStream in
	 Mode <- {Property.get 'internal.debug'}
	 CommonEventPort <- {NewPort ?EventStream}
	 StreamSnapshot <- EventStream
	 thread Server, MakeSnapshots(EventStream) end
	 thread
	    for Event in {Primitives.getEventStream} do
	       {Send @CommonEventPort Event}
	    end
	 end
      end
      meth MakeSnapshots(Es)
	 case Es of _|Er then
	    StreamSnapshot <- Er
	    Server, MakeSnapshots(Er)
	 end
      end
      meth start($)
	 lock
	    if @Ticket == unit then ConnectionPort in
	       Ticket <- {Connection.offerUnlimited
			  fun {$ SiteName CommandPort}
			     {Send ConnectionPort SiteName#CommandPort#$}
			  end}
	       thread
		  for _#CommandPort#Result in {NewPort $ ?ConnectionPort} do
		     RemoteEventStream
		  in
		     lock Old in
			Old = Connections <- CommandPort|Old
			Result = {NewPort ?RemoteEventStream}#@Mode
		     end
		     thread
			%--** terminate this thread when removing a connection
			for Event in RemoteEventStream do
			   {Send @CommonEventPort Event}
			end
		     end
		  end
	       end
	    end
	    @Ticket
	 end
      end
      meth setMode(OnOff)
	 lock
	    Mode <- OnOff
	    for P in @Connections do Server, send(P setMode(OnOff)) end
	 end
      end
      meth getEventStream($)
	 @StreamSnapshot
      end
      meth breakpointAt(File Line Enabled $)
	 {FoldR {Map @Connections
		 fun {$ P}
		    Server, send(P breakpointAt(File Line Enabled $))
		 end} Or false}
      end
      meth send(P X)
	 try
	    {Send P X}
	 catch system(dp(conditions:[permFail] ...) ...) then
	    lock
	       Connections <- {Filter @Connections fun {$ Q} P \= Q end}
	    end
	    case X of getStack(_ _ _ ?Stack) then
	       Stack = nil
	    [] getEnvironment(_ _ ?Environment) then
	       Environment = unit
	    [] threadState(_ ?State) then
	       State = terminated
	    else skip
	    end
	    {Send @CommonEventPort term(thr: X.1)}
	 end
      end
   end

   TheServer = {New Server init()}

   fun {Start}
      {TheServer start($)}
   end

   proc {SetMode OnOff}
      {Primitives.setMode OnOff}
      {TheServer setMode(OnOff)}
   end

   fun {GetEventStream}
      {TheServer getEventStream($)}
   end

   fun {BreakpointAt File Line Enabled}
      {Or
       {Primitives.breakpointAt File Line Enabled}
       {TheServer breakpointAt(File Line Enabled $)}}
   end

   fun {GetThreadId T}
      if {IsThread T} then {Primitives.getThreadId T}
      elsecase T of 'thread'(id: Id ...) then Id
      end
   end

   fun {GetThreadName T}
      if {IsThread T} then {Primitives.getThreadName T}
      elsecase T of 'thread'(name: Name ...) then Name
      end
   end

   fun {GetParentId T}
      if {IsThread T} then {Primitives.getParentId T}
      elsecase T of 'thread'(parent: Id ...) then Id
      end
   end

   fun {GetStack T Count Verbose}
      if {IsThread T} then {Primitives.getStack T Count Verbose}
      elsecase T of 'thread'(port: P ...) then
	 {TheServer send(P getStack(T Count Verbose $))}
      end
   end

   fun {GetEnvironment T FrameID}
      if {IsThread T} then {Primitives.getEnvironment T FrameID}
      elsecase T of 'thread'(port: P ...) then
	 {TheServer send(P getEnvironment(T FrameID $))}
      end
   end

   fun {ThreadState T}
      if {IsThread T} then {Primitives.threadState T}
      elsecase T of 'thread'(port: P ...) then
	 {TheServer send(P threadState(T $))}
      end
   end

   proc {Suspend T}
      if {IsThread T} then {Primitives.suspend T}
      elsecase T of 'thread'(port: P ...) then {TheServer send(P suspend(T))}
      end
   end

   proc {Resume T}
      if {IsThread T} then {Primitives.resume T}
      elsecase T of 'thread'(port: P ...) then {TheServer send(P resume(T))}
      end
   end

   proc {Unleash T FrameID DoStep}
      if {IsThread T} then {Primitives.unleash T FrameID DoStep}
      elsecase T of 'thread'(port: P ...) then
	 {TheServer send(P unleash(T FrameID DoStep))}
      end
   end

   proc {Detach T}
      if {IsThread T} then {Primitives.detach T}
      elsecase T of 'thread'(port: P ...) then {TheServer send(P detach(T))}
      end
   end

   proc {Terminate T}
      if {IsThread T} then {Primitives.terminate T}
      elsecase T of 'thread'(port: P ...) then {TheServer send(P terminate(T))}
      end
   end
end
