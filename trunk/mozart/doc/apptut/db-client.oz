functor
import
   Application Connection System Pickle
define
   Args = {Application.getCmdArgs
	   record(
	      url(single char:&u type:string optional:false)
	      get(single char:&g type:atom)
	      put(single char:&p type:atom))}
   DB   = {Connection.take {Pickle.load Args.url}}
   if     {HasFeature Args get} then
      {System.showInfo {DB get(Args.get $)}}
   elseif {HasFeature Args put} then
      case Args.1 of [Value] then
	 {DB put(Args.put Value)}
      else
	 {System.showError 'Missing value argument'}
	 {Application.exit 1}
      end
   else
      {System.showError 'One of --get or --put is required'}
      {Application.exit 1}
   end
   {Application.exit 0}
end
