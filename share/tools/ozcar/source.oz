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
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
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
	 Succeeded   = {Debug.breakpointAt File Line YesNo}
	 P           = case {UnknownFile File} then ""
		       else 'file ' # {StripPath File} # ', '
		       end # 'line ' # Line
      in
	 case YesNo then
	    case Succeeded then
	       {Ozcar PrivateSend(status('Breakpoint in ' # P))}
	    else
	       {Ozcar PrivateSend(status('Failed to set breakpoint in ' # P))}
	    end
	 else
	    case Succeeded then
	       {Ozcar PrivateSend(status('Deleted breakpoint in ' # P))}
	    else
	       {Ozcar PrivateSend(status('Failed to delete breakpoint in ' #
					 P))}
	    end
	 end
      end

   end
end
