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

declare

proc{Square X S}
   {GFD.mult X X S}
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
   {GFD.linear [AA BB] GFD.rt.'=:' CC GFD.cl.val}
   {GFD.rel A GFD.rt.'=<:' B GFD.cl.val}
   {GFD.rel B GFD.rt.'=<:' C GFD.cl.val}

   {GFD.distribute ff Root}
end

{Show {SearchAll Pythagoras}}

