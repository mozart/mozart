functor
import Connection Pickle
export Start
define
   proc {Start Proc File}
      Requests P = {NewPort Requests} Ticket
      proc {Proxy Msg}
	 if {Port.send P request(Msg $)} then skip
	 else raise remoteError end end
      end
   in
      {New Connection.gate init(Proxy Ticket) _}
      {Pickle.save Ticket File}
      {ForAll Requests
       proc {$ R}
	  case R of request(Msg OK) then
	     try {Proc Msg} OK=true catch _ then
		try OK=false catch _ then skip end
	     end
	  else skip end
       end}
   end
end
