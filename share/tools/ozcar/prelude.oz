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

\insert string
\insert tk

%% some builtins...
Dbg = dbg(on:           proc {$}
			   {System.set internal(debug:true)}
			end
	  off:          proc {$}
			   {System.set internal(debug:false)}
			end
	  stream:       {`Builtin` 'Debug.getStream' 1}
	  step:         {`Builtin` 'Debug.setStepFlag' 2}
	  trace:        {`Builtin` 'Debug.setTraceFlag' 2}
	  checkStopped: {`Builtin` 'Debug.checkStopped' 2}
	  unleash:      {`Builtin` 'Thread.unleash' 2}
	 )

UserActionLock = {NewLock}

proc {EnqueueCompilerQuery M}
   {Emacs.condSend.compiler enqueue(M)}
end

%% send a warning/error message
proc {OzcarShow X}
   case {Cget verbose} then
      {System.show X}
   else skip end
end
proc {OzcarMessage M}
   case {Cget verbose} then
      {System.showInfo {OzcarMessagePrefix} # M}
   else skip end
end
proc {OzcarError M}
   {System.showError OzcarErrorPrefix # M}
end

fun {V2VS X}
   Depth = {Cget printDepth}
   Width = {Cget printWidth}
in
   {System.valueToVirtualString X Depth Width}
end

proc {SendEmacs M}
   case {Cget useEmacsBar} then
      {Emacs.condSend.interface M}
   else skip end
end

ValuesHelp             = {NewName}
BreakpointStaticHelp   = {NewName}
BreakpointDynamicHelp  = {NewName}
StatusHelp             = {NewName}

fun {CheckState T}
   S = {Thread.state T}
in
   case     {Dbg.checkStopped T} then S
   elsecase S == terminated      then S
   else                               running end
end

local
   fun {MakeSpace N}
      case N < 1 then nil else 32 | {MakeSpace N-1} end
   end
in
   fun {PrintF S N}
      %% Format S to have length N, fill up with spaces -- break up
      %% line if S is too long
      SpaceCount = N - {VirtualString.length S}
   in
      case SpaceCount < 1 then
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
