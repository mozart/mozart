%%%
%%% Authors:
%%%   Gert Smolka <smolka@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Gert Smolka, 1998
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

declare
proc {Money Root}
   S E N D M O R Y
   RootVal
in
   Root =  [S E N D M O R Y]
   RootVal = [1000 100 10 1 1000 100 10 1 ~10000 ~1000 ~100 ~10 ~1]
   {FD.dom 0#9 Root}
   {FD.distinct Root}
   {FD.sumC RootVal
    [S E N D M O
     R E M O N E Y]
    '=:' 0}

    {FD.distribute ff Root}

   S \=: 0 
   M \=: 0
                1000*S + 100*E + 10*N + D
   +            1000*M + 100*O + 10*R + E
   =: 10000*M + 1000*O + 100*N + 10*E + Y
   {FD.distribute ff Root}
end

%{ExploreAll Money}
{Browse {SearchOne Money}}

/*
{Inspect {SearchOne Money}}

{Inspect {SearchAll Money}}

*/