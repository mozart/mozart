%%%
%%% Authors:
%%%   Denys Duchier <duchier@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Denys Duchier, 1999
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

%%% The following puzzle appeared on comp.theory in Nov 1999:
%%%
%%% I am looking for an algorithm. I wanted to program a tournament
%%% schedule for checkers. But I can't seem to find an algorithm. There
%%% are six tables and there are six rounds that people can play. There
%%% are 12 people who will be playing.
%%% Every player should play against another player (never play against
%%% someone twice). And every player should sit on another table every
%%% round (so at the end of the tournament he should have sit on every
%%% table).

declare
proc {Tournament Solution}
   %% N tables, N rounds, 2*N players
   N=6 NN=2*N
   %%
   Players = {Tuple.make o NN}
   {For 1 NN 1
    proc {$ I} Player = Players.I in
       %% each player is represented by a record
       %% * on feature `table' is a tuple of FDs
       %%   for the table at which he plays at each round
       %% * on feature `opponent' is a tuple of FDs
       %%   for the opponent against whom he plays
       %%   at each round
       Player = o(table   : {FD.tuple o N 1#N }
		  opponent: {FD.tuple o N 1#NN})
       %% never plays against himself
       {For 1 N 1 proc {$ R} Player.opponent.R\=:I end}
       %% never plays the same opponent twice
       {FD.distinct Player.opponent}
       %% never plays at the same table twice
       {FD.distinct Player.table}
    end}
   %% at each round, for each table there must be precisely
   %% 2 players assigned to it
   {For 1 N 1
    proc {$ I}
       %% assignment of players to tables at round I
       Player2Table = {Record.map Players fun {$ P} P.table.I end}
    in
       %% for each table J
       {For 1 N 1
	proc {$ J} {FD.exactly 2 Player2Table J} end}
    end}
   %% if   I plays against J at table T
   %% then J plays against I at table T
   {For 1 (NN-1) 1
    proc {$ I} P1 = Players.I in
       {For (I+1) NN 1
	proc {$ J} P2 = Players.J in
	   {For 1 N 1
	    proc {$ R} B in
	       B::0#1
	       B
	       =(P1.table.R=:P2.table.R)
	       =(P1.opponent.R=:J)
	       =(P2.opponent.R=:I)
	    end}
	end}
    end}
in
   %% on the first round, players can be arbitrarily assigned to tables
   %% e.g. players 1&2 to table 1, player 3&4 to table 2, etc...
   {For 1 NN 1
    proc {$ I}
       Players.I.table.1 = (I+1) div 2
    end}
   %% to cut down on symmetries, player 1 can play tables in increasing
   %% order
   {For 1 N 1
    proc {$ R}
       Players.1.table.R = R
    end}
   Solution = Players
   {FD.distribute
    ff {Record.foldL Players
	fun {$ L P} {Append {Record.toList P.table} L} end
	nil}}
end

{ExploreOne Tournament}
