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

\insert string
\insert tk

%% some builtins...
Dbg = dbg(on:           proc {$}
			   {Property.put 'internal.debug' true}
			end
	  off:          proc {$}
			   {Property.put 'internal.debug' false}
			end
	  stream:       Debug.getStream
	  step:         Debug.setStepFlag
	  trace:        Debug.setTraceFlag
	  checkStopped: fun {$ T}
			   try
			      {Thread.isSuspended T}
			   catch error(kernel(deadThread ...) ...) then
			      false
			   end
			end
	  unleash:      Debug.threadUnleash
	 )

UserActionLock = {NewLock}

proc {EnqueueCompilerQuery M}
   {Emacs.condSend.compiler enqueue(M)}
end

%% send a warning/error message
proc {OzcarShow X}
   if {Cget verbose} then
      {System.show X}
   end
end
proc {OzcarMessage M}
   if {Cget verbose} then
      {System.showInfo {OzcarMessagePrefix} # M}
   end
end
proc {OzcarError M}
   {System.showError OzcarErrorPrefix # M}
end

fun {V2VS X}
   Depth = {Cget printDepth}
   Width = {Cget printWidth}
in
   {Value.toVirtualString X Depth Width}
end

proc {SendEmacs M}
   if {Cget useEmacsBar} then
      case {Cget emacsInterface} of false then skip
      elseof I then {I M}
      end
   end
end

ValuesHelp             = {NewName}
BreakpointStaticHelp   = {NewName}
BreakpointDynamicHelp  = {NewName}
StatusHelp             = {NewName}

fun {CheckState T}
   S = {Thread.state T}
in
   if {Dbg.checkStopped T} orelse S == terminated then S else running end
end

local
   fun {MakeSpace N}
      if N < 1 then nil else 32 | {MakeSpace N-1} end
   end
in
   fun {PrintF S N}
      %% Format S to have length N, fill up with spaces -- break up
      %% line if S is too long
      SpaceCount = N - {VirtualString.length S}
   in
      if SpaceCount < 1 then
	 S # '\n' # {MakeSpace N}
      else
	 S # {MakeSpace SpaceCount}
      end
   end
end

fun {UnknownFile F}
   F == ''
end

fun {StripPath File}
   F = {Atom.toString File}
   S = {Str.rchr F &/}
in
   case S of _|R then R else F end
end
