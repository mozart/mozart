%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

\insert prof-string
\insert prof-tk

%% some builtins...
Profile = profile(getInfo: {`Builtin` statisticsGetProcs 1}
		  reset:   {`Builtin` statisticsReset 0} )

%% some constants
NL = [10]  %% newline

%% send a warning/error message
proc {ProfilerMessage M}
   %% disabled...
   %{System.showInfo {ProfilerMessagePrefix} # M}
   skip
end
proc {ProfilerError M}
   {System.showInfo ProfilerErrorPrefix # M}
end

%% a null object which ignores all messages
NullObject = {New class meth otherwise(M) skip end end ''}

fun {V2VS X}
   {System.valueToVirtualString X PrintDepth PrintWidth}
end
 
StatusHelp             = {NewName}

%% Dictionary.xxx is too long, really...
Dput     = Dictionary.put
Dentries = Dictionary.entries
Dkeys    = Dictionary.keys
Dget     = Dictionary.get
Ditems   = Dictionary.items
Dremove  = Dictionary.remove
Dmember  = Dictionary.member
DcondGet = Dictionary.condGet

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
   LS = 'lookup: '
   fun {DoLookupFile F SearchList}
      case SearchList == nil then
	 {ProfilerError LS # F # ' NOT FOUND!'} % should not happen!
	 nil
      else Try = SearchList.1 # F in
	 try
	    {OS.stat Try _}
	    {ProfilerMessage LS # F # ' actually is ' # Try}
	    {VS2A Try}
	 catch system(...) then
	    {ProfilerMessage LS # F # ' is not ' # Try}
	    {DoLookupFile F SearchList.2}
	 end
      end
   end
in
   fun {LookupFile F}
      S   = {Atom.toString F}
      Abs = case S
	    of     &/|_   then true  % absolute path (Unix...
	    elseof _|&:|_ then true  %               ...Windows)
	    else false end
   in
      case Abs then
	 try                         % absolute path
	    {OS.stat F _}
	    {ProfilerMessage LS # F # ' found'}
	    F
	 catch system(...) then
	    {ProfilerError LS # F # ' NOT FOUND!'} % should not happen!
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
   case {UnknownFile File} then
      '???'
   else
      S = {Str.rchr {Atom.toString File} &/}
   in
      case {List.length S} > 1 then
	 {S2A S.2}
      else
	 '???'
      end
   end
end

