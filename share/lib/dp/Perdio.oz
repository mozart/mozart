functor
import
   Glue at 'x-oz://boot/Glue'
   Event(put)
   Timer(alarm:Alarm)
export
   handler : PerdioHandler
define
   %% serves only to request this functor
   proc {PerdioHandler _} skip end

   proc {StartTask Label HowLong Priority Proc}
      Sync = {NewCell _}
      proc {TaskHandler _}
	 {Exchange Sync unit _}
      end
      {Event.put Label TaskHandler}
   in
      thread
	 This = {Thread.this}
	 {Thread.setPriority This Priority}
	 proc {Task}
	    Again = {Access Sync}
	 in
	    if {Proc} then {WaitOr Again {Alarm HowLong}}
	    else {Wait Again} end
	    {Task}
	 end
      in
	 {Task}
      end
   end

   {StartTask 'dp.probe'       3000 low Glue.'task.probe'}
   {StartTask 'dp.tmpDown'    60000 low Glue.'task.tmpDown'}
   {StartTask 'dp.myDown'       500 low Glue.'task.myDown'}
   {StartTask 'dp.flowControl' 1000 low Glue.'task.flowControl'}

end
