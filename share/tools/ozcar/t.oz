{Browse {Ozcar list($)}}

{Debug.on}

declare
T = {Ozcar list($)}.1
D = {Thread.getName T}

{D step}
{D cont}
{D stack}

{Thread.resume T}
{SM T off}

declare SM = 
{`Builtin` setStepMode 2}

{Browse D}

Y = 4711

\sw +optimize

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

declare
TS   = {`Builtin` taskstack 2}
Children = {`Builtin` 'Thread.children' 2}
Parent = {`Builtin` 'Thread.parent' 2}
Root = {Parent {Thread.this}}
proc {KillAll M T}
   C = {Children T}
in
   case  C \= nil then
      {Show C}
      {ForAll C proc {$ Th} {KillAll M Th} end}
   else skip
   end
   {Show thr({Thread.id T})}
   {Thread.M T}
end

{KillAll terminate {Parent {Thread.this}}}

{Browse {Thread.id {Thread.this}}}