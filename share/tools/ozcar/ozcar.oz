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

	meth reinit
	   ThreadManager,reinit
	end

	meth off
	   {Dbg.off}
	   {Tk.send wm(withdraw self.toplevel)}
	   {Compile '\\switch -debuginfo'}
	   SourceManager,removeBar
	end

	meth on
	   {Tk.batch [update(idletasks)
		      wm(deiconify self.toplevel)]}
	   case {NewCompiler} then
	      case {CgetTk emacsThreads} then
		 {Compile '\\switch +debuginfo'}
	      else
		 {Compile '\\switch +debuginfovarnames +debuginfocontrol'}
	      end
	   else
	      {Compile '\\switch +debuginfo'}
	   end
	   {Dbg.on}
	   case @currentThread == undef then
	      Gui,status(InitStatus)
	   else skip end
	end

     end init}
