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
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.

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
	 case {self.emacsThreadsMenu getCurrent($)} == AttachText then
	    {EnqueueCompilerQuery setSwitch(debuginfo true)}
	 else
	    {EnqueueCompilerQuery setSwitch(debuginfovarnames true)}
	    {EnqueueCompilerQuery setSwitch(debuginfocontrol true)}
	 end
	 {Dbg.on}
	 case @currentThread == unit then
	    Gui,status('Ready to attach threads')
	 else skip end
      end

      meth off
	 {Dbg.off}
	 {Tk.send wm(withdraw self.toplevel)}
	 {EnqueueCompilerQuery setSwitch(debuginfo false)}
	 {Emacs.condSend.interface removeBar}
      end

      meth conf(...)=M
	 {Record.forAllInd M
	  proc {$ F V}
	     case {Config confAllowed(F $)} then
		{Config set(F V)}
	     else
		raise ozcar(badConfigFeature(F)) end
	     end
	  end}
      end
   end

in

   PrivateSend = {NewName}

   Ozcar =
   {New class

	   prop
	      locking
	      final

	   attr
	      MyOzcar : unit

	   meth init
	      skip
	   end

	   meth reInit
	      lock
		 case @MyOzcar == unit then skip else
		    {@MyOzcar destroy}
		 end
		 MyOzcar <- {New OzcarClass init}
	      end
	   end

	   meth on
	      {@MyOzcar on}
	   end

	   meth off
	      {@MyOzcar off}
	   end

	   meth conf(...)=M
	      {@MyOzcar M}
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

	end init}

   {Ozcar reInit}
end
