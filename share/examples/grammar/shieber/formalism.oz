%%%
%%% Authors:
%%%   Gert Smolka <smolka@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Gert Smolka, 1998
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

GrammarAgent = {New class $ from BaseObject
		       prop final
		       attr gram:nil
		       meth grammar(type:_ gram:_) = G
			  gram <- G
		       end
		       meth query(P) 
			  Type = @gram.type
			  Gram = @gram.gram
		       in
			  {ExploreOne proc {$ F}
					 {Type s F}
					 {P F} {Gram F}
				      end}
		       end
		    end noop}

