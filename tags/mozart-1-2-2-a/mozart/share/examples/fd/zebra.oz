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

%%% Zebra Puzzle, from PvH's 1989 book.
%%% 
%%% Five men with different nationalities live in the
%%% first five houses of a street.  There are only
%%% houses on one side of the street.  The men practice
%%% distinct professions, and each of them has a
%%% favorite drink and a favorite animal, all of them
%%% different.  The five houses are painted with
%%% different colors.  The following facts are known:
%%% 
%%%    The Englishman lives in a red house.
%%%    The Spaniard owns a dog.
%%%    The Japanese is a painter.
%%%    The Italian drinks tea.
%%%    The Norwegian lives in the first house.
%%%    The owner of the green house drinks coffee.
%%%    The green house comes after the white one.
%%%    The sculptor breeds snails.
%%%    The diplomat lives in the yellow house.
%%%    Milk is drunk in the third house.
%%%    The Norwegian's house is next to the blue one.
%%%    The violinist drinks juice.
%%%    The fox is in the house next to that of the doctor.
%%%    The horse is in the house next to that of the diplomat.
%%%    The zebra is in the white house.
%%% 
%%% Who lives where?
						     
declare
proc {Zebra Nb}
   Groups     = [ [english spanish japanese italian norvegian]
		  [green red yellow blue white]
		  [painter diplomat violinist doctor sculptor]
		  [dog zebra fox snails horse]
		  [juice water tea coffee milk] ]
   Properties = {FoldR Groups Append nil}
   proc {Partition Group}
      %% The properties in Group hold for distinct house numbers
      {FD.distinct {Map Group fun {$ P} Nb.P end}}
   end
   proc {Adjacent X Y}
      {FD.distance X Y '=:' 1}
   end
in
   %% Nb maps all properties to house numbers
   {FD.record number Properties 1#5 Nb}
   {ForAll Groups Partition}
   Nb.english = Nb.red
   Nb.spanish = Nb.dog
   Nb.japanese = Nb.painter
   Nb.italian = Nb.tea
   Nb.norvegian = 1
   Nb.green = Nb.coffee
   Nb.green >: Nb.white
   Nb.sculptor = Nb.snails
   Nb.diplomat = Nb.yellow
   Nb.milk = 3
   {Adjacent Nb.norvegian Nb.blue}
   Nb.violinist = Nb.juice
   {Adjacent Nb.fox Nb.doctor}
   {Adjacent Nb.horse Nb.diplomat}
   Nb.zebra = Nb.white
   {FD.distribute ff Nb}
end

{ExploreAll Zebra}

