%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

Profiler =
{New class
	from
	   Gui
	   SourceManager

	meth init
	   Gui,init
	   SourceManager,init
	end
	
	meth off
	   {Tk.send wm(withdraw self.toplevel)}
	   {Compile "\\sw -profile"}
	   {{`Builtin` setProfileMode 1} false}
	end
	
	meth on
	   {Tk.batch [update(idletasks)
		      wm(deiconify self.toplevel)]}
	   {Compile "\\sw +profile"}
	   {{`Builtin` setProfileMode 1} true}
	   {Profile.reset}
	end
	
     end init}
