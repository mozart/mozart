%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
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
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

{Error.registerFormatter explorer
 fun {$ E}
    T = 'error in Oz Explorer'
 in
    case E
    of explorer(Kind OM) then
       error(kind: T
	     msg: case Kind
		  of actionAdd then 'Illegal action addition'
		  [] actionDel then 'Illegal action deletion'
		  [] option    then 'Illegal option specification'
		  end
	     items: [hint(l:'Message'
			  m:oz(OM))])
    else
       error(kind: T
	     items: [line(oz(E))])
    end
 end}
