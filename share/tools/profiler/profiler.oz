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

Profiler =
{New class
	from
	   Gui

	meth init
	   Gui,init
	end

	meth off
	   Time.repeat,stop
	   {Tk.send wm(withdraw self.toplevel)}
	   {Compile "\\switch -profile"}
	   {Profile.mode false}
	end

	meth on
	   {Tk.batch [update(idletasks)
		      wm(deiconify self.toplevel)]}
	   {Compile "\\switch +profile"}
	   {Profile.mode true}
	   {Profile.reset}
	   case {Cget update} > 0 then
	      Time.repeat,go
	   else skip end
	end

     end init}
