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

local
   T = 'Gump Scanner error'
in
   {ErrorRegistry.put gump
    fun {$ Exc}
       case {Error.dispatch Exc} of gump(fileNotFound FileName) then
	  {Error.format T
	   'Could not open file'
	   [hint(l: 'File name' m: oz(FileName))]
	   Exc}
       elseof gump(fatalError VS) then
	  {Error.format T
	   VS
	   nil
	   Exc}
       else
	  {Error.formatGeneric T Exc}
       end
    end}
end
