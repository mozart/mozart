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
	   
	   %% still searching for the gc bug... :-(
	   %% found it (Tue Mar 11 1997)
	   %{System.set gc(on:false)}
	   
	   {Debug.on}
	   Gui,rawStatus(TitleName # ' initialized')
	end
	
	meth exit
	   {Debug.off}
	   ThreadManager,close
	   SourceManager,close
	   Gui,status(~1) {Delay 1500}
	   {self.toplevel tkClose}
	end
     end init}
