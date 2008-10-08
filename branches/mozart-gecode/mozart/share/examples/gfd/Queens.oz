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
      %D
      L1 L2
   in
      Root = {GFD.list N 1#N}
      {List.takeDrop Root 3 L1 L2}
      %D = {List.toTuple '#' Root}
      {GFD.distinctP post(Root cl:GFD.cl.val)}
      {GFD.distinctP post(Root C1 cl:GFD.cl.val)}
      {GFD.distinctP post(Root C2 cl:GFD.cl.val)}
      %{GFD.distribute generic(order:min value:splitMax) L1}
      {GFD.distribute algo(order:naive) L1}
      %{GFD.distribute generic(order:max value:splitMax) L2}
      {GFD.distribute algo(order:naive) L2}
   end
end

{Show {SearchOne {Queens 10}}}
