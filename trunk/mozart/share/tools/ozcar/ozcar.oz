%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

Ozcar =
{New class
	from
	   ThreadManager
	   Gui
	   SourceManager

	meth init
	   ThreadManager,init
	   Gui,          init
	   SourceManager,init
	end
	
	meth off
	   {Debug.off}
	   {Tk.send wm(withdraw self.toplevel)}
	   SourceManager,removeBar
\ifndef NEWCOMPILER
	   {Compile "\\sw -debuginfo"}
\else
	   {`Compiler` feedVirtualString('\\switch -debuginfo\n')}
\endif
	end
	
	meth on
	   {Tk.batch [update(idletasks)
		      wm(deiconify self.toplevel)]}
\ifndef NEWCOMPILER
	   {Compile "\\sw +debuginfo"}
\else
	   {`Compiler` feedVirtualString('\\switch +debuginfo\n')}
\endif
	   {Debug.on}
	   case @currentThread == undef then
	      Gui,status(InitStatus)
	   else skip end
	end
	
     end init}
