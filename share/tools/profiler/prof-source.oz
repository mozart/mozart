%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   proc {MagicEmacsBar File Line State}
      {Delay 5} %% needed for Emacs
      {Print {VS2A 'oz-bar ' # File # ' ' # Line # ' ' # State}}
      {Delay 5}
   end
   
in
   
   class SourceManager from Tk.toplevel
      
      meth init
	 skip
      end
      
      meth bar(file:F line:L state:S)
	 case {UnknownFile F} then
	    SourceManager,removeBar
	 else
	    {MagicEmacsBar {LookupFile F} L S}
	 end
      end
      
      meth removeBar
	 {MagicEmacsBar undef 0 hide}
      end
      
   end
end
