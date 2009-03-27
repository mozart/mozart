%%%
%%% Authors:
%%%     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
%%%     Alberto Delgado <adelgado@cic.puj.edu.co>
%%%     Alejandro Arbelaez <aarbelaez@puj.edu.co>
%%%
%%%  Contributors:
%%%     Gustavo A. Gomez Farhat <gafarhat@univalle.edu.co>
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
   proc{$ Row}
      L1N = {List.number 1 N 1}
      LM1N = {List.number ~1 ~N ~1}
   in
      Row = {GFD.tuple queens N 1#N}
      {GFD.distinctP post(Row cl:GFD.cl.val)}
      {GFD.distinctOffset post(Row L1N)}
      {GFD.distinctOffset post(Row LM1N)}
      {GFD.distribute ff Row}
   end
end

{Show {SearchOne {Queens 10}}}
