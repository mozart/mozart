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
   Debug(getStream breakpointAt
	 getId getParentId
	 getTaskStack getFrameVariables
	 setStepFlag setTraceFlag threadUnleash) at 'x-oz://boot/Debug'
   Property(put)
export
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
   proc {SetMode OnOff}
      {Property.put 'internal.debug' OnOff}
   end

   GetEventStream = Debug.getStream
   BreakpointAt   = Debug.breakpointAt
   GetThreadId    = Debug.getId
   GetThreadName  = Debug.getId
   GetParentId    = Debug.getParentId
   GetStack       = Debug.getTaskStack
   GetEnvironment = Debug.getFrameVariables

   fun {ThreadState T}
      try
	 case {Thread.isSuspended T}#{Thread.state T}
	 of false#S then S
	 [] _#terminated then terminated
	 [] true#runnable then stoppedRunnable
	 [] true#blocked then stoppedBlocked
	 end
      catch error(kernel(deadThread ...) ...) then
	 terminated
      end
   end

   Suspend = Thread.suspend
   Resume  = Thread.resume

   proc {Unleash T FrameID DoStep}
      %% prerequisite: T is suspended
      {Debug.setStepFlag T DoStep}
      {Debug.threadUnleash T FrameID}
      {Thread.resume T}
   end

   proc {Detach T}
      try
	 {Debug.setTraceFlag T false}
	 {Debug.setStepFlag T false}
	 {Thread.resume T}
      catch error(kernel(deadThread ...) ...) then skip
      end
   end

   proc {Terminate T}
      try
	 {Debug.setTraceFlag T false}
	 {Debug.setStepFlag T false}
	 {Thread.terminate T}
      catch error(kernel(deadThread ...) ...) then skip
      end
   end
end
