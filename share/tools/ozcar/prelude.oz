%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

\insert string
\insert tk

%% some builtins...

local
   
   TS  = {`Builtin` taskstack 3}
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

proc {OzcarMessage M}
   {System.showInfo OzcarMessagePrefix # M}
end

fun {VS2A X}
   {String.toAtom {VirtualString.toString X}}
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
      case     {IsUnit X}       then '<unit>'
      elsecase {IsArray X}      then '<array>'
      elsecase {IsAtom X}       then '<atom>'
      elsecase {IsBool X}       then '<bool>'
      elsecase {IsCell X}       then '<cell>'
      elsecase {IsClass X}      then '<class>'
      elsecase {IsDictionary X} then '<dictionary>'
      elsecase {IsFloat X}      then '<float>'
      elsecase {IsInt   X}      then '<int>'
      elsecase {IsList X}       then '<list>'
      elsecase {IsLiteral X}    then '<literal>'
      elsecase {IsLock X}       then '<lock>'
      elsecase {IsName X}       then '<name>'
      elsecase {IsObject X}     then '<object>'
      elsecase {IsPort X}       then '<port>'
      elsecase {IsProcedure X}  then '<procedure>'
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
