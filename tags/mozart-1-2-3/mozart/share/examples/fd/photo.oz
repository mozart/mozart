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
proc {Photo Root}
   Persons       = [betty chris donald fred gary mary paul]
   Preferences   = [betty#gary betty#mary chris#betty
		    chris#gary fred#mary fred#donald
		    paul#fred paul#donald]
   NbPersons     = {Length Persons}
   Alignment     = {FD.record alignment Persons 1#NbPersons}
   Satisfaction  = {FD.decl} 
   proc {Satisfied P#Q S}
      {FD.reified.distance Alignment.P Alignment.Q '=:' 1 S}
   end
in
   Root = Satisfaction#Alignment
   {FD. distinct Alignment}
   {FD.sum {Map Preferences Satisfied} '=:' Satisfaction}
   Alignment.fred <: Alignment.betty     % redundant
   {FD.distribute generic(order:naive value:max) [Satisfaction]}
   {FD.distribute split Alignment}
end


{ExploreOne Photo}


/*
{ExploreBest Photo proc {$ Old New} Old.1 <: New.1 end}
{SearchBest Photo proc {$ Old New} Old.1 <: New.1 end _}
*/