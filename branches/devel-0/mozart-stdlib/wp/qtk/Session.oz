functor

import
   Fault
%  System(show:Show)
%  Browser(browse:Browse)

export
   new:NewSession
   Connect
   Disconnect
   Break
   GetStream
   GetStateStream
   GetSideStream
   GetState
   GetRef
   GetPeer
   SSend
   ASend
   LSend
   SideSend
   Bind
   Configure
   WaitDisconnectOrs
   WaitConnectOrs
   WaitOkOrs
   WaitNotOkOrs
   WaitDisconnectOr
   WaitConnectOr
   WaitOkOr
   WaitNotOkOr
   WaitDisconnect
   WaitConnect
   WaitOk
   WaitNotOk
   IsConnected

prepare
   Session={NewName}
   Ref={NewName}
   
define

   proc{Show _} skip end

   class SessionClass

      prop
	 locking sited

      feat
	 id
	 port
	 map
	 ref
	 rref

      attr
	 stream
	 statestream
	 sidestream
	 other
	 tempproc
	 sendth
	 events
	 config
	 mdth
	 bufhead
	 buftail

      meth init
	 proc{ApplyMsg Id P}
	    lock {Show l1#(@other==Id)}
	       if @other==Id then
		  {P}
	       else
		  try
		     {Port.send {WeakDictionary.get self.map Id} brk(self.id)}
		  catch _ then skip end
	       end
	    end
	 end
      in
	 self.id={NewName}
	 self.map={NewDictionary}
	 self.ref={Chunk.new r(Ref:self.id#self.port
			       Session:self)}
	 self.rref={Chunk.new r(Ref:self.id#self.port)}
	 other<-unit
	 tempproc<-unit
	 sendth<-unit
	 mdth<-unit
	 statestream<-disconnect|_
	 bufhead<-@buftail
	 events<-r(connect:proc{$} skip end
		   disconnect:proc{$} skip end
		   tempFail:proc{$} skip end
		   permFail:proc{$} skip end
		   ok:proc{$} skip end)
	 config<-r(autoBreakAfter:inf)
	 thread
	    {ForAll {NewPort $ self.port}
	     proc{$ M}
		{Show rec#M}
		case M
		of con(Tgt) then
		   lock {Show l2}
		      if @other==unit then
			 {self Connect(Tgt)}
			 {self Raise(connect)}
		      else
			 thread
			    try
			       {Port.send Tgt.2 brk(self.id)}
			    catch _ then skip end
			 end
		      end
		   end
		[] brk(Id) then
		   {ApplyMsg Id
		    proc{$}
		       {self Disconnect(disconnect)}
		    end}
		[] ping(Id) then
		   {ApplyMsg Id
		    proc{$}
		       thread
			  {Port.send @other pong(self.id)}
		       end
		    end}
		[] pong(Id) then
		   {ApplyMsg Id proc{$} skip end}
		[] ign then skip
		[] snc(Ack) then
		   _={Fault.enable Ack site nil}
		   thread
		      Ack=unit
		   end
		[] as(Id Msg) then
		   {ApplyMsg Id
		    proc{$}
		       N
		    in
		       @stream=Msg|N
		       stream<-N
		    end}
		[] ss(Id Msg Ack) then
		   {ApplyMsg Id
		    proc{$}
		       N
		    in
		       @stream=Msg|N
		       stream<-N
		    end}
		   _={Fault.enable Ack site nil}
		   thread
		      Ack=unit
		   end
		[] ds(Msg) then
		   N
		in
		   @sidestream=Msg|N
		   sidestream<-N
		end
	     end}
	 end
	 {Wait self.port}
      end
      
      meth SetTempProc
	 Id=@other
	 Pt={Dictionary.get self.map Id}
      in
	 tempproc<-proc{$ E FS}
		      lock {Show l3}
			 if @other==Id then %% this should always be the case...
			    {self Raise(tempFail)}
			    tempproc<-unit
			    thread
			       {Port.send Pt ign} %% this line blocks until the tempFail is resolved
			       lock {Show l4}
				  if @other==Id andthen @tempproc==unit then
				     {self Raise(ok)}
				     {self SetTempProc}
				  end
			       end
			    end
			 end
		      end
		   end
	 _={Fault.installWatcher Pt [tempFail] @tempproc}
      end
      
      meth Connect(To)
	 lock {Show l5}
	    if @other\=unit then
	       raise alreadyConnected end
	    else
	       %% pre : @bufhead==@buftail (no pending messages)
	       %% @tempproc==unit (no tempFail procedure)
	       %% @sendth==unit (no send thread defined)

	       %% Step 1 : gets info

	       Id#Pt=To

	       proc{PermFailProc E FS}
		  lock {Show l6}
		     if @other==Id then
			{self Disconnect(permFail)}
		     end
		     {Dictionary.remove self.map Id}
		  end
	       end

	       Sync
	       
	    in
	       
	       %% Step 2 : disable exceptions and place fault handler for perm fault

	       _={Fault.enable Pt site nil}

	       if {Not {Dictionary.member self.map Id}} then
		  {Dictionary.put self.map Id Pt}
		  _={Fault.installWatcher Pt [permFail]
		     PermFailProc}
	       end

	       %% Step 3 : start the send thread

	       sendth<-Sync
	       thread
		  proc{Loop}
		     M=@bufhead
		     {WaitOr M.1 Sync}
		  in
		     if {IsFree Sync} then
			lock {Show l7}
			   case M.1
			   of snd(Msg) then
			      {Port.send Pt Msg}
			   [] dsc then
			      {Port.send Pt brk(self.id)}
			      {self Disconnect(disconnect)}
			   end
			   bufhead<-M.2
			end
			{Loop}
		     end
		  end
	       in
		  {Loop}
	       end

	       other<-Id
	       {self SetTempProc}
	       
	    end
	 end
      end

      meth Disconnect(Msg)
	 lock {Show l8}
	    %% pre : whatever
	    %% post : pre of Connect
	    bufhead<-@buftail
	    try
	       _={Fault.deInstallWatcher {Dictionary.get self.map @other} @tempproc}
	    catch _ then skip end
	    other<-unit
	    tempproc<-unit
	    @sendth=unit
	    {self Raise(Msg)}
	 end
      end

      meth Send(M)
	 lock {Show l9}
	    if @other\=unit then
	       N
	    in
	       @buftail=M|N
	       buftail<-N
	    end
	 end
      end

      meth Raise(Evt)
	 lock {Show l10}
	    R=Evt|_
	 in
	    {Show l10a}
	    @statestream=_|R
	    statestream<-R
	    try {Thread.terminate @mdth} catch _ then skip end
	    {Show l10b}
	    {Show Evt}
	    if Evt==tempFail then
	       {Show l10c}
	       Dly=@config.autoBreakAfter
	    in
	       {Show l10d}
	       if Dly\=inf then
		  mdth<-_
		  thread
		     @mdth={Thread.this}
		     {Delay Dly}
		     {self break}
		  end
		  {Show l10e}
		  {Wait @mdth}
		  {Show l10f}
	       end
	    end
	    {Show l10g}
	 end
	 {Show l10h}
	 {Show @events}
	 thread
	    {@events.Evt}
	 end
	 {Show l10i}
      end

      meth connect(To)
	 {Show cc0}
	 lock {Show l11}
	    {Show cc1}
	    {self Connect(To.Ref)}
	    {Show cc2}
	    {self Send(snd(con(self.id#self.port)))}
	    {Show cc3}
	    {self Raise(connect)}
	    {Show cc4}
	 end
      end

      meth disconnect
	 lock {Show l12}
	    if @other\=unit then
	       {self Send(dsc)}
	    else
	       raise notConnected end
	    end
	 end
      end

      meth break
	 lock {Show l13}
	    if @other\=unit then
	       O={Dictionary.get self.map @other}
	    in
	       {self Disconnect(disconnect)}
	       thread
		  {Port.send O brk(self.id)}
	       end
	    end
	 end
      end

      meth asend(Msg)
	 lock {Show l14}
	    if @other==unit then
	       raise notConnected end
	    else
	       {self Send(snd(as(self.id Msg)))}
	    end
	 end
      end

      meth waitOrDisco(A)
	 SS
      in
	 lock {Show l15}
	    SS=@statestream
	 end
	 thread
	    proc{Loop L}
	       {WaitOr A L}
	       if {IsFree A} then
		  case L
		  of disconnect|_ then skip
		  [] permFail|_ then skip
		  [] _|Xs then {Loop Xs}
		  end
	       end
	    end
	 in
	    {Loop SS}
	 end
      end

      meth ssend(Msg)
	 A
      in
	 lock {Show l16}
	    if @other==unit then
	       raise notConnected end
	    else
	       {self Send(snd(ss(self.id Msg A)))}
	    end
	 end
	 {self waitOrDisco(A)}
      end

      meth lsend(Msg)
	 %% sends to the local stream
	 lock {Show l17}
	    N
	 in
	    @stream=Msg|N
	    stream<-N
	 end
      end

      meth sideSend(To Msg)
	 {Port.send To.Ref.2 ds(self.ref#Msg)}
      end
      
      meth getStream($)
	 @stream
      end

      meth getStateStream($)
	 @statestream
      end

      meth getSideStream($)
	 @sidestream
      end

      meth getPeer($)
	 @other
      end

      meth bind(Evt P)
	 lock {Show l18}
	    if {HasFeature @events Evt} then
	       events<-{Record.adjoinAt @events Evt P}
	    else
	       raise unknownEvent end
	    end
	 end
      end

      meth config(P V)
	 lock {Show l19}
	    if {HasFeature @config P} then
	       config<-{Record.adjoinAt @config P V}
	    else
	       raise unknownParameter end
	    end
	 end
      end
	 
      
   end

   fun{NewSession}
      Obj={New SessionClass init}
   in
      Obj.ref
   end

   proc{Connect C To}
      {C.Session connect(To)}
   end

   proc{Disconnect C}
      {C.Session disconnect}
   end

   proc{Break C}
      {C.Session break}
   end

   fun{GetStream C}
      {C.Session getStream($)}
   end

   fun{GetStateStream C}
      {C.Session getStateStream($)}
   end

   fun{GetSideStream C}
      {C.Session getSideStream($)}
   end

   fun{GetState C}
      {GetStateStream C}.1
   end

   fun{GetRef C}
      C.Session.rref
   end

   fun{GetPeer C}
      {C.Session getPeer($)}
   end

   proc{ASend C M}
      {C.Session asend(M)}
   end

   proc{SSend C M}
      {C.Session ssend(M)}
   end

   proc{LSend C M}
      {C.Session lsend(M)}
   end

   proc{SideSend C To M}
      {C.Session sideSend(To M)}
   end

   proc{Bind C Evt P}
      {C.Session bind(Evt P)}
   end

   proc{Configure C Param V}
      {C.Session config(Param V)}
   end

   proc{WaitEvent C L V}
      SS={GetStateStream C}
      G
      {ForAll V
       proc{$ E}
	  thread
	     {WaitOr G E} G=unit
	  end
       end}
      proc{Loop D}
	 {WaitOr D G}
	 if {IsFree G} then
	    case D of X|Xs then
	       if {IsDet Xs} orelse
		  {Not {List.member X L}} then {Loop Xs} end
	    end
	 end
      end
   in
      {Loop SS}
   end
   
   proc{WaitDisconnectOrs C V}
      {WaitEvent C [permFail disconnect] V}
   end

   proc{WaitConnectOrs C V}
      {WaitEvent C [connect ok tempFail] V}
   end

   proc{WaitOkOrs C V}
      {WaitEvent C [connect ok] V}
   end
   
   proc{WaitNotOkOrs C V}
      {WaitEvent C [tempFail permFail disconnect] V}
   end

   proc{WaitDisconnectOr C V}
      {WaitEvent C [permFail disconnect] [V]}
   end

   proc{WaitConnectOr C V}
      {WaitEvent C [connect ok tempFail] [V]}
   end

   proc{WaitOkOr C V}
      {WaitEvent C [connect ok] [V]}
   end
   
   proc{WaitNotOkOr C V}
      {WaitEvent C [tempFail permFail disconnect] [V]}
   end

   proc{WaitDisconnect C}
      {WaitEvent C [permFail disconnect] nil}
   end

   proc{WaitConnect C}
      {WaitEvent C [connect ok tempFail] nil}
   end

   proc{WaitOk C}
      {WaitEvent C [connect ok] nil}
   end
   
   proc{WaitNotOk C}
      {WaitEvent C [tempFail permFail disconnect] nil}
   end

   fun{IsConnected C}
      {List.member {GetStateStream C}.1 [ok connect tempFail]}
   end

end