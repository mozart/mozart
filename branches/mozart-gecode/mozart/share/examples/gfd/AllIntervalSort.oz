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


proc{Adiff_sn X N}
   SN in
   SN = {Tuple.make sn N}
   for I in N..1;~1 do
      SN.I = {GFD.int (I-1)#(I-1)}
   end
   {GFD.sortedness X SN GFD.cl.bnd}
end

proc{Adiff_sn_star X N}
   SNSTAR in
   SNSTAR = {Tuple.make snstar N-1}
   for I in N-1..1;~1 do
      %%SNSTAR.I = {GFD.int (I+1)#(I+1)}
      SNSTAR.I = {GFD.int I#I}
   end
   {GFD.sortedness X SNSTAR GFD.cl.bnd}
end

proc{Difference X D N}
   DIFF in
   DIFF = {Tuple.make diff N-1}
   for I in 1..(N-1) do %NN = N-1 in
      DIFF.I = {GFD.int (1-N)-1#(N-1)}
   end
   for I in 1..(N-1) do
      {GFD.linear2 [1 ~1] [X.(I+1) X.I] GFD.rt.'=:' DIFF.I GFD.cl.bnd}
      {GFD.abs DIFF.I D.I GFD.cl.bnd}
   end
end

proc{Break_negation X}
   {GFD.rel X.1 GFD.rt.'=<:' X.2 GFD.cl.bnd}
end

proc{Break_reversal D N}
   {GFD.rel D.1 GFD.rt.'=<:' D.(N-1) GFD.cl.bnd}
end

fun{AllInterval N}
   proc{$ Root}
      Dom_zn = 0#(N-1)
      Dom_zns = 1#(N-1)
      X D
   in
      Root = [X D]
      
      X = {GFD.tuple x N Dom_zn}
      D = {GFD.tuple d N-1 Dom_zns}
      
      {Difference X D N}
      
      {Break_negation X}
      {Break_reversal D N}
      
      
      {Adiff_sn X N}
      {Adiff_sn_star D N}
      
      {GFD.distribute ff X}
   end
end

{Show {SearchOne {AllInterval 7}}}