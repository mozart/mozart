%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

\insert string
\insert tk

%% some builtins...
Dbg = dbg(on:           proc {$}
			   {System.set internal(debug:true)}
			end
	  off:          proc {$}
			   {System.set internal(debug:false)}
			end
	  mode:         {`Builtin` 'Debug.mode' 1}
	  emacsThreads: {`Builtin` 'Debug.addEmacsThreads' 1}
	  subThreads:   {`Builtin` 'Debug.addSubThreads' 1}
	  stream:       {`Builtin` 'Debug.getStream' 1}
	  contflag:     {`Builtin` 'Debug.setContFlag' 2}
	  stepmode:     {`Builtin` 'Debug.setStepFlag' 2}
	  trace:        {`Builtin` 'Debug.setTraceFlag' 2}
	  checkStopped: {`Builtin` 'Debug.checkStopped' 2}
	 )

IsBuiltin = {`Builtin` 'isBuiltin' 2}

fun {NewCompiler}
   %% return true when using new compiler, false otherwise
   try {{`Builtin` 'getOPICompiler' 1} _} true
   catch error(...) then false
   end
end

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
Dnew       = Dictionary.new
Dput       = Dictionary.put
Dentries   = Dictionary.entries
Dkeys      = Dictionary.keys
Dget       = Dictionary.get
Ditems     = Dictionary.items
Dremove    = Dictionary.remove
Dmember    = Dictionary.member
DcondGet   = Dictionary.condGet
DremoveAll = Dictionary.removeAll

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
	 S # '\n' # {MakeSpace N}
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
   case F == '' then {OzcarError 'Warning: empty file name'} else skip end
   F == nofile orelse F == ''
end

fun {StripPath File}
   case {UnknownFile File} then
      '???'
   else
      S = {Str.rchr {Atom.toString File} &/}
   in
      case S of _|Rest then
	 Rest
      else
	 '???'
      end
   end
end

