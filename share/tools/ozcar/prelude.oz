%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

\insert string
\insert tk

%% some builtins...
Dbg = dbg( taskstack   : {`Builtin` taskstack 3}
	   emacsThreads: {`Builtin` debugEmacsThreads 1}
	   subThreads  : {`Builtin` debugSubThreads 1}
	   frameVars   : {`Builtin` frameVariables 3}
	   stream      : {`Builtin` globalThreadStream 1}
	   contflag    : {`Builtin` setContFlag 2}
	   stepmode    : {`Builtin` setStepMode 2}
	   trace       : {`Builtin` traceThread 2}
	   checkStopped: {`Builtin` checkStopped 2}
	 )

fun {NewCompiler}
   %% return true when using new compiler, false otherwise
   try {{`Builtin` 'getOPICompiler' 1} _} true
   catch error(...) then false
   end
end

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
      {System.showInfo {OzcarMessagePrefix} # M}
   else skip end
end
proc {OzcarError M}
   {System.showInfo OzcarErrorPrefix # M}
end

%% a null object which ignores all messages
NullObject = {New class meth otherwise(M) skip end end ''}

%% Dictionary.xxx is too long, really...
Dput     = Dictionary.put
Dentries = Dictionary.entries
Dkeys    = Dictionary.keys
Dget     = Dictionary.get
Ditems   = Dictionary.items
Dremove  = Dictionary.remove
Dmember  = Dictionary.member
DcondGet = Dictionary.condGet

fun {V2VS X}
   P = {System.get errors}
in
   {System.valueToVirtualString X P.depth P.width}
end

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


%% file lookup

local
   LS = 'file lookup: '
   fun {DoLookupFile SearchList F OrigF}
      case SearchList of nil then
	 %% must have been the name of an unsaved file or buffer in Emacs:
	 OrigF
      elseof Path|SearchListRest then Try = Path # F in
	 try
	    case {OS.stat Try} == reg then
	       {OzcarMessage LS # F # ' is ' # Try}
	       {VS2A Try}
	    else
	       {OzcarMessage LS # F # ' is not ' # Try # ' (not a plain file)'}
	       {DoLookupFile SearchListRest F OrigF}
	    end
	 catch system(...) then
	    {OzcarMessage LS # F # ' is not ' # Try}
	    {DoLookupFile SearchListRest F OrigF}
	 end
      end
   end
in
   fun {LookupFile F}
      S   = {Atom.toString F}
      Abs = case S                   % absolute path?
	    of     &/|_   then true
	    elseof _|&:|_ then Platform == WindowsPlatform
	    else false end
   in
      case Abs then
	 %% the file doesn't need to exist, since it may be the name of
	 %% an unsaved buffer or file in Emacs:
	 F
      else                           % ...no!
	 %% strip "./" or "././"
	 Suffix = case S of &.|&/|T then
		     case T of &.|&/|R then R
		     else T end
		  else S end
      in
	 {DoLookupFile OzPath Suffix F}
      end
   end
end

fun {UnknownFile F}
   F == undef orelse F == '' orelse F == noDebugInfo orelse F == nofile
end

fun {StripPath File}
   case {UnknownFile File} then
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

