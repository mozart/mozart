%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Contributors:
%%%   Martin Mueller <mmueller@ps.uni-sb.de>
%%% 
%%% Copyright:
%%%   Christian Schulte, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

{ErrorRegistry.put
 
 explorer

 fun {$ Exc}
    E = {Error.dispatch Exc}
    T = 'error in Oz Explorer'
 in
    case E
    of explorer(Kind OM) then
       {Error.format T
	case Kind
	of actionAdd then 'Illegal action addition'
	[] actionDel then 'Illegal action deletion'
	[] option    then 'Illegal option specification'
	end
	[hint(l:'Message'
	      m:oz(OM))]
	Exc}
    else
       {Error.formatGeneric T Exc}
    end
 end}
