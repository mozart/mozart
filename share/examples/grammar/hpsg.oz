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

proc {Rules P L R}                            % binary rules only P -> L R
   {Phrase P}
   P^phon = {Append L^phon R^phon}
   dis P^subcat = nil
       P^headDtr = R
       P^compDtr = L
   []  P^subcat = [_]
       P^headDtr = L
       P^compDtr = R
   end
end

proc {Phrase P}
   H C
in
   P = p(cat:_ subcat:_ phon:_ headDtr:H compDtr:C)
   H^subcat = C^cat | P^subcat                % subcat principle
   C^subcat = nil                             % saturated complements
   P^cat = H^cat                              % head-feature principle
end

proc {Word W}
   P C S
in
   W = w(phon:[P] cat:C subcat:S)
   dis P=mary         C=noun        S=nil
   []  P=john         C=noun        S=nil
   []  P=girl         C=noun        S=[determiner]
   []  P=nice         C=adjective   S=nil
   []  P=pretty       C=adjective   S=nil
   []  P=the          C=determiner  S=nil
   []  P=laughs       C=verb        S=[noun]
   []  P=meets        C=verb        S=[noun noun]
   []  P=kisses       C=verb        S=[noun noun]
   []  P=embarrasses  C=verb        S=[noun noun]
   []  P=thinks       C=verb        S=[verb noun]
   []  P=is           C=verb        S=[adjective noun]
   []  P=met          C=adjective   S=nil
   []  P=kissed       C=adjective   S=nil
   []  P=embarrassed  C=adjective   S=nil
   end
end

fun {Parse Phon}
   {Parse1 {Map Phon proc {$ W F} F^phon=[W] {Word F} end}}
end

fun {Parse1 Fs}
   case Fs of [F] then F
   else {Parse1 {Move Fs}} end
end

proc {Move Fs ?Gs}
   case Fs of F|(G|Fr=T) then
      dis X in {Rules X F G}  Gs=X|Fr
      []  Gs = F|{Move T}
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
