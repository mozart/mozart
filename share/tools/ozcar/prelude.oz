%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

\insert string
\insert tk

%% some builtins...

local
   
   TS  = {`Builtin` taskstack 2}
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

proc {Message M}
   Prefix = "Message from Ozcar: "
in
   {System.showInfo Prefix # M}
end

fun {VS2A X}
   {String.toAtom {VirtualString.toString X}}
end

% transform an argument list
fun {FormatArgs A}
   {List.mapInd A
    fun {$ N X}
       case {IsDet X} then
	  case     {IsUnit X}       then N # '<unit>'       # X
	  elsecase {IsArray X}      then N # '<array>'      # X
	  elsecase {IsAtom X}       then N # '<atom>'       # X
	  elsecase {IsBool X}       then N # '<bool>'       # X
	  elsecase {IsCell X}       then N # '<cell>'       # X
	  elsecase {IsClass X}      then N # '<class>'      # X
	  elsecase {IsDictionary X} then N # '<dictionary>' # X
	  elsecase {IsFloat X}      then N # '<float>'      # X
	  elsecase {IsInt   X}      then N # '<int>'        # X
	  elsecase {IsList X}       then N # '<list>'       # X
	  elsecase {IsLiteral X}    then N # '<literal>'    # X
	  elsecase {IsLock X}       then N # '<lock>'       # X
	  elsecase {IsName X}       then N # '<name>'       # X
	  elsecase {IsObject X}     then N # '<object>'     # X
	  elsecase {IsPort X}       then N # '<port>'       # X
	  elsecase {IsProcedure X}  then N # '<procedure>'  # X
	  elsecase {IsTuple X}      then N # '<tuple>'      # X
	  elsecase {IsRecord X}     then N # '<record>'     # X
	  elsecase {IsChunk X}      then N # '<chunk>'      # X
	  else                           N # '<???>'        # X
	  end
       else                              N # '_'            # X
       end
    end $}
end

TagCounter =
{New class 
	attr n
	meth clear n<-0 end
	meth get($) N=@n in n<-N+1 N end
     end clear}
