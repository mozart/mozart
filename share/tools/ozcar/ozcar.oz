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

   class OzcarClass
      from
	 ThreadManager
	 SourceManager
	 Gui

      prop
	 final

      meth init
	 ThreadManager,init
	 Gui,init
      end

      meth on
	 {Tk.batch [update(idletasks)
		    wm(deiconify self.toplevel)]}
	 case {CgetTk emacsThreads} then
	    {Compile '\\switch +debuginfo'}
	 else
	    {Compile '\\switch +debuginfovarnames +debuginfocontrol'}
	 end
	 {Dbg.on}
	 case @currentThread == unit then
	    Gui,status(TitleName # ' initialized')
	 else skip end
      end

      meth off
	 {Dbg.off}
	 {Tk.send wm(withdraw self.toplevel)}
	 {Compile '\\switch -debuginfo'}
	 {Emacs removeBar}
      end
   end

in

   PrivateSend = {NewName}

   Ozcar =
   {New class

	   prop
	      final

	   attr
	      MyOzcar : unit

	   meth reInit
	      case @MyOzcar == unit then skip else
		 {@MyOzcar destroy}
	      end
	      MyOzcar <- {New OzcarClass init}
	   end

	   meth on
	      {@MyOzcar on}
	   end

	   meth off
	      {@MyOzcar off}
	   end

	   meth bpAt(_ _ _)=M
	      {@MyOzcar M}
	   end

	   meth lastClickedValue(?V)
	      {@MyOzcar lastClickedValue(V)}
	   end

	   meth !PrivateSend(M)
	      {@MyOzcar M}
	   end

	end reInit}
end
