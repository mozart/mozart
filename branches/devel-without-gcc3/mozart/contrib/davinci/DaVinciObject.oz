%%%
%%% Author:
%%%   Tobias Mueller <tmueller@ps.uni-sb.de>
%%%
%%% Contributor:
%%%   Leif Kornstaedt
%%%
%%% Copyright:
%%%   Tobias Mueller, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor

export
   DaVinciClass

import
   Error
   Open
   DaVinciScanner

\ifdef DEBUG
   Browser(browse: Browse)
\endif
   
define

   fun {CommandListToVS L}
      case L
      of H|nil then {CommandValueToVS H}
      [] H1|H2|T then  {CommandValueToVS H1}#","#{CommandListToVS H2|T}
      else
	 {Exception.raiseError daVinci(unexpectedCommand L)} unit
      end
   end

   fun {CommandValueToVS C}
      if     {IsInt C} then "\""#C#"\""
      elseif {IsFloat C} then "\""#C#"\""
      elseif C == nil then "[]"
      elseif {IsString C} then "\""#{StringToAtom C}#"\""
      elseif {IsList C} then '['#{CommandListToVS C}#']'
      elseif {IsAtom C} then "\""#C#"\""
      elseif {IsTuple C} then {Label C}#'('#{CommandListToVS {Record.toList C}}#')'
      else
	 {Exception.raiseError daVinci(unexpectedCommand C)} unit
      end
   end

   class DaVinciClass
      attr
	 pipe
	 in_stream
	 in_stream_tail
	 in_pipe_scanner: {New DaVinciScanner.inPipeScannerClass init()}

      meth init(S)
	 pipe <- {New Open.pipe
		  init(cmd:  'daVinci' args: ['-pipe'])}
	 in_stream <- S
	 in_stream_tail <- @in_stream
	 thread
	    {@in_pipe_scanner scanVirtualString({@pipe read(list: $)})}
	    DaVinciClass,listen_in_pipe
	 end
      end

      meth read_pipe(Mesg)
	 local T V M in
	    {@in_pipe_scanner getToken(T V)}
	    M = case T of 'EOF'
		then V1 in
		   {@in_pipe_scanner scanVirtualString({@pipe read(list: $)})}
		   {@in_pipe_scanner getToken(_ V1)}
		   V1
		else
		   V
		end
	    Mesg = {DaVinciScanner.daVinciAnswerToValue M}
	 end
      end

      meth listen_in_pipe
	 local Tail Msg in
	    try
	       Msg = DaVinciClass,read_pipe($)
	    catch E=error(daVinci(...) ...) then
	       Msg = E
	    end
	    @in_stream_tail = Msg | Tail
	    in_stream_tail <- Tail

	    if Msg == 'quit' then Tail = nil
	    else DaVinciClass,listen_in_pipe
	    end
	 end
      end

      meth to_pipe(VS)
\ifdef DEBUG
	 {Browse VS}
\endif
	 {@pipe write(vs: VS)}
      end
      
      meth close
	 DaVinciClass,to_pipe("menu(file(exit))\n")
      end

      meth send(C)
	 VS = {CommandValueToVS C}
      in
	 DaVinciClass,to_pipe(VS#"\n")
      end

      meth sendVS(VS)
	 DaVinciClass,to_pipe(VS#"\n")
      end

      meth graph(VS)
	 DaVinciClass,to_pipe("graph(new("#VS#"))\n")
      end
   end

   {Error.registerFormatter daVinci
    fun {$ E}
       T = 'DaVinci error'
    in
       case E of daVinci(unexpectedCommand C) then
	  error(kind: T
		msg: 'unexpected command'
		items: [hint(l: 'Found' m: oz(C))])
       [] daVinci(unexpectedResponse) then
	  error(kind: T
		msg: 'unexpected response')
       [] daVinci(parseError Msg VS Col) then
	  error(kind: 'DaVinci parse error'
		msg: Msg
		items: [hint(l: 'Output' m: VS)
			hint(l: 'Column' m: Col)])
       [] daVinci(programError Msg) then
	  error(kind: T
		msg: Msg)
       [] daVinci(parserError Msg) then
	  error(kind: T
		msg: Msg)
       else
	  error(kind: T
		items: [line(oz(E))])
       end
    end}

end
