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

fun{AllInterval N}
   proc{$ Root}
      D X
   in
      X = {GFD.tuple x N 0#N-1}
      D = {GFD.tuple d N-1 1#N-1}
      X = Root
      for I in 1..(N-1) do Tmp1 in
	 Tmp1 = {GFD.decl}
	 {GFD.minus X.(I+1) X.I Tmp1}
	 {GFD.abs post(Tmp1 D.I cl:GFD.cl.bnd)}
      end
      {GFD.distinctP post(X cl:GFD.cl.bnd)}
      {GFD.distinctP post(D cl:GFD.cl.bnd)}
      
      X.1 =<: X.2
      D.1 >=: D.2
      
      {GFD.distributeBR naive Root}
   end
end

{Show {SearchOne {AllInterval 8}}}
