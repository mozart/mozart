functor
import Application
define
   Args = {Application.getCmdArgs
	   record(threads(single type:int optional:false)
		  times(  single type:int optional:false))}
   proc {Yield}
      {Thread.preempt {Thread.this}}
   end
   proc {ProcessMessage Msg}
      if Msg==1 then raise done end else {Yield} end
   end
   proc {CreateThreads N AllDone}
      if N==0 then AllDone=unit else RestDone in
	 thread
	    try {ForAll Messages ProcessMessage}
	    catch done then AllDone=RestDone end
	 end
	 {CreateThreads N-1 RestDone}
      end
   end
   Messages Mailbox={Port.new Messages}
   AllDone = {CreateThreads Args.threads}
   {ForAll {List.number Args.times 1 ~1}
    proc {$ I} {Port.send Mailbox I} {Yield} end}
   {Wait AllDone}
   {Application.exit 0}
end
