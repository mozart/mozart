functor

import
   Fault

export
   NewBidiPort

prepare
   CTL={NewName}
   RetryDelay=1000
   Init={NewName}
   
define
   
   {Fault.defaultDisable _}

   fun {FOneOf F AFS}
      case AFS of nil then false 
      [] AF2|AFS2 then 
	 case F#AF2
	 of permFail#permFail(...) then true 
	 [] tempFail#tempFail(...) then true 
	 [] remoteProblem(I)#remoteProblem(I ...) then true 
	 else {FOneOf F AFS2}
	 end 
      end 
   end

   class BidiPort

      prop locking
   
      attr
	 other
	 stream
	 events
	 timeout
	 buffer

      feat
	 port

      meth !Init
	 other<-_
	 events<-r(temp:proc{$} skip end
		   perm:proc{$} skip end
		   disco:proc{$} skip end)
	 timeout<-10 %% by default : retries 10 times before disconnecting
	 thread
	    S
	    self.port={NewPort S}
	 in
	    {ForAll S
	     proc{$ Id#M}
		lock
		   Other=@other
		in
		   if {IsDet Other} then
		      case Id
		      of !CTL#M then
			 case M
			 of connect(Id) then
			    {self Connect(Id)}
			 [] break then
			    buffer<-_
			    if {IsDet Other} then
			       other<-_
			    end
			    {self Raise(disco)}
			 [] !Other#M then
			    N
			 in
			    @stream=M|N
			    stream<-N
			 end
		      else
			 {self SendDontCare(Id CTL#break)}
		      end
		   else
		      {self SendDontCare(Id CTL#break)}
		   end
		end
	     end}
	 end
	 thread
	    proc{Retry V}
	       W
	    in
	       {Wait @buffer}
	       lock
		  M|_=@buffer
	       in
		  try
		     {Port.send P M}
		  catch system(dp(conditions:FS ...) ...) then
		     if {FOneOf permFail FS} then
			W=0
			{self disconnect}
			{self Raise(perm)}
		     elseif {FOneOf tempFail FS} then
			W=V+1
		     else skip end
		  end
		  if {IsFree W} then
		     %% send succeeded
		     proc{Loop}
			if {IsDet @buffer} then
			   Bad
			   P#M|N=@buffer
			in
			   try
			      {Port.send P M}
			   catch system(dp(conditions:FS ...) ...) then
			      if {FOneOf permFail FS} then
				 Bad=unit
				 {self Raise(perm)}
				 {self disconnect}
			      elseif {FOneOf tempFail FS} then
				 Bad=unit
				 {self Raise(temp)}
			      else skip end
			   end
			   if {IsFree Bad} then
			      buffer<-N
			      {Loop}
			   end
			end
		     end
		  in
		     W=0
		     {self Raise(ok)}
		     {Loop}
		  end
	       end
	       {Delay RetryDelay}
	       if W>@timeout then
		  {self disconnect}
		  {self Raise(disco)}
		  {Retry 0}
	       else
		  {Retry W}
	       end
	    end
	 in
	    {Retry 0}
	 end
	 {Wait self.port}
      end

      meth getPort($)
	 self.port
      end
      
      meth getStream($)
	 @stream
      end

      meth send(M)
	 Tgt=@other
      in
	 if Tgt==unit then skip
	 else
	    {self SendSafe(Tgt M)}
	 end
      end

      meth SendDontCare(P M)
	 try
	    {Port.send P M}
	 catch system(dp(...) ...) then skip end
      end

      meth SendSafe(P M)
	 %% will try to send the message M on port P
	 %% retrying if there is a temp fail
	 lock
	    if {IsFree @buffer} then
	       try
		  {Port.send P M}
	       catch system(dp(conditions:FS ...) ...) then
		  if {FOneOf permFail FS} then
		     {self Raise(perm)}
		     {self disconnect}
		  elseif {FOneOf tempFail FS} then
		     N
		  in
		     {self Raise(temp)}
		     @buffer=M|N
		  else skip end
	       end
	    else
	       N
	    in
	       @buffer=M|N
	       buffer<-N
	    end
	 end
      end

      meth Connect(P)
	 lock
	    {self disconnect}
	    @other=P
	 end
      end

      meth connectTo(P)
	 lock
	    {self Connect(P)}
	    {self SendSafe(P CTL#connect(self.port))}
	 end
      end
   
      meth onTemp(P)
	 {self Bind(temp P)}
      end

      meth onPerm(P)
	 {self Bind(perm P)}
      end

      meth onDisconnect(P)
	 {self Bind(disco P)}
      end

      meth Bind(Evt P)
	 lock
	    events<-{Record.adjoinAt @events Evt P}
	 end
      end
   
      meth disconnect
	 lock
	    buffer<-_
	    if {IsDet @other} then
	       {self SendDontCare(@other CTL#break)}
	       other<-_
	    end
	 end
      end

      meth break
	 %% closes the connection instantly, don't care about tempFail messages
	 %% that might or might not have been sent
	 skip
      end

      meth setTimeOut(T)
	 timeout<-T
      end

      meth Raise(Evt)
	 lock
	    {@events.Evt}
	 end
      end

      meth isConnected($)
	 @other\=unit
      end

      meth getConnection($)
	 @other
      end

      meth waitForConnection
	 {Wait @other}
      end
	 
   end

   fun{NewBidiPort}
      {New BidiPort Init}
   end

end