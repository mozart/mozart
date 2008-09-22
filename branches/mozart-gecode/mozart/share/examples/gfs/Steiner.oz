%%%
%%% Authors:
%%%     Gustavo Andres Gomez <gafarhat@univalle.edu.co>
%%%     Andres Felipe Barco <anfelbar@univalle.edu.co>
%%%
%%% Copyright:
%%%     Gustavo Gomez, 2008
%%%     Andres Barco, 2008
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

%%% Adapted from "Problem Solving With Finite Set Constraints in Oz. A Tutorial"
%%% by Tobias Muller, 1998


declare 
fun {Steiner N}
   if 
      N mod 6 == 1 orelse N mod 6 == 3            
   then 
      proc {$ Ss}  
	 {GFS.var.list.upperBound (N*(N-1)) div 6 [1#N] Ss}
	 
	 {ForAll Ss proc {$ S} {GFS.card post(S 3)} end}    
	 
	 {ForAllTail Ss                              
	  proc {$ S1|Sr}                             
	     {ForAll Sr                              
	      proc {$ S2} S3 in
		 S3 = {GFS.var.decl}
		 S3 = {GFS.intersect S1 S2}
		 {GFS.cardRange post(S3 0 1)}               
	      end}
	  end}
	 
	 %{GFS.distribute opt(order:naive value:min) Ss}
	 {GFS.distribute naive Ss}
      end 
   else proc {$ _} fail end 
   end 
end


{Show {SearchOne {Steiner 7}}}
