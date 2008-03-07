functor
import
   DP
   Site
   DPService
   Error
export
   OfferOnce
   OfferMany
   Retract
   Take
define
   {DP.init}
   Tickets={NewDictionary}
   {DPService.register
    {Site.this }
    'oz:ticket'
    proc{$ 'oz:ticket' S M}
       case M
       of take(Id ?X) then
	  case {Dictionary.condGet Tickets Id unit}
	  of unit then
	     X=bad
	  [] ticket(Y Opts) then
	     if{Value.condSelect Opts once false}then
		{Dictionary.remove Tickets Id}
	     end
	     X=ok(Y)
	  end
       [] retract(Id) then
	  if S=={Site.this} then
	     {Dictionary.remove Tickets Id}
	  else
	     {Error.printException {Exception.error dp(service localOnly 'oz:ticket' M)}}
	  end
       else
	  {Error.printException {Exception.error dp(service unknownMessage 'oz:ticket' M)}}
       end
    end}
   fun{Take Ticket}
      SiteURI#TicketId={ParseTicket Ticket} in
      case {DPService.send {Site.resolve SiteURI} 'oz:ticket' take(TicketId $)}
      of ok(X) then
	 X
      else
	 {Exception.raiseError dp(ticket bad Ticket)}
	 unit
      end
   end
   proc{Retract Ticket}
      SiteURI#TicketId={ParseTicket Ticket}
   in
      {DPService.send {Site.resolve SiteURI} 'oz:ticket' retract(TicketId)}
   end
   fun{OfferOnce X}
      {Offer X opts(once:true)}
   end
   fun{OfferMany X}
      {Offer X opts()}
   end
   fun{Offer X Opts}
      T={NewTicketId}
   in
      Tickets.T:=ticket(X Opts)
      {MakeTicket T}
   end
   local
      IdC={NewCell 0}
   in
      fun{NewTicketId}O N in
	 O=IdC:=N
	 N=O+1
	 O
      end
   end
   fun{MakeTicket T}
      URIs={Site.allURIs {Site.this}}
      R=
      for U in URIs return:R default:unit do
	 if{List.isPrefix "oz-site://s(" U} then
	    {R {VirtualString.toAtom "oz-ticket"#{List.drop U {Length "oz-site"}}#"?"#T}}
	 end
      end
   in
      if R==unit then
	 {Exception.raiseError dp(ticket make URIs)}
      end
      R
   end
   fun{ParseTicket VS}
      T={VirtualString.toString VS}
   in
      if {List.isPrefix "oz-ticket://s(" T} then TId in
	 ("oz-site"#
	  {List.takeDropWhile {List.drop T {Length "oz-ticket"}} fun{$C}C\=&?end $ TId})
	 #
	 {String.toInt TId.2}
      else
	 {Exception.raiseError dp(ticket parse T)}
	 unit
      end
   end
end