%% This is a hack to keep both the old Oz-layer and the new one usable.
functor
import
   NewConnection(offerOnce:OfferOnce offerMany:OfferMany retract:Retract take:NewTake)
   OldConnection(offer:Offer offerUnlimited:OfferUnlimited gate:Gate take:OldTake)
export
   Offer
   OfferUnlimited
   Gate
   
   OfferOnce
   OfferMany
   Retract
   
   Take
define
   fun{Take T}
      case {List.takeWhile {VirtualString.toString T} fun{$C}C\=&: end}
      of "x-ozticket" then
	 {OldTake T}
      else
	 {NewTake T}
      end
   end
end