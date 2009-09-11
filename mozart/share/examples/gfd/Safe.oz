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
   {GFD.tuple code 9 1#9 C}
   {GFD.distinctP  post(C cl:bnd)}
   {GFD.linearP post([1 ~1 ~1] [C.4 C.6 C.7] '=:' 0 cl:val)}
   
   local Tmp1 Tmp2 in
      Tmp1 = {GFD.decl}
      Tmp2 = {GFD.decl}
      {GFD.multP post(C.1 C.2 Tmp1)}
      {GFD.multP post(Tmp1 C.3 Tmp2)}
      {GFD.linearP post([1 ~1 ~1] [Tmp2 C.8 C.9] '=:' 0 cl:val)}
   end
   
   {GFD.linearP post([1 1 1 ~1] [C.2 C.3 C.6 C.8] '<:' 0 cl:val)}
   {GFD.linearP post([1 ~1] [C.9 C.8] '<:' 0 cl:val)}

   for I in 1..9 do
      C.I \=: I
   end
   {GFD.distribute ff C}
end

{Show {SearchAll Safe}}