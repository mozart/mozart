%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   OS(system)
export
   'class': ThumbnailsClass
define
   GIF2THUMBNAIL = 'gif2thumbnail'

   class ThumbnailsClass
      attr DirName: unit N: unit ToProcess: unit
      meth init(Dir)
	 DirName <- Dir
	 N <- 0
	 ToProcess <- nil
      end
      meth get(FileName ?OutFileName)
	 N <- @N + 1
	 OutFileName = 'thumbnail'#@N#'.gif'
	 ToProcess <- FileName#OutFileName|@ToProcess
      end
      meth process(Reporter)
	 case @ToProcess of nil then skip
	 elseof Xs then
	    {Reporter startSubPhase('generating thumbnails')}
	    {ForAll Xs
	     proc {$ FileName#OutFileName}
		case
		   {OS.system
		    GIF2THUMBNAIL#' '#FileName#' '#@DirName#'/'#OutFileName}
		of 0 then skip
		elseof I then
		   {Exception.raiseError ozDoc(thumbnail FileName I)}
		end
	     end}
	    ToProcess <- nil
	 end
      end
   end
end
