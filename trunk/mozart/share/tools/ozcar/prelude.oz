%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

\insert string
\insert tk

%% some builtins...
Dbg = dbg( taskstack   : {`Builtin` taskstack 3}
	   suspend     : {`Builtin` suspendDebug 1}
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
proc {OzcarError M}
   {System.showInfo OzcarErrorPrefix # M}
end

S2A = String.toAtom  %% string to atom
fun {VS2A X}         %% virtual string to atom
   {S2A {VirtualString.toString X}}
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

local
   LS = 'lookup: '
   fun {DoLookupFile F SearchList}
      case SearchList == nil then
	 {OzcarError LS # F # ' NOT FOUND!'} % should not happen!
	 nil
      else Try = SearchList.1 # F in
	 try
	    {OS.stat Try _}
	    {OzcarMessage LS # F # ' actually is ' # Try}
	    {VS2A Try}
	 catch system(...) then
	    {OzcarMessage LS # F # ' is not ' # Try}
	    {DoLookupFile F SearchList.2}
	 end
      end
   end
in
   fun {LookupFile F}
      S = {Atom.toString F}
   in
      case S.1 == &/ then
	 try                   % absolute path
	    {OS.stat F _}
	    {OzcarMessage LS # F # ' really is ' # F}
	    F
	 catch system(...) then
	    {OzcarError LS # F # ' NOT FOUND!'} % should not happen!
	    nil
	 end
      else                     % relative path
	 %% strip "./" or "././"
	 Suffix = case S.1 == &. andthen S.2.1 == &/ then T = S.2.2 in
		     case T.1 == &. andthen T.2.1 == &/ then
			T.2.2
		     else T end
		  else S end
      in
	 {DoLookupFile Suffix OzPath} 
      end
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

