%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

\insert string
\insert tk

%% some builtins...
Dbg = dbg( taskstack   : {`Builtin` taskstack 3}
	   frameVars   : {`Builtin` frameVariables 3}
	   stream      : {`Builtin` globalThreadStream 1}
	   contflag    : {`Builtin` setContFlag 2}
	   stepmode    : {`Builtin` setStepMode 2}
	   trace       : {`Builtin` traceThread 2}
	 )

%% some constants
NL = [10]  %% newline

%% send a warning/error message

proc {OzcarShow X}
   case {Cget verbose} then
      {Show X}
   else skip end
end
proc {OzcarMessage M}
   case {Cget verbose} then
      {System.showInfo OzcarMessagePrefix # M}
   else skip end
end

fun {VS2A X} %% virtual string to atom
   {String.toAtom {VirtualString.toString X}}
end

%% Dictionary.xxx is too long, really...
Dput     = Dictionary.put
Dentries = Dictionary.entries
Dkeys    = Dictionary.keys
Dget     = Dictionary.get
Ditems   = Dictionary.items
Dremove  = Dictionary.remove
Dmember  = Dictionary.member

local
   fun {MakeSpace N}
      case N < 1 then nil else 32 | {MakeSpace N-1} end
   end
in
   fun {PrintF S N}  %% Format S to have length N, fill up with spaces
                     %% break up line if S is too long
      SpaceCount = N - {VirtualString.length S}
   in
      case SpaceCount < 1 then
	 S # NL # {MakeSpace N}
      else
	 S # {MakeSpace SpaceCount}
      end
   end
end

fun {LookupPath F}
   %% heuristic: if F begins with '.?.' then it's a prelude file
   FS = {Atom.toString F}
in
   case FS.1 == &. andthen FS.2.2.1 == &. then
      {VS2A {System.get home} # '/lib/' # FS.2.2.2.2}
   else
      F
   end
end

fun {UnknownFile F}
   F == undef orelse F == '' orelse F == noDebugInfo orelse F == nofile
end

fun {StripPath File}
   case File == '' then
      '???'
   else
      S = {Str.rchr {Atom.toString File} &/}
   in
      case {List.length S} > 1 then
	 S.2
      else
	 '???'
      end
   end
end

