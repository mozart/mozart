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
	   {Compile "\\switch -debuginfo"}
	   SourceManager,removeBar
	end
	
	meth on
	   {Tk.batch [update(idletasks)
		      wm(deiconify self.toplevel)]}
	   {Compile "\\switch +debuginfo"}
	   {Debug.on}
	   case @currentThread == undef then
	      Gui,status(InitStatus)
	   else skip end
	end
	
     end init}
