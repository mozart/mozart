%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

\insert string
\insert tk

%% some builtins...
Dbg = dbg( taskstack   : {`Builtin` taskstack 3}
	   stream      : {`Builtin` globalThreadStream 1}
	   stepmode    : {`Builtin` setStepMode 2}
	   trace       : {`Builtin` traceThread 2}
	   state       : {`Builtin` queryDebugState 2}
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

fun {StripPath File}
   {Str.rchr {Atom.toString File} &/}.2
end

% transform an argument list
fun {FormatArgs A}
   {List.mapInd A
    fun {$ N X}
       N # {ArgType X} # X
    end}
end

fun {ArgType X}
   case {IsDet X} then
      case     {IsArray X}      then '<array>'
      elsecase {IsAtom X}       then case X
				     of 'nil'         then "'nil'"
				     [] '|'           then "'|'"
				     [] '#'           then "'#'"
				     [] 'unallocated' then 'unalloc'
				     else                  '\'' # X # '\''
				     end
      elsecase {IsBool X}       then case X of true then 'true' else 'false'
				     end
      elsecase {IsCell X}       then '<cell>'
      elsecase {IsClass X}      then '<class>'
      elsecase {IsDictionary X} then '<dict>'
      elsecase {IsFloat X}      then '<float>'
      elsecase {IsInt X}        then X
      elsecase {IsList X}       then '<list>'
      elsecase {IsUnit  X}      then 'unit'
      elsecase {IsName X}       then '<name>'
      elsecase {IsLock X}       then '<lock>'
      elsecase {IsObject X}     then '<object>'
      elsecase {IsPort X}       then '<port>'
      elsecase {IsProcedure X}  then '<proc>'
      elsecase {IsTuple X}      then '<tuple>'
      elsecase {IsRecord X}     then '<record>'
      elsecase {IsChunk X}      then '<chunk>'
      else                           '<???>'
      end
   else                              '_'
   end
end

TagCounter =
{New class 
	attr n
	meth clear n<-0 end
	meth get($) N=@n in n<-N+1 N end
     end clear}
