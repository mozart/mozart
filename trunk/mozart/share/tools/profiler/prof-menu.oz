%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local
   
   TkEmacs = {New Tk.variable tkInit(ConfigEmacs)}
   
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
	   MB(text: 'Action'
	      menu:
		 [C(label:   'Update'
		    action:  self # action(UpdateButtonText)
		    key:     u)
		  C(label:   'Reset'
		    action:  self # action(ResetButtonText)
		    key:     r)])
	   MB(text: 'Options'
	      menu:
		 [CB(label:   'Use Emacs'
		     variable: TkEmacs
		     action:   self # toggleEmacs
		     key:      e)
		  C(label:    'Automatic Update...'
		    action:   self # configureUpdate
		    key:      a)])]
	  [MB(text: 'Help'
	      menu:
		 [C(label:   'Help on Help'
		    action:  self # help(nil))])]}
      end
   end
end
