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

local
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
in
   
   class Randomizer from BaseObject
      prop final locking
      attr Stamp:0 On:false Low:MedLowTime High:MedHighTime
      feat broker
      
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
