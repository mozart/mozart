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
   OS
   
   Configure(delta:    Delta
	     delay:    DelayMove
	     capacity: Capacity
	     goods:    Goods)
   Country(cities)

export
   'class': Randomizer
   
define

   %% Randomizer
   local
      Speed = (1000 * Delta) div DelayMove % Relative speed
   in
      MedLowTime   = 20 * Speed        % Lower bound for random time interval
      MedHighTime  = 3 * MedLowTime    % Upper bound for random time interval
      SlowLowTime  = MedLowTime div 2  % Lower bound for random time interval
      SlowHighTime = MedHighTime div 2 % Upper bound for random time interval
      FastLowTime  = 2 * MedLowTime    % Lower bound for random time interval
      FastHighTime = 2 * MedHighTime   % Upper bound for random time interval
   end
   
   LowGood  = Capacity div 10 % Lower bound for random weight of goods
   HighGood = Capacity        % Upper bound for random weight of goods


   Cities = {List.toTuple '#' Country.cities}
   
   fun {RandomDot T}
      T.({OS.rand} mod {Width T} + 1)
   end
   
   fun {Uniform Low High}
      {OS.rand} mod (High - Low) + Low
   end
   
   fun {DiffCity City}
      RandCity = {RandomDot Cities}
   in
      if RandCity==City then {DiffCity City}
      else RandCity
      end
   end
   
   class Randomizer
      prop
	 final locking
      attr
	 Stamp: 0
	 On:    false
	 Low:   MedLowTime
	 High:  MedHighTime
      feat
	 broker
      
      meth init(broker:B)
	 self.broker = B
      end
      
      meth toggle
	 case
	    lock
	       if @On then On <- false  Stamp <- @Stamp + 1 ~1
	       else On <- true @Stamp
	       end
	    end
	 of ~1 then skip elseof S then {self Go(S)}
	 end
      end
      
      meth slow
	 lock Low <- SlowLowTime   High <- SlowHighTime end
      end
      
      meth medium
	 lock Low <- MedLowTime    High <- MedHighTime  end
      end
      
      meth fast
	 lock Low <- FastLowTime   High <- FastHighTime end
      end
      
      meth Go(S)
	 CurS CurLow CurHigh
      in
	 lock
	    CurS    = @Stamp
	    CurLow  = @Low
	    CurHigh = @High
	 end
	 if CurS==S then
	    Src={RandomDot Cities}
	 in
	    {self.broker announce(src:    Src
				  dst:    {DiffCity  Src}
				  what:   {RandomDot Goods}
				  weight: {Uniform LowGood HighGood})}
	    {Delay {Uniform CurLow CurHigh}}
	    {self Go(S)}
	 end
      end
      
      meth close
	 lock
	    {Wait _}
	 end
      end
      
   end
end
