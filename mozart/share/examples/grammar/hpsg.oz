%%%
%%% Authors:
%%%   Gert Smolka <smolka@ps.uni-sb.de>
%%%
%%% Contributors:
%%%   Denys Duchier <duchier@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Gert Smolka, 1998
%%%   Denys Duchier, 2002
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

proc {Rules P L R}                            % binary rules only P -> L R
   {Phrase P}
   P^phon = {Append L^phon R^phon}
   thread
      or P^index   = 0
	 P^subcat  = nil
	 P^headDtr = R
	 P^compDtr = L
      [] P^index   = 1
	 P^subcat  = [_]
	 P^headDtr = L
	 P^compDtr = R
      end
   end
end

proc {Phrase P}
   H C I
in
   P = p(cat:_ subcat:_ phon:_ headDtr:H compDtr:C index:I)
   I::0#1
   H^subcat = C^cat | P^subcat                % subcat principle
   C^subcat = nil                             % saturated complements
   P^cat    = H^cat                           % head-feature principle
end

Lexicon =
% WORD        # CAT        # SUBCAT
[ mary        # noun       # nil
  john        # noun       # nil
  girl        # noun       # [determiner]
  nice        # adjective  # nil
  pretty      # adjective  # nil
  the         # determiner # nil
  laughs      # verb       # [noun]
  meets       # verb       # [noun noun]
  kisses      # verb       # [noun noun]
  embarrasses # verb       # [noun noun]
  thinks      # verb       # [verb noun]
  is          # verb       # [adjective noun]
  met         # adjective  # nil
  kissed      # adjective  # nil
  embarrassed # adjective  # nil ]

LexiconLength = {Length Lexicon}

proc {Word W}
   P C S I
in
   W = w(phon:[P] cat:C subcat:S index:I)
   I :: 1#LexiconLength
   for Phon#Cat#Subcat in Lexicon K in 1;K+1 do
      thread or I=K P=Phon C=Cat S=Subcat [] I\=:K end end
   end
end

proc {Parse Phon Phrase}
   Words   = {Map Phon proc {$ W F} F^phon=[W] {Word F} end}
   Indices = {Map Words fun {$ W} W.index end}
   PORT
in
   %% only one thread must be in control of the distribution strategy
   %% communication with this thread is done by means of a port
   %% abstractions that post disjunctions should send a message to
   %% this port with a FD variable allow to choose one or the other
   %% alternative of the disjunctions
   thread
      for Msg in {NewPort $ PORT} break:Break do
	 case Msg
	 of choose(I) then {FD.distribute naive [I]}
	 [] finish    then
	    %% make sure all lexical choices have been made
	    {FD.distribute naive Indices}
	    {Break}
	 end
      end
   end
   {Parse1 Words PORT Phrase}
   {Send PORT finish({Map Words fun {$ W} W.index end})}
end

fun {Parse1 Fs PORT}
   case Fs of [F] then {Send PORT finish} F
   else {Parse1 {Move Fs PORT} PORT} end
end

proc {Move Fs PORT ?Gs}
   case Fs of F|(G|Fr=T) then I in
      I::0#1
      {Send PORT choose(I)}
      thread
	 or I=0 Gs={Rules $ F G}|Fr
	 [] I=1 then Gs=F|{Move T PORT}
	 end
      end
   else fail
   end
end

proc {SParse Phon}
   {ExploreOne fun {$} {Parse Phon} end}
end


/*

{SParse [the girl is nice]}

{SParse [mary thinks john embarrasses the girl]}

{SParse [john thinks mary thinks mary thinks the girl thinks 
         john thinks mary thinks mary thinks the girl thinks 
         john thinks mary thinks mary thinks the girl thinks 
	 mary thinks the girl is embarrassed]}

*/
