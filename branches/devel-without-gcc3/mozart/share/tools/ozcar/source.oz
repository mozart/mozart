%%%
%%% Author:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
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

%%% Most of the functionality has been moved
%%% to the Emacs component (../Emacs.oz)

local

   skip

in

   class SourceManager

      meth bpAt(File Line YesNo)
	 Succeeded   = {Primitives.breakpointAt File Line YesNo}
	 P           = if {UnknownFile File} then ""
		       else 'file ' # {StripPath File} # ', '
		       end # 'line ' # Line
      in
	 if YesNo then
	    if Succeeded then
	       {Ozcar PrivateSend(status('Breakpoint in ' # P))}
	    else
	       {Ozcar PrivateSend(status('Failed to set breakpoint in ' # P))}
	    end
	 else
	    if Succeeded then
	       {Ozcar PrivateSend(status('Deleted breakpoint in ' # P))}
	    else
	       {Ozcar PrivateSend(status('Failed to delete breakpoint in ' #
					 P))}
	    end
	 end
      end

   end
end
