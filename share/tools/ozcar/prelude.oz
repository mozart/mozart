%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

\insert string
\insert tk

%% some builtins...
Dbg = dbg( taskstack   : {`Builtin` taskstack 3}
	   stream      : {`Builtin` globalThreadStream 1}
	   contflag    : {`Builtin` setContFlag 2}
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

% transform an argument list
fun {FormatArgs A}
   {List.mapInd A
    fun {$ N X}
       N # {ArgType X} # X
    end}
end

fun {ArgType X}
   case {IsDet X} then
      case     {IsArray X}      then ArrayType
      elsecase {IsThread X}     then ThreadType
      elsecase {IsAtom X}       then case X
				     of 'nil'         then NilAtom
				     [] '|'           then ConsAtom
				     [] '#'           then HashAtom
				     [] 'unallocated' then UnAllocatedType
				     else                  '\'' # X # '\''
				     end
      elsecase {IsBool X}       then case X then TrueName else FalseName end
      elsecase {IsCell X}       then CellType
      elsecase {IsClass X}      then ClassType
      elsecase {IsDictionary X} then DictionaryType
      elsecase {IsFloat X}      then FloatType
      elsecase {IsInt X}        then X
      elsecase {IsList X}       then ListType
      elsecase {IsUnit  X}      then UnitType
      elsecase {IsName X}       then NameType
      elsecase {IsLock X}       then LockType
      elsecase {IsObject X}     then ObjectType
      elsecase {IsPort X}       then PortType
      elsecase {IsProcedure X}  then ProcedureType
      elsecase {IsTuple X}      then TupleType
      elsecase {IsRecord X}     then RecordType
      elsecase {IsChunk X}      then ChunkType
      else                           UnknownType
      end
   else                              UnboundType
   end
end

TagCounter =
{New class 
	attr n
	meth clear n<-0 end
	meth get($) N=@n in n<-N+1 N end
     end clear}
