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
	
	meth hide
	   I T = @currentThread
	in
	   case T \= undef then         %% == undef: closed from within Ozcar
	      I = {Thread.id T}         %% \= undef: closed from within Emacs
	      ThreadManager,forget(T I)
	   else skip end
	   {Debug.off}
	   {Tk.send wm(withdraw self.toplevel)}
	   {Compile "\\sw +optimize -debuginfo"}
	end
	
	meth unhide
	   {Tk.send wm(deiconify self.toplevel)}
	   {Compile "\\sw -optimize +debuginfo"}
	   {Debug.on}
	end
	
	meth exit
	   {Debug.off}
	   ThreadManager,close
	   SourceManager,close
	   Gui,status(~1) {Delay 1500}
	   {self.toplevel tkClose}
	end
     end init}
