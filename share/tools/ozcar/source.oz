%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   skip

in

   class SourceManager

      meth init
	 skip
      end

      meth bpAt(File Line YesNo)
	 Succeeded   = {Debug.breakpointAt File Line YesNo}
	 FileAndLine = {StripPath File} # ', line ' # Line
      in
	 case YesNo then
	    case Succeeded then
	       {Ozcar status('Breakpoint at ' # FileAndLine)}
	    else
	       {Ozcar status('Failed to set breakpoint at ' # FileAndLine)}
	    end
	 else
	    case Succeeded then
	       {Ozcar status('Deleted breakpoint at ' # FileAndLine)}
	    else
	       {Ozcar status('Failed to delete breakpoint at ' # FileAndLine)}
	    end
	 end
      end

   end
end
