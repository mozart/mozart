functor
import
   Server at 'server.ozf'
   Application
define
   class Registry
      feat db
      meth init {Dictionary.new self.db} end
      meth put(Key Val) {Dictionary.put self.db Key Val} end
      meth get(Key Val) {Dictionary.get self.db Key Val} end
      meth condGet(Key Default Val)
	 {Dictionary.condGet self.db Key Default Val}
      end
   end
   DB = {New Registry init}
   Args = {Application.getCmdArgs
	   record(
	      ticketfile(single char:&t type:string optional:false))}
   {Server.start DB Args.ticketfile}
end
