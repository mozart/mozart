%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   proc {MagicEmacsBar File Line Column State}
      C = case Column == unit then 0 else Column end
   in
      {Delay 5} %% needed for Emacs (??)
      {Print {VS2A 'oz-bar ' # File # ' ' # Line # ' ' # C # ' ' # State}}
      {Delay 5}
   end

in

   class SourceManager from Tk.toplevel
      prop
	 locking

      attr
	 BarSync  : _

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

      meth bar(file:F line:L column:C state:S)
	 BarSync <- _ = unit
	 case {UnknownFile F} orelse L == unit then
	    SourceManager,removeBar
	 else
	    {MagicEmacsBar {LookupFile F} L C S}
	 end
      end

      meth delayedBar(file:F line:L column:C state:S<=unchanged)
	 New in BarSync <- New = unit
	 thread
	    {WaitOr New {Alarm TimeoutToUpdateBar}}
	    case {IsDet New} then skip else
	       SourceManager,bar(file:F line:L column:C state:S)
	    end
	 end
      end

      meth configureBar(State)
	 New in BarSync <- New = unit
	 thread
	    {WaitOr New {Alarm TimeoutToConfigBar}}
	    case {IsDet New} then skip else
	       {MagicEmacsBar unchanged 0 0 State}
	    end
	 end
      end

      meth removeBar
	 BarSync <- _ = unit
	 {OzcarMessage 'removing bar...'}
	 {MagicEmacsBar nofile 0 0 hide}
      end

   end
end
