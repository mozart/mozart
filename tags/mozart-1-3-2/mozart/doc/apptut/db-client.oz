functor
import
   Application Connection System Pickle
define
   Args = {Application.getCmdArgs
	   record(
	      url(single type:string optional:false)
	      get(single type:atom)
	      put(single type:atom))}
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
