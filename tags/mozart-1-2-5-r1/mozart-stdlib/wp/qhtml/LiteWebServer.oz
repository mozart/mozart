functor

import
   Open
   
export
   StartServer
   StartServerOnPort
   StopServer
   AddEntry
   RemoveEntry
   GetEntry
   WebServer

define

   ServerDict={NewDictionary}
   TextSocket=class $ from Open.text Open.socket end
   NoArgs={NewName}
   
   class WebServer
      feat Socket Port Data ThId
      meth init(port:P<=NoArgs takePort:TP<=_)
	 self.Data={NewDictionary}
	 self.Socket={New TextSocket init(type:stream
					  protocol:"tcp")}
	 if {IsDet P} then {self.Socket bind(takePort:TP)}
	 else {self.Socket bind(port:P)} end
	 {self.Socket listen(backLog:5)}
      end
      meth GetId(Ref $)
	 if {VirtualString.is Ref} then
	       {VirtualString.toAtom Ref}
	 else Ref end
      end
      meth addEntry(Entry Value)
	 self.Data.{self GetId(Entry $)}:=Value
      end
      meth removeEntry(Entry)
	 {Dictionary.remove self.Data {self GetId(Entry $)}}
      end
      meth getEntry(Entry $)
	 self.Data.{self GetId(Entry $)}
      end
      meth getEntryKeys($)
	 {Dictionary.keys self.Data}
      end
      meth start
	 proc{Loop}
	    O={self.Socket accept(accepted:$
				  acceptClass:TextSocket)}
	    thread
	       fun{Get Str Remain}
		  {List.takeDropWhile
		   {List.dropWhile Str fun{$ C} C==&  end}
		   fun{$ C} C\=&  end $ Remain}
	       end
	       Req
	       try
		  Req={O getS($)}
	       catch _ then skip end
	       Cmd={Get Req _}
	       URI={Reverse
		    {List.takeWhile
		     {Reverse {Get {Get Req $ _} _}}
		     fun{$ C} C\=&/ end}}
	    in
	       if Cmd=="GET" then
		  Ref=if URI=="" then unit else {VirtualString.toAtom URI} end
	       in
		  if {Dictionary.member self.Data Ref} then
		     try
			{O write(vs:self.Data.Ref)}
		     catch _ then skip end
		  end
	       end
	       try
		  {O close}
	       catch _ then skip end
	    end
	 in
	    {Loop}
	 end
      in
	 if {IsFree self.ThId} then
	    thread
	       self.ThId={Thread.this}
	       {Loop}
	    end
	    {Wait self.ThId}
	 end
      end
      meth stop
	 if {IsDet self.Socket} then {self.Socket close} end
	 if {IsDet self.ThId} then
	    try
	       {Thread.terminate self.ThId}
	    catch _ then skip end
	 end
      end
   end
   
   fun{StartServer}
      P
      O={New WebServer init(port:P)}
   in
      {Dictionary.put ServerDict P O}
      {O start}
      P
   end
   
   proc{StartServerOnPort P}
      O={New WebServer init(takePort:P)}
   in
      {Dictionary.put ServerDict P O}
      {O start}
   end
   
   proc{StopServer PortNu}
      {{Dictionary.get ServerDict PortNu} stop}
   end

   proc{AddEntry P Ref Value}
      {{Dictionary.get ServerDict P} addEntry(Ref Value)}
   end
   proc{RemoveEntry P Ref}
      {{Dictionary.get ServerDict P} removeEntry(Ref)}
   end
   fun{GetEntry P Ref}
      {{Dictionary.get ServerDict P} getEntry(Ref $)}
   end
end
