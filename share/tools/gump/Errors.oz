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
%%%   http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%   http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

{ErrorRegistry.put gump
 fun {$ E} T in
    T = 'Gump Scanner error'
    case E of gump(fileNotFound FileName) then
       error(kind: T
	     msg: 'Could not open file'
	     items: [hint(l: 'File name' m: oz(FileName))])
    elseof gump(fatalError VS) then
       error(kind: T
	     msg: VS)
    else
       error(kind: T
	     items: [line(oz(E))])
    end
 end}
