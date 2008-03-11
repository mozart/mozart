%%%
%%% Authors:
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

declare


fun{MagicSequence N}
   proc{$ Seq}
      Cs = {List.number ~1 N-2 1}
   in
      Seq = {GFD.tuple sequence N 0#N-1}
      for I in 0..N-1 do
	 {GFD.countP post(Seq I GFD.rt.'=:' Seq.(I+1) cl:GFD.cl.val)}
      end
      {GFD.linearP post(Seq GFD.rt.'=:' N cl:GFD.cl.val)}
      {GFD.linearP post(Cs Seq GFD.rt.'=:' 0 cl:GFD.cl.val)}
      {GFD.distributeBR ff Seq}
   end
end

{Show {SearchAll {MagicSequence 15}}}
