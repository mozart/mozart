%%%
%%% Authors:
%%%     Alejandro Arbelaez <aarbelaez@puj.edu.co>
%%%
%%% Copyright:
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

proc{Square X S}
   {GFD.multP post(X X S)}
end

proc{Pythagoras Root}
   [A B C] = Root
   AA BB CC
in

   {GFD.dom 1#1000 Root}
   {GFD.dom 0#FD.sup [AA BB CC]}
   {Square A AA}
   {Square B BB}
   {Square C CC}
   {GFD.sum [AA BB] '=:' CC}
   A =<: B
   B =<: C

   {GFD.distribute ff Root}
end

{Show {SearchAll Pythagoras}}

