%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Contributors:
%%%   Martin Mueller <mmueller@ps.uni-sb.de>
%%% 
%%% Copyright:
%%%   Leif Kornstaedt, 1996-1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%   $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

{ErrorRegistry.put
 
 gump
 
 fun {$ Exc}
    E = {Error.dispatch Exc}
    T = 'Gump Scanner error'
 in
    case E of gump(fileNotFound FileName) then
       {Error.format T
	'Could not open file'
	[hint(l: 'File name' m: oz(FileName))]
	Exc}
    else
       {Error.formatGeneric T Exc}
    end
 end}

