%%%
%%% Authors:
%%%   Author's name (Author's email address)
%%%
%%% Contributors:
%%%   optional, Contributor's name (Contributor's email address)
%%%
%%% Copyright:
%%%   Organization or Person (Year(s))
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
%%%
%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   skip

in

   class SourceManager

      meth bpAt(File Line YesNo)
	 Succeeded   = {Debug.breakpointAt File Line YesNo}
	 P           = {StripPath File} # ', line ' # Line
      in
	 case YesNo then
	    case Succeeded then
	       {Ozcar PrivateSend(status('Breakpoint at ' # P))}
	    else
	       {Ozcar PrivateSend(status('Failed to set breakpoint at ' # P))}
	    end
	 else
	    case Succeeded then
	       {Ozcar PrivateSend(status('Deleted breakpoint at ' # P))}
	    else
	       {Ozcar PrivateSend(status('Failed to delete breakpoint at ' #
					 P))}
	    end
	 end
      end

   end
end
