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
	   {Compile '\\sw -debuginfo'}
	   SourceManager,removeBar
	end
	
	meth on
	   {Tk.batch [update(idletasks)
		      wm(deiconify self.toplevel)]}
	   case {NewCompiler} then
	      case {CgetTk emacsThreads} then
		 {Compile '\\sw +debuginfo'}
	      else
		 {Compile '\\sw +debuginfovarnames +debuginfocontrol'}
	      end
	   else
	      {Compile '\\sw +debuginfo'}
	   end
	   {Debug.on}
	   case @currentThread == undef then
	      Gui,status(InitStatus)
	   else skip end
	end
	
     end init}
