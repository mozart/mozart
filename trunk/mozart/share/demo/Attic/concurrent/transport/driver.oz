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

class Driver from History
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
	    act(load:Load ...)|act(load:NewLoad city:Dst ...)|_ = @plan
	 in
	    plan <- @plan.2
	    if N>2 then status <- active
	    end
	    {self.truck drive(Dst Load NewLoad)}
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
	 act(load:Load ...)|act(load:NewLoad city:Dst ...)|_ = @plan
      in
	 plan <- @plan.2
	 if {Length @plan}=<1 then
	    status <- idle
	 end
	 drive(Dst Load NewLoad)
      end
   end

   meth close
      History, tkClose
      if @status==idle then {self.truck close}
      else status <- closed
      end
   end
      
end

