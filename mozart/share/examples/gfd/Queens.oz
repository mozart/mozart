%%%
%%% Authors:
%%%     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
%%%     Alberto Delgado <adelgado@cic.puj.edu.co>
%%%     Alejandro Arbelaez <aarbelaez@puj.edu.co>
%%%
%%% Copyright:
%%%     Gustavo Gutierrez, 2006
%%%     Alberto Delgado, 2006
%%%     Alejandro Arbelaez, 2006
%%%
%%% Last change:
%%%   $Date: 2006-10-19T01:44:35.108050Z $ by $Author: ggutierrez $
%%%   $Revision: 2 $
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

declare

fun{Queens N}
   proc{$ Root}
      C1 = {List.number 1 N 1}
      C2 = {List.number ~1 ~N ~1}
      D
   in
      Root = {GFD.list N 1#N}
      D = {List.toTuple '#' Root}
      {GFD.distinctP post(Root cl:GFD.cl.val)}
      {GFD.distinctP post(C1 Root cl:GFD.cl.val)}
      {GFD.distinctP post(C2 Root cl:GFD.cl.val)}

      {Wait {GFD.distributeC Root}}
      %{Wait {GFD.assignC Root}}
   end
end

{Show {OS.getPID}}
%{Delay 10000}
{Show {SearchAll {Queens 6}}}

%S  C A B C D

%S = {Space.new {Queens 6}}
/*
{Browse {Space.ask S}}
{Space.commitB S 2#5}

[[5 3 1 6 4 2] [4 1 5 2 6 3] [3 6 2 5 1 4] [2 4 6 1 3 5]]
*/