%%%
%%% Authors:
%%%   Tobias Müller <tmueller@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Tobias Müller, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
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
fun {Hamming Bits Distance NumSymbols}      
   proc {MinDist X Y}      
      Common1s = {FS.intersect X Y}
      Common0s = {FS.complIn {FS.union X Y} {FS.value.make [1#Bits]}}
   in
      Bits - {FS.card Common1s} - {FS.card Common0s} >=: Distance
   end
in
   proc {$ Xs}
      Xs = {FS.var.list.upperBound NumSymbols [1#Bits]}
      
      {ForAllTail Xs proc {$ X|Y} {ForAll Y proc {$ Z} {MinDist X Z} end} end}
      
      {FS.distribute naive Xs}
   end
end

{Explorer.object add(information
		     proc {$  Node Sol}
			{Browse {Map Sol
				 fun {$ X} {Map {List.number 1 7 1}
					    fun {$ E} {FS.reified.isIn E X}
					    end}
				 end}}
		     end
		     label: 'Hamming Code')}
