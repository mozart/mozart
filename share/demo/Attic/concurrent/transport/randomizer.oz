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
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
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
      case RandCity==City then {DiffCity City}
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
               case @On then On <- false  Stamp <- @Stamp + 1 ~1
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
         case CurS==S then
            Src={RandomDot Cities}
         in
            {self.broker announce(src:    Src
                                  dst:    {DiffCity  Src}
                                  what:   {RandomDot Goods}
                                  weight: {Uniform LowGood HighGood})}
            {Delay {Uniform CurLow CurHigh}}
            {self Go(S)}
         else skip
         end
      end
      meth close
         lock
            {Wait _}
         end
      end
   end
end
