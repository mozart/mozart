functor
import
   Compiler Application
   Server at 'server.ozf'
define
   class OZC
      prop locking
      feat engine interface
      meth init
	 self.engine    = {New Compiler.engine    init}
	 self.interface = {New Compiler.interface init(self.engine)}
	 {self.engine enqueue(setSwitch(expression true))}
      end
      meth compile(VS $)
	 lock F in
	    {self.engine
	     enqueue(feedVirtualString(VS return(result:F)))}
	    {Wait {self.engine enqueue(ping($))}}
	    if {self.interface hasErrors($)} then
	       no({self.interface getMessages})
	    else yes(F) end
	 end
      end
   end
   Service = {New OZC init}
   Args = {Application.getCmdArgs
	   record(
	      ticketfile(single char:&t type:string optional:false))}
   {Server.start Service Args.ticketfile}
end
