%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%   Tobias Mueller <tmueller@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Tobias Mueller and Leif Kornstaedt, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   OS(system)
export
   'class': PostScriptToGIFClass
define
   PS2GIF = 'ps2gif'

   class PostScriptToGIFClass
      attr
	 DirName: unit

      meth init(Dir)
	 DirName <- Dir
      end

      meth convertPostScript(InFileName ?OutFileName)
	 OutFileName = InFileName#'.gif'
	 case
	    {OS.system PS2GIF#' '#InFileName#' '#@DirName#'/'#OutFileName}
	 of 0 then skip
	 elseof I then
	    {Exception.raiseError ozDoc(psToGif I)}
	 end
      end
   end
end
