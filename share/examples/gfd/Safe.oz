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

proc{Safe C}
   C1 C2 C3 C4 C5 C6 C7 C8 C9
in
   C = [C1 C2 C3 C4 C5 C6 C7 C8 C9]
   C:::1#9
   {GFD.distinctP  post(C cl:GFD.cl.bnd)}

   {GFD.sumC [1 ~1 ~1] [C4 C6 C7] '=:' 0}
   local Tmp1 Tmp2 in
      Tmp1 = {GFD.decl}
      Tmp2 = {GFD.decl}
      {GFD.multP post(C1 C2 Tmp1)}
      {GFD.multP post(Tmp1 C3 Tmp2)}
      {GFD.sumC [1 ~1 ~1] [Tmp2 C8 C9] '=:' 0}
   end
   {GFD.sumC [1 1 1 ~1] [C2 C3 C6 C8] '<:' 0}
   {GFD.sumC [1 ~1] [C9 C8] '<:' 0}
   for I in 1..9 do
      {List.nth C I} \=: I
   end
   {GFD.distributeBR ff C}
end

{Show {SearchAll Safe}}