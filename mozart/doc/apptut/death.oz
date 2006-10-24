functor
import Application
define
   proc {Yield} {Thread.preempt {Thread.this}} end
   proc {Run N}
      {Yield}
      if N>1 then {Run N-1} end
   end
   Args = {Application.getCmdArgs
	   record(threads(single type:int optional:false)
		  times(  single type:int optional:false))}
   proc {Main N AllDone}
      if N==0 then AllDone=unit else RestDone in
	 thread {Run Args.times} AllDone=RestDone end
	 {Main N-1 RestDone}
      end
   end
   {Wait {Main Args.threads}}
   {Application.exit 0}
end
