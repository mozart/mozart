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
