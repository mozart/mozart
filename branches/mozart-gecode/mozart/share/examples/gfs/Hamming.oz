%%%
%%% Authors:
%%%     Andres Felipe Barco <anfelbar@univalle.edu.co>
%%%     Gustavo Andres Gomez <gafarhat@univalle.edu.co>
%%%
%%% Copyright:
%%%     Andres Barco, 2008
%%%     Gustavo Gomez, 2008
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
fun {Hamming Bits Distance NumSymbols}       
   proc {MinDist X Y}
      Common1s = {GFS.var.decl}
      Common0s = {GFS.var.decl}
      Uni = {GFS.var.decl}
      Ca1 = {GFD.decl}
      Ca2 = {GFD.decl}
      
      {GFS.intersect X Y Common1s}
      {GFS.union X Y Uni}
      
      Common0s = {GFS.complIn Uni
		  {GFS.value.make [1#Bits]}}
   in
      {GFS.card post(Common1s Ca1)}
      {GFS.card post(Common0s Ca2)}
      Bits - Ca1 - Ca2 >=: Distance
   end 
in 
   proc {$ Xs}
      Xs = {GFS.var.list.upperBound NumSymbols [1#Bits]}
      
      {ForAllTail Xs proc {$ X|Y}
			{ForAll Y proc {$ Z}
				     {MinDist X Z}
				  end}
		     end}
      
      %{GFS.distribute opt(order:naive value:min) Xs}
      %{GFS.distribute naive Xs}
      {GFS.distribute generic(order:order(sel:min)) Xs}
   end 
end

/*{Browse
 {Map {SearchOne {Hamming 7 2 16}}.1
  fun {$ X}  
     {ForThread 7 1 ~1 fun {$ Is I}
			  {GFS.reified.isIn I X}|Is
		       end nil}  
  end}}
*/

{Show {SearchOne {Hamming 7 2 16}}}