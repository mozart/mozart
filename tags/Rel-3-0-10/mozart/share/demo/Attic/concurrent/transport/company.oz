%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
%%%   Christian Schulte, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

class Company from Contract History
   prop final
   feat toplevel
   
   meth init(toplevel:T name:C)
      Contract, init
      History, init(master:T suffix:C)
      self.toplevel = T
   end
      
   meth announce(answer:A reply:R ...) = Announcement
      Contract, Announcement
      History,  Announcement
   end
      
   meth add(driver:D city:C)
      T          = self.toplevel
      ThisDriver = {NewAgent Driver init(toplevel:T name:D city:C
					 truck:ThisTruck)}
      ThisTruck  = {NewAgent Truck  init(parent:T.map city:C
					 driver:ThisDriver)}
   in
      Contract, add(D ThisDriver)
   end

   meth close
      History,  tkClose
      Contract, close
   end

      
end
