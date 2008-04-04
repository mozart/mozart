%%%
%%% Authors:
%%%   Yves Jaradin (yves.jaradin@uclouvain.be)
%%%   Raphael Collet (raphael.collet@uclouvain.be)
%%%
%%% Copyright:
%%%   Yves Jaradin, 2008
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   DP
   Site
   DPService
   Error

export
   offer:          OfferOnce
   offerUnlimited: OfferMany
   OfferOnce
   OfferMany
   Retract
   Take
   Gate

define
   %% initialize the ticket service
   {DP.init}
   Tickets={NewDictionary}
   {DPService.register {Site.this} 'oz:ticket'
    proc {$ 'oz:ticket' S M}
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
	     {Error.printException
	      {Exception.error dp(service localOnly 'oz:ticket' M)}}
	  end
       else
	  {Error.printException
	   {Exception.error dp(service unknownMessage 'oz:ticket' M)}}
       end
    end}

   %% obtain the value corresponding to a given ticket
   fun {Take Ticket}
      SiteURI#TicketId={ParseTicket Ticket} in
      case {DPService.send {Site.resolve SiteURI} 'oz:ticket' take(TicketId $)}
      of ok(X) then
	 X
      else
	 {Exception.raiseError dp(ticket bad Ticket)}
	 unit
      end
   end

   %% retract a ticket
   proc{Retract Ticket}
      SiteURI#TicketId={ParseTicket Ticket}
   in
      {DPService.send {Site.resolve SiteURI} 'oz:ticket' retract(TicketId)}
   end

   %% offer a value
   fun {OfferOnce X}
      {DoOffer X opts(once:true)}
   end
   fun {OfferMany X}
      {DoOffer X opts()}
   end

   fun {DoOffer X Opts}
      T={NewTicketId}
   in
      Tickets.T:=ticket(X Opts)
      {MakeTicket T}
   end
   local C={NewCell 0} in
      fun {NewTicketId}
	 I J in I=C:=J  J=I+1  I
      end
   end

   %% utils: create and parse tickets
   fun {MakeTicket T}
      URIs={Site.allURIs {Site.this}}
   in
      try
	 for U in URIs  return:Return do
	    if {List.isPrefix "oz-site://s(" U} then
	       {Return {VirtualString.toAtom
			"oz-ticket"#{List.drop U {Length "oz-site"}}#"?"#T}}
	    end
	 end
      catch _ then
	 {Exception.raiseError dp(ticket make URIs)}
	 unit
      end
   end
   fun {ParseTicket VS}
      T={VirtualString.toString VS}
   in
      if {List.isPrefix "oz-ticket://s(" T} then URI TId in
	 {List.takeDropWhile {List.drop T {Length "oz-ticket"}}
	  fun {$ C} C\=&? end
	  URI _|TId}
	 ("oz-site"#URI)#{String.toInt TId}
      else
	 {Exception.raiseError dp(ticket parse T)}
	 unit
      end
   end

   %% Gates: an alternative class for managing tickets
   class Gate
      feat Ticket
      meth init(X ?AT<=_)
	 AT = (self.Ticket = {OfferMany X})
      end
      meth getTicket($)
	 self.Ticket
      end
      meth close
	 {Retract self.Ticket}
      end
   end
end
