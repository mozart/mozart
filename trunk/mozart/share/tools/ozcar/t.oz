{Browse {Ozcar list($)}}

declare
T = {Ozcar list($)}.1
D = {Thread.getName T}

{D step}
{D stack}

{Browse D}


declare
local
   TS   = {`Builtin` taskstack 2}
   Children = {`Builtin` 'Thread.children' 2}
   Parent = {`Builtin` 'Thread.parent' 2}
   Root = {Parent {Thread.this}}
in
   fun {CurrentThreads T}
      C = {Children T}
   in
      {Thread.state T}#{Thread.id T}#{Thread.id {Parent T}}#{TS T}#
      case C == nil then
	 nochildren
      else
	 {Map C CurrentThreads}
      end
   end
   {Browse {CurrentThreads Root}}
end

