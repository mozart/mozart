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
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
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
	 if {self.emacsThreadsMenu getCurrent($)} == AttachText then
	    {EnqueueCompilerQuery setSwitch(debuginfo true)}
	 else
	    {EnqueueCompilerQuery setSwitch(controlflowinfo true)}
	    {EnqueueCompilerQuery setSwitch(staticvarnames true)}
	 end
	 {Dbg.on}
	 if @currentThread == unit then
	    Gui,status('Ready to attach threads')
	 end
      end

      meth off
	 case {Cget closeAction} of unit then
	    {Dbg.off}
	    {Tk.send wm(withdraw self.toplevel)}
	    {EnqueueCompilerQuery setSwitch(debuginfo false)}
	    {SendEmacs removeBar}
	 elseof P then {P}
	 end
      end

      meth conf(...)=M
	 {Record.forAllInd M
	  proc {$ F V}
	     if {Config confAllowed(F $)} then
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
		 if @MyOzcar \= unit then
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
