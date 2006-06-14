%%%
%%% Authors:
%%%   Jörg Würtz <wuertz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Jörg Würtz, 1998
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


%%% From Constraint Satisfaction using CLP, AI 58(1992), 113-159
%%% Assembly line with 10 slots. 10 cars out of 6 classes.
%%% 5 production units providing a certain option.
%%% Each unit has a capacity, eg. 2 of 3: of 3 consecutive slots at
%%% most 2 may afford this unit (option).


declare StateConstraints in

local 

   proc {StateDomains Slots Options NbSlots NbClasses NbOptions}
      {List.make NbSlots Slots}      
      Slots = {FD.dom 1#NbClasses}
      {MakeTuple a NbOptions*NbSlots Options}
      {Record.forAll Options proc{$ O} O::0#1 end}
   end
   
   %% Implements R out of S for option Option
   %% in S consecutive slots at most R can have option Option
   %% Ops is the option tuple
   %% Ops_i^j:  slot i requires option j; Ops.(j-1)*NbSlots+i
   %% O11, O21, O31, O41, O12, O22, O32, O42 etc. for 4 slots
   proc {OutOf R S Ops NbSlots Option}
      From To in
      From = (Option-1)*NbSlots + 1
      To = (Option-1)*NbSlots + NbSlots - (S-1)
      {Loop.for From To 1 proc{$ C} {SumUp Ops C C+S-1}=<:R end}
   end
   
   %% Ops.From + Ops.(From+1) + ... + Ops.To = Res
   proc {SumUp Ops From To Res}
      {Loop.forThread From To 1 fun{$ In Index}
				   {FD.plus In Ops.Index}
				end 
       0 Res}
   end
   
   %% OptionInfo.1 = R#S#O|...  R outof S  from option O
   proc {StateCapacityConstraints Options NbSlots OptionInfo}
      {ForAll OptionInfo proc{$ X} R#S#O = !X in 
			    {OutOf R S Options NbSlots O} 
			 end}
   end
   
   fun {GetNumber CarInfo OpInfo}
      case OpInfo#CarInfo 
      of (H|R)#(_#Nb|T) then
	 if H==1 then Nb+{GetNumber T R} 
	 else {GetNumber T R} 
	 end
      [] nil#nil then 0
      end
   end
   
   %% p cars require option j. 
   %% O1j+...+O(nbslots-s)j >= p-r
   proc {StateSurrogates Options NbSlots OptionInfo2 OptionInfo1 CarInfo}
      case OptionInfo2#OptionInfo1
      of (H2|T2)#((R#S#O)|T1) then
	 % P = number of cars requiring option coded by H2
	 P={GetNumber CarInfo H2}
      in
	 {Loop.for 1 NbSlots div S 1
	  proc {$ K}
	     From = (O-1)*NbSlots+1
	     To   = (O-1)*NbSlots+NbSlots-K*S
	  in
	     {SumUp Options From To}>=:P-K*R 
	  end}
	 {StateSurrogates Options NbSlots T2 T1 CarInfo}
      [] nil#nil then skip
      end
   end


   %% CarInfo = C#Nb|...;  Nb cars from class C
   proc {StateDemandConstraints Slots CarInfo}
      {ForAll CarInfo proc {$ C#Nb} {FD.atMost Nb Slots C} end}
   end
   
   
   %% OptionInfo = [1 0 0 0 1 1]|...;   Option ? is required by classes 1,4,5
   proc {StateLinkConstraints Slots Options NbSlots OptionInfo}
      {List.forAllInd Slots
       proc {$ SC Slot}
	  {List.forAllInd OptionInfo
	   proc {$ OC OI}
	      {FD.element Slot OI Options.((OC-1)*NbSlots+SC)}
	   end} 
       end}
   end
in 

   proc {StateConstraints Slots NbSlots NbOptions NbClasses OptionInfo CarInfo}
      Options SlotVars
   in 
      SlotVars = {List.make NbSlots}
      Slots = {List.foldLInd SlotVars 
	       fun {$ Ind In S} 
		  {AdjoinAt In {VirtualString.toAtom 'slot'#Ind} S}
	       end
	       slots}
      {StateDomains SlotVars Options NbSlots NbClasses NbOptions}
      {StateCapacityConstraints Options NbSlots OptionInfo.1}
      {StateDemandConstraints SlotVars CarInfo}
      {StateLinkConstraints SlotVars Options NbSlots OptionInfo.2}
      {FD.distribute ff SlotVars}
   end

end % of local

/*
declare
OutOfInfo  = [1#2#1 2#3#2 1#3#3 2#5#4 1#5#5]  % R out of S for option
OptionInfo = [[1 0 0 0 1 1]
	      [0 0 1 1 0 1]
	      [1 0 0 0 1 0]
	      [1 1 0 1 0 0]
	      [0 0 1 0 0 0]]  % class requires option?
CarInfo    = [1#1 2#1 3#2 4#2 5#2 6#2]    % number of cars of class

{ExploreOne proc {$ Slots} 
	       {StateConstraints Slots 10 5 6 
		OutOfInfo#OptionInfo CarInfo} 
	    end}

*/


