functor
export
   Tails Count
require
   BootValue(
      newReadOnly : NewReadOnly
      bindReadOnly: BindReadOnly)
   at 'x-oz://boot/Value'
prepare
   proc {Tails L R}
      {NewReadOnly R}
      thread {TailsLoop L R}end
   end
   proc {TailsLoop L R}
      {WaitNeeded R}
      case L
      of nil  then {BindReadOnly R nil}
      [] _|L2 then R2={NewReadOnly} in
	 {BindReadOnly R L|R2}
	 {TailsLoop L2 R2}
      end
   end

   proc {Count From To By R}
      R={NewReadOnly}
   in
      thread
	 {if By<0 then CountDown else CountUp end
	  From To By R}
      end
   end
   proc {CountDown From To By R}
      {WaitNeeded R}
      if From<To then {BindReadOnly R nil}
      else R2={NewReadOnly} in
	 {BindReadOnly R From|R2}
	 {CountDown From+By To By R2}
      end
   end
   proc {CountUp From To By R}
      {WaitNeeded R}
      if From>To then {BindReadOnly R nil}
      else R2={NewReadOnly} in
	 {BindReadOnly R From|R2}
	 {CountUp From+By To By R2}
      end
   end
end
