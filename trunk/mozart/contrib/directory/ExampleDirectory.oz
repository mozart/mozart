functor
import
   RecordC('^' tell)
   Discovery
   Connection(offerUnlimited take)
export
   Server
   ServiceProvider
   Client
define
   local
      fun {PrepareConstraint RIn} ROut in
	 {RecordC.tell {Record.label RIn} ROut}
	 {List.forAll {Record.arity RIn}
	  proc {$ F} X in
	     if {Record.is RIn.F} then
		X = {PrepareConstraint RIn.F}
	     else
		X = RIn.F
	     end
	     {RecordC.'^' ROut F X}
	  end}
	 ROut
      end
   in      
      fun{SubRecord R1 R2}
	 try
	    R2 = {PrepareConstraint R1}
	    true
	 catch failure(debug:d(info:[eq(...)] ...)) then
	    false
	 end
      end
   end
   class Store
      attr
	 directory
	 
      meth init()
	 directory <- nil
      end
      meth lookup(Pattern Tickets)
	 proc {FindMatching Xs Ys}
	    case Xs
	    of D#T#_|Xr then
	       if {SubRecord Pattern D} then Yr in
		  Ys = D#T|Yr
		  {FindMatching Xr Yr}
	       else
		  {FindMatching Xr Ys}
	       end
	    [] nil then
	       Ys = nil
	    end
	 end
      in
	 Tickets = {FindMatching @directory}
      end
      meth add(ServiceDescription Ticket ?Key)
	 Key = {Name.new}
	 directory <- ServiceDescription#Ticket#Key|@directory
      end
      meth remove(Key)
	 directory <- {List.filter @directory fun {$ K} K.3 \= Key end}
      end
   end

   class Server
      feat
	 server
	 store
	 
      meth init(port:PortNr <= useDefault expectedClientId:ClientId <= unit
		id:ServerId <= directory(kind:recordMatching))
	 proc {Serv Xs}
	    case Xs
	    of X|Xr then
	       case X 
	       of identity(ID) then
		  ID = ServerId
	       [] lookup(ClId Pattern ?Answer) then
		  if ClientId == unit orelse {SubRecord ClientId ClId} then
		     Answer = {self.store lookup(Pattern $)}
		  end
	       [] add(ClId ServiceDesc Ticket ?Key) then
		  if ClientId == unit orelse {SubRecord ClientId ClId} then
		     {self.store add(ServiceDesc Ticket ?Key)}
		  end
	       [] remove(ClId Key) then
		  if ClientId == unit orelse {SubRecord ClientId ClId} then
		     {self.store remove(Key)}
		  end
	       else
		  skip
	       end
	       {Serv Xr}
	    end
	 end
	 Stream P = {Port.new Stream} Ticket
      in
	 self.store = {New Store init()}

	 {Connection.offerUnlimited P ?Ticket} 
	 thread {Serv Stream} end

	 if PortNr == useDefault then
	    self.server = {New Discovery.server init(info:Ticket)}
	 else
	    self.server = {New Discovery.server init(info:Ticket port:PortNr)}
	 end
      end
      meth close()
	 {self.server close()}
      end
   end

   fun {Filter Xs ServerId TimeOut}
      Ys Alarm Fine NrLeft = {Cell.new {List.length Xs}} L = {Lock.new}
      fun {Check X} T P P1 in
	 thread
	    {Thread.this T}
	    try
	       P1 = {Connection.take X}
	       if {Port.is P1} then Answer in
		  {Port.send P1 identity(Answer)}
		  if {SubRecord ServerId Answer} then
		     P = P1
		  else
		     P = unit
		  end
		  lock L then Old New in
		     {Cell.exchange NrLeft Old New}
		     New = Old - 1
		  end
	       end
	    catch error(connection(illegalTicket ...) ...) then
	       P = unit
	    end
	 end
	 P#T
      end
      proc {Finished} Left in
	 lock L then
	    Left = {Cell.access NrLeft}
	 end
	 if Left > 0 then
	    {Delay 1}
	    {Finished}
	 end
      end	    
      fun {InnerFilter Ys}
	 case Ys
	 of P#T|Yr then
	    if {Value.isFree P} then
	       {Thread.terminate T}
	       {InnerFilter Yr}
	    elseif P == unit then
	       {InnerFilter Yr}
	    else
	       P|{InnerFilter Yr}
	    end
	 [] nil then
	    nil
	 end
      end
   in
      thread {Delay TimeOut} Alarm = unit end
      thread {Finished} Fine = unit end
      Ys = {List.map Xs Check}
      {Value.waitOr Alarm Fine}
      {InnerFilter Ys}
   end

   class DirectoryClient
      feat ports id
      meth init(serverPort:PortNr <= useDefault
		id:ClientId <= unit
		expectedServerId:ServerId <= directory(kind:recordMatching)
		timeOut:TimeOut <= 1000)
	 fun {NoDuplicates Xs}
	    case Xs
	    of X|Xr then
	       if {List.member X Xr} then
		  {NoDuplicates Xr}
	       else
		  X|{NoDuplicates Xr}
	       end
	    else
	       Xs
	    end
	 end
	 TimeUnit = TimeOut div 4
	 Found X C 
      in
	 self.id = ClientId
	 if PortNr == useDefault then
	    C = {New Discovery.client init()}
	 else
	    C = {New Discovery.client init(port:PortNr)}
	 end
	 {C getAll(timeOut:3*TimeUnit info:Found)}
	 X = {NoDuplicates {Filter Found ServerId TimeUnit}}
	 {C close()}
	 case X
	 of P|Pr then
	    self.ports = P|Pr
	 else
	    raise
	       error(directory('Error in directory service. No server found'))
	    end  
	 end
      end
   end
   class ServiceProvider from DirectoryClient
      feat
	 removeKeys
	 
      meth init(...) = M
	 self.removeKeys = {Dictionary.new}
	 DirectoryClient, M
      end
      %% Returns keyword for removing the service
      meth add(description:Desc ticket:Ticket key:?Key <= _) Ks in
	 Ks = {List.map self.ports fun {$ P}
				      {Port.send P add(self.id Desc Ticket $)}
				   end}
	 Key = {Name.new}
	 {Dictionary.put self.removeKeys Key Ks}
      end
      meth remove(key:Key) Ks in
	 Ks = {Dictionary.condGet self.removeKeys Key unit}
	 if Ks \= unit then
	    {Dictionary.remove self.removeKeys Key}
	    {List.forAllInd self.ports
	     proc {$ I P}
		{Port.send P remove(self.id {List.nth Ks I})}
	     end}
	 end
      end
   end
   fun {Append Ls}
      case Ls
      of L1|L2|Lr then
	 {Append {List.append L1 L2}|Lr}
      [] [L1] then
	 L1
      [] nil then
	 nil
      end
   end
   class Client from DirectoryClient
      meth lookup(pattern:Pattern services:?Services) Ss in
	 Ss = {List.map self.ports
	       fun {$ P}
		  {Port.send P lookup(self.id Pattern $)}
	       end}
	 Services = {Append Ss} 
      end
   end
end
