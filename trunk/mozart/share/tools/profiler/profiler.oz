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
\ifndef NEWCOMPILER
	   {Compile "\\sw -profile"}
\else
	   {`Compiler` feedVirtualString('\\switch -profile\n')}
\endif
	   {{`Builtin` setProfileMode 1} false}
	end
	
	meth on
	   {Tk.batch [update(idletasks)
		      wm(deiconify self.toplevel)]}
\ifndef NEWCOMPILER
	   {Compile "\\sw +profile"}
\else
	   {`Compiler` feedVirtualString('\\switch +profile\n')}
\endif
	   {{`Builtin` setProfileMode 1} true}
	   {Profile.reset}
	end
	
     end init}
