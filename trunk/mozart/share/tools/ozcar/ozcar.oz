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
	   {Compile "\\sw +optimize -debuginfo"}
	end
	
	meth on
	   {Tk.send wm(deiconify self.toplevel)}
	   {Compile "\\sw -optimize +debuginfo"}
	   {Debug.on}
	   Gui,rawStatus(InitStatus)
	end
	
     end init}
