
functor

import
   Open

export
   Server
   StartServer
   StartServerOnPort
   StopServer
   
define

   TextSocket=class $ from Open.socket Open.text end
   
   class Client from TextSocket
      feat RThId WThId In Quit
      meth getIOStreams(Input Output)
	 thread
	    self.RThId={Thread.this}
	    self.In={NewPort Input}
	    proc{ReadLoop Remain}
	       if {IsFree self.Quit} then
		  M={List.append Remain {self receive($)}}
		  fun{Apply M}
		     L R
		     {List.takeDropWhile M fun{$ C} C\=&, end L R}
		  in
		     if L==M then
			M % , not yet received enough
		     else
			N={String.toInt L}
		     in
			if {Length R}=<N then
			   M % message not complete yet
			else
			   Msg Re
			in
			   {List.takeDrop {List.drop R 1} N Msg Re}
			   {Send self.In Msg}
			   if {List.take Msg 3}=="bye" then
			      self.Quit=unit
			   end
			   {Apply Re}
			end
		     end
		  end
	       in
		  {ReadLoop {Apply M}}
	       end
	    end
	 in
	    {ReadLoop ""}
	 end
	 thread
	    self.WThId={Thread.this}
	    proc{WriteLoop X}
	       case X of M|Ms then
		  {self send(M)}
		  {WriteLoop Ms}
	       end
	    end
	 in
	    {WriteLoop {NewPort $ Output}}
	 end
      end
      meth send(M)
	 try
	    {self write(vs:M#"\r\n" len:_)}
	 catch _ then
	    {self close}
	 end
      end
      meth receive(M)
	 I in
	 try
	    {self read(list:M len:I)}
	 catch _ then I=0 end
	 if I==0 then {self close} end
      end
      meth close
	 {Send self.In "bye"}
	 try
	    {Thread.terminate self.WThId}
	 catch _ then skip end
	 try
	    {self shutDown(how: [receive send])}
	 catch _ then skip end
	 try
	    Open.socket,close
	 catch _ then skip end
	 try
	    {Thread.terminate self.RThId}
	 catch _ then skip end
      end
   end

   class Server
      feat Port ThId Socket
      meth init(port:P)
	 self.Socket={New TextSocket init(type:stream protocol:"tcp")}
	 if {IsFree P} then {self.Socket bind(port:P)} else {self.Socket bind(takePort:P)} end
	 {self.Socket listen(backLog:5)}
	 self.Port=P
      end
      meth accept(acceptClass:AcceptClass<=Open.socket host:H<=_ port:P<=_ accepted:Obj)
	 {self.Socket accept(acceptClass:AcceptClass host:H port:P accepted:Obj)}
      end
      meth start(onconnect:P)
	 thread
	    proc{Loop}
	       Obj
	    in
	       {self accept(acceptClass:Client accepted:Obj)}
	       {P Obj}
	       {Loop}
	    end
	 in
	    self.ThId={Thread.this}
	    {Loop}
	 end
	 {Wait self.ThId}
      end
      meth close
	 {Thread.terminate self.ThId}
	 {self.Socket shutDown(how: [receive send])}
	 {self.Socket close}
      end
   end

   ServerDict={NewDictionary}
   
   fun{StartServer ConnectProcedure}
      Port
      S={New Server init(port:Port)}
   in
      {Dictionary.put ServerDict Port S}
      {S start(onconnect:ConnectProcedure)}
      Port
   end
   
   proc{StartServerOnPort Port ConnectProcedure}
      S={New Server init(port:Port)}
   in
      {Dictionary.put ServerDict Port S}
      {S start(onconnect:ConnectProcedure)}
   end

   proc{StopServer Port}
      try
	 {{Dictionary.condGet ServerDict Port r} close}
	 {Dictionary.remove ServerDict Port}
      catch _ then skip end
   end
   
end
