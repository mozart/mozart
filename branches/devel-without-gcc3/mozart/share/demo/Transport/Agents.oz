%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
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
   Tk
   
   Plan(make: MakePlan)
   Widgets(history: History
	   truck:   Truck)
   AgentAbstractions(new:      NewAgent
		     contract: Contract)
   
export
   Broker
   Company
   Driver

define
   
   class Broker
      from Contract History
      prop final
      feat toplevel map
	 
      meth init(toplevel:T)
	 Contract, init
	 History, init(master:T suffix:'Broker')
	 self.toplevel = T
      end
      
      meth announce(...) = M
	 A R
      in
	 Contract, {Adjoin M announce(answer:?A reply:R)}
	 if A\=reject then
	    R=grant
	 end
	 History, M
      end
      
      meth add(company:C driver:D<=unit ...) = M
	 if D==unit then
	    Contract,add(C {NewAgent Company
			    init(name:C toplevel:self.toplevel)})
	 else 
	    {Contract,get(C $) add(driver:D city:M.city)}
	 end
      end
      
      meth remove(company:C driver:D<=unit)
	 if D==unit then Contract,remove(C) else
	    {Contract,get(C $) remove(D)}
	 end
      end
      
      meth close
	 History,  tkClose
	 Contract, close
      end

   end


   class Company
      from Contract History
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


   class Driver
      from History
      prop final
	 
      feat truck
	 
      attr plan:nil status:idle
	 
      meth init(toplevel:M name:N truck:T city:C)
	 History, init(master:M suffix:N)
	 plan <- [act(city:C load:0 lift:nil drop:nil)]
	 %% Create the truck
	 self.truck = T
      end
      
      meth deiconify
	 {Tk.send wm(deiconify self)}
      end
      
      meth announce(answer:A reply:R ...) = Announce
	 NewPlan 
      in
	 {MakePlan @plan Announce ?A ?NewPlan}
	 case R
	 of reject then skip
	 [] grant  then plan <- NewPlan
	 end
	 History, Announce
	 Driver,  check
      end
      
      meth check
	 case @status
	 of closed then {self.truck close}
	 [] idle   then
	    N={Length @plan}
	 in
	    if N>1 then
	       case @plan of
		  act(load:Load ...)|act(load:NewLoad city:Dst ...)|_
	       then
		  plan <- @plan.2
		  if N>2 then status <- active
		  end
		  {self.truck drive(Dst Load NewLoad)}
	       end
	    end
	 [] active then skip
	 end
      end
      
      meth getMessage($)
	 case @status
	 of closed then close
	 [] idle then
	    History, print('idle.')
	    load(0)
	 [] active then
	    case @plan of
	       act(load:Load ...)|act(load:NewLoad city:Dst ...)|_
	    then
	       plan <- @plan.2
	       if {Length @plan}=<1 then
		  status <- idle
	       end
	       drive(Dst Load NewLoad)
	    end
	 end
      end
      
      meth close
	 History, tkClose
	 if @status==idle then
	    {self.truck close}
	 else
	    status <- closed
	 end
      end
      
   end

end

