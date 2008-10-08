%%%
%%% Authors:
%%%   Andres Felipe Barco <anfelbar@univalle.edu.co>
%%%
%%% Copyright:
%%%   Andres Felipe Barco, 2008
%%%
%%% Last change:
%%%   $Date: $ by $Author: $
%%%   $Revision: $
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

%%% Adapted from a finite domain example in Mozart-Oz version 1.3.2 by 
%%% Gert Smolka, 1998.

%%% Planning a Conference
%%%
%%% Eleven sessions (S1 to S11) have to take place in
%%% a conference.  The conference is to be structured
%%% into a sequence of slots, where every session
%%% needs to be assigned to a slot, and at most three
%%% sessions can be assigned to a slot.  In addition,
%%% the following constraints must be satisfied:
%%%
%%%   S4 must take place before S11
%%%
%%%   S1 must not take place at the same time
%%%   as S2, S3, S5, S7, S8, S10
%%%
%%%   ...
%%%
%%% Compute a plan that minimizes the number of slots.


declare
fun {Conference Data}
   NbSessions    = Data.nbSessions
   NbParSessions = Data.nbParSessions
   Constraints   = Data.constraints
   MinNbSlots    = NbSessions div NbParSessions
in
   proc {$ Plan}
      NbSlots  = {GFD.int MinNbSlots#NbSessions}
   in
      {GFD.distribute naive [NbSlots]}
      %% Plan: Session --> Slot
      {GFD.tuple plan NbSessions 1#NbSlots Plan} 
      %% at most NbParSessions per slot
      {For 1 NbSlots 1  
       proc {$ Slot} {GFD.atMost NbParSessions Plan Slot} end}
      %% impose Constraints
      {ForAll Constraints
       proc {$ C}
    case C
    of before(X Y) then Plan.X <: Plan.Y
    [] disjoint(X Ys) then
       {ForAll Ys proc {$ Y} Plan.X \=: Plan.Y end}
    end
       end}
      {GFD.distributeBR naive Plan}
   end
end

Data = data(nbSessions: 11
      nbParSessions: 3
      constraints:
         [before(4 11)
    before(5 10)
    before(6 11)
    disjoint(1 [2 3 5 7 8 10])
    disjoint(2 [3 4 7 8 9 11])
    disjoint(3 [5 6 8])
    disjoint(4 [6 8 10])
    disjoint(6 [7 10])
    disjoint(7 [8 9])
    disjoint(8 [10]) ]
     )



{Show {SearchOne {Conference Data}}}
