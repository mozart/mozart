%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   proc {MagicEmacsScrollbar F L C}
      {Delay 3} %% needed for Emacs
      {Print {VS2A 'oz-scrollbar ' # F # ' ' # L # ' ' # C}}
      {Delay 3}
   end
   
in
   
   class SourceManager from Tk.toplevel
      prop
	 locking
      
      meth init
	 skip
      end
      
      meth bpAt(File Line YesNo)
	 Ok   = {Debug.breakpointAt File Line YesNo}
	 FL   = {StripPath File} # ', line ' # Line
	 D1   = 'Deleted b'
	 D2   = 'delete b'
	 Set  = 'set b'
	 Br   = 'reakpoint at '
	 Err  = 'Failed to '
      in
	 case YesNo then
	    case Ok then
	       {Ozcar status('B'#Br#FL)}
	    else
	       {Ozcar status(Err#Set#Br#FL)}
	    end
	 else
	    case Ok then
	       {Ozcar status(D1#Br#FL)}
	    else
	       {Ozcar status(Err#D2#Br#FL)}
	    end
	 end
      end
  
      meth scrollbar(file:F line:L color:C what:What<=appl)
	 lock
	    case {UnknownFile F} then
	       case What == stack then skip else
		  {MagicEmacsScrollbar undef 0 hide}
	       end
	    else
	       {MagicEmacsScrollbar {LookupFile F} L ColorMeaning.C}
	    end
	 end
      end
      
   end
end
