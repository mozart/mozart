%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local
   
   TkVerbose = {New Tk.variable tkInit(ConfigVerbose)}
   
   C  = command
   MB = menubutton
   CB = checkbutton
   CC = cascade
   
in
   
   class Menu
      meth init
	 self.menuBar = 
	 {MyMenuBar self.toplevel self.toplevel
	  [MB(text: IconName
	      menu:
		 [C(label:   'About...'
		    action:  self # about
		    key:     ctrl(i))
		  separator
		  C(label:   'Close'
		    action:  self # off
		    key:     ctrl(x))])
	   MB(text: 'Options'
	      menu:
		 [CB(label:   'Verbose'
		     variable: TkVerbose
		     action:   Config # toggle(verbose)
		     feature:  verbose)])]
	  [MB(text: 'Help'
	      menu:
		 [C(label:   'Help on Help'
		    action:  self # help(nil))])]}
      end
   end
end
