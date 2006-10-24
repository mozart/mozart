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

Profiler =
{New class
	from
	   Gui

	meth init
	   Gui,init
	end

	meth off
	   case {Cget closeAction} of unit then
	      Time.repeat,stop
	      {Tk.send wm(withdraw self.toplevel)}
	      {EnqueueCompilerQuery setSwitch(profile false)}
	      {Profile.mode false}
	   elseof P then {P}
	   end
	end

	meth on
	   {Tk.batch [update(idletasks)
		      wm(deiconify self.toplevel)]}
	   {EnqueueCompilerQuery setSwitch(profile true)}
	   {Profile.mode true}
	   {Profile.reset}
	   if {Cget update} > 0 then
	      Time.repeat,go
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

     end init}
