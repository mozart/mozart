functor
import Application Open Pickle Connection
define
   Args = {Application.getCmdArgs
	   record(
	      url( single type:string optional:false)
	      'in'(single type:string optional:false)
	      out( single type:string optional:false))}
   File = {New Open.file init(name:Args.'in')}
   Text = {File read(list:$ size:all)}
   {File close}
   OZC  = {Connection.take {Pickle.load Args.url}}
   case {OZC compile({ByteString.make Text} $)}
   of yes(F) then
      {Pickle.save F Args.out}
      {Application.exit 0}
   elseof no(Msgs) then raise ozc(Msgs) end end
end
