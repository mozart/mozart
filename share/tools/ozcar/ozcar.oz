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
	 case {Cget emacsThreads} then
	    {EnqueueCompilerQuery setSwitch(debuginfo true)}
	 else
	    {EnqueueCompilerQuery setSwitch(debuginfovarnames true)}
	    {EnqueueCompilerQuery setSwitch(debuginfocontrol true)}
	 end
	 {Dbg.on}
	 case @currentThread == unit then
	    Gui,status(TitleName # ' initialized')
	 else skip end
      end

      meth off
	 {Dbg.off}
	 {Tk.send wm(withdraw self.toplevel)}
	 {EnqueueCompilerQuery setSwitch(debuginfo false)}
	 {Emacs removeBar}
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
