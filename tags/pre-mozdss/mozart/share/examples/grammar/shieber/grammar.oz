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

%%%
%%% Sample Grammar from Stuart Shieber's CSLI Lecture Notes 4
%%% includes: subject-verb agreement
%%%           complex subcategorization
%%%           logical form construction
%%%           lexical organization 
%%% Acknowledgements to Martin Emele
%%%


%%%%%%%
% RULES
%%%%%%%

/*
S --> NP VP
   S^head^form = finite
   S^head = VP^head
   VP^subcat = [NP]


VP --> V X1 ... Xn
   VP^head = V^head
   VP^subcat = [NP]
   V^subcat = [NP X1 ... Xn]
*/


local
% auxiliaries for better readability
proc {Hd Y X} Y=X|_ end
proc {Tl Y X} Y=_|X end
   
proc {Gram F}
   case {Label F}
   of s then
      NP={Type np}  VP={Type vp}
   in
      F^head^form = finite
      F^head = VP^head
      VP^subcat = [NP]
      {GramRec F [NP VP]}
   [] vp then 
      V={Type v}
   in
      F^head = V^head
      F^subcat = [{Hd V^subcat}]
      {GramRec F V|{Tl V^subcat}}
   [] np then {NPLexicon F}
   [] v  then {VLexicon F}
   end
end

proc {GramRec P Dtrs}
   {FoldL Dtrs 
    proc {$ Lexin D Lexout} 
       Lexin=D^lexin  D^lexout=Lexout {Gram D} 
    end
    P^lexin P^lexout}
end


%%%%%%%%%
% LEXICON
%%%%%%%%%

proc {NPLexicon NP}
   Lex|NP^lexout = NP^lexin  Agree = NP^head^agree
in
   NP^head^lf = Lex
   dis Lex= uther      Agree= ThirdSgMasc
   []  Lex= cornwall   Agree= ThirdSgMasc
   []  Lex= kuno       Agree= ThirdSgMasc
   []  Lex= knights    Agree= ThirdPlMasc
   end
end

proc {VLexicon V} 
   Lex|V^lexout = V^lexin   LF = {Hd V^head^lf}
in
   dis Lex= sleeps     {Intransitive V}  {ThirdSingular V}     LF= sleep
   []  Lex= sleep      {Intransitive V}                        LF= sleep
                       {NotThirdSingularOrNonfinite V} 
   []  Lex= slept      {Intransitive V}  {Pastparticiple V}    LF= sleep
   []  Lex= storms     {Transitive V}    {ThirdSingular V}     LF= storm
   []  Lex= stormed    {Transitive V}    {Pastparticiple V}    LF= storm
   []  Lex= storm      {Transitive V}                          LF= storm
		       {NotThirdSingularOrNonfinite V} 
   []  Lex= beats      {Transitive V}    {ThirdSingular V}     LF= beat
   []  Lex= beaten     {Transitive V}    {Pastparticiple V}    LF= beat
   []  Lex= beat       {Transitive V}                          LF= beat
	               {NotThirdSingularOrNonfinite V} 
   []  Lex= has        {Auxiliary V}     {ThirdSingular V}
   []  Lex= have       {Auxiliary V}     {NotThirdSingular V}
   []  Lex= persuades  {TransitiveTo V}  {ThirdSingular V}     LF= persuade
   []  Lex= persuade   {TransitiveTo V}                        LF= persuade
	               {NotThirdSingularOrNonfinite V} 
   []  Lex= persuaded  {TransitiveTo V}  {Pastparticiple V}    LF= persuade
   []  Lex= to         {Infinitival V}
   end
end

%%%%%%%%%%%%%
% Definitions
%%%%%%%%%%%%%

fun {Type T}
   case T
   of s   then   s(head:{Type vhd} lexin:{Type lex} lexout:nil)
   [] np  then  np(head:{Type nhd} lexin:{Type lex} lexout:{Type lex})
   [] vp  then  vp(head:{Type vhd} subcat:{Type lvn} 
		   lexin:{Type lex} lexout:{Type lex})
   [] v   then   v(head:{Type vhd} subcat:{Type lvn} 
		   lexin:{Type lex} lexout:{Type lex})
   [] vhd then vhd(form:{Type 'for'} lf:{Type lf})
   [] nhd then nhd(agree:{Type agr} lf:{Type a})
   else _
   end 
   /*
   [] 'for' then finite + nonfinite + pastparticiple + infinitival
   [] agr then agr(num   : singular + plural
		   person: first + second + third
		   gender: masculine + feminine + neuter)
   [] lex then listof a
   [] lvn then listof np + vp
   []  lf then a + [a lf ...]
   []   a then <Atom>
   */
end

ThirdSgMasc = agr(num:singular person:third gender:masculine)
ThirdPlMasc = agr(num:plural person:third gender:masculine)

proc {Intransitive V}
   Subj = {Type np}
in 
   V^subcat = [Subj]
   V^head^lf = [_ Subj^head^lf]
end
proc {Transitive V}
   Subj = {Type np} Obj = {Type np}
in
   V^subcat = [Subj Obj]
   V^head^lf = [_ Subj^head^lf Obj^head^lf]
end
proc {TransitiveTo V}
   Subj = {Type np} Obj = {Type np} VComp = {Type vp}
in
   V^subcat = [Subj Obj VComp]
   VComp^head^form = infinitival
   VComp^subcat = [Obj]
   V^head^lf = [_ Subj^head^lf Obj^head^lf VComp^head^lf]
end
proc {Auxiliary V}
   Subj = {Type np} VP = {Type vp}
in
   {Pastparticiple VP}
   VP^subcat = [Subj]
   V^subcat = [Subj VP]
   V^head^lf = [perfective VP^head^lf]
end
proc {Infinitival V}
   Subj = {Type np} VP = {Type vp}
in
   V^head^form = infinitival
   {Nonfinite VP}
   VP^subcat = [Subj]
   V^subcat = [Subj VP]
   V^head^lf = VP^head^lf
end
proc {ThirdSingular V}
   V^head^form = finite
   {Hd V^subcat}^head^agree = agr(num:singular person:third gender:_)
end
proc {NotThirdSingular V}
   A = {Hd V^subcat}^head^agree
in 
   V^head^form = finite
   not A^person=third A^num=singular end
end
proc {Nonfinite V}  V^head^form = nonfinite  end
proc {Pastparticiple V}  V^head^form = pastparticiple end
proc {NotThirdSingularOrNonfinite V}
   or {NotThirdSingular V} [] {Nonfinite V} end
end

%%%%%%%%%%%%%%
% Load Grammar
%%%%%%%%%%%%%

in

{GrammarAgent grammar(gram:Gram type:Type)}

end


