%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>


%% some builtins...

local
   
   TS  = {`Builtin` taskstack 2}
   SS  = {`Builtin` globalThreadStream 1}
   P   = {`Builtin` 'Thread.parent' 2}
   C   = {`Builtin` 'Thread.children' 2}
   TID = {`Builtin` 'getThreadByID' 2}
   SM  = {`Builtin` setStepMode 2}
   QDS = {`Builtin` queryDebugState 2}
   
in
   
   Dbg = dbg( taskstack      : TS
	      stream         : SS
	      parent         : P
	      children       : C
	      threadByID     : TID
	      stepmode       : SM
	      state          : QDS
	    )
end


%% send a suspend/resume message to all subthreads of a given thread
%% (and to the start thread itself!)

proc {KillAll M T}
   C = {Dbg.children T}
in
   {Thread.M T}
   case  C \= nil then
      {ForAll C proc {$ T} {KillAll M T} end}
   else skip
   end
end


%% send a warning/error message

proc {Message M}
   Prefix = "Message from Ozcar: "
in
   {System.showInfo Prefix # M}
end

proc {DebugMessage M}
   {Show M}
end
