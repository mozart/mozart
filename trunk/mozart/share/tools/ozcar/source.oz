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
