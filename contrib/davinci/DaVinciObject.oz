functor

export
   DaVinciClass

import
   Open
   System
   DaVinciScanner

define

   fun {CommandListToVS L}
      case L
      of H|nil then {CommandValueToVS H}
      [] H1|H2|T then  {CommandValueToVS H1}#","#{CommandListToVS H2|T}
      else {System.showInfo 'Unexpected command in CommandListToVS.'} error
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
      else {System.showInfo 'Unexpected command in CommandValueToVS.'} error
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
         local Tail Msg = DaVinciClass,read_pipe($)
         in
            @in_stream_tail = Msg | Tail
            in_stream_tail <- Tail

            if Msg == 'quit' then Tail = nil
            else DaVinciClass,listen_in_pipe
            end
         end
      end

      meth close
         {@pipe write(vs: "menu(file(exit))\n")}
      end

      meth send(C)
         VS = {CommandValueToVS C}
      in
         {@pipe write(vs: VS#"\n")}
      end

      meth graph(VS)
         {@pipe write(vs: "graph(new("#VS#"))\n")}
      end
   end

end
