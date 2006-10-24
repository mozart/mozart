%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 2001
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

{Error.registerFormatter ozcar
 fun {$ E} T in
    T = 'Ozcar error'
    case E of ozcar(badConfigFeature F X) then
       error(kind: T
	     msg: 'bad configuration parameter'
	     items: [hint(l: 'Feature' m: oz(F))
		     hint(l: 'Value' m: oz(X))])
    else
       error(kind: T
	     items: [line(oz(E))])
    end
 end}
