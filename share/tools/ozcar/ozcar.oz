%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

Ozcar =
{New class
	from
	   ThreadManager
	   SourceManager
	   Gui
	
	meth init
	   ThreadManager,init
	   SourceManager,init
	   Gui,          init
	end
	
	meth exit
	   {Debug.off}
	   ThreadManager,close
	   SourceManager,close
	   Gui,status("See you again...")  {Delay 900}
	   {self.toplevel tkClose}
	end
     end init}
