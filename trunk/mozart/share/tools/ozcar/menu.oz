%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local
   
   TkVerbose          = {New Tk.variable tkInit(ConfigVerbose)}
   TkSystemProcedures = {New Tk.variable tkInit(ConfigSystemProcedures)}

   C  = command
   MB = menubutton
   CB = checkbutton
   
in
   
   class Menu
      meth init
	 self.menuBar = 
	 {TkTools.menubar self.toplevel self.toplevel
	  [MB(text: 'Ozcar'
	      menu:
		 [C(label:   'About...'
		    action:  self # about
		    feature: about)]
	      feature: ozcar)
	   MB(text: 'File'
	      menu:
		 [C(label:   'Quit'
		    action:  self # exit
		    key:     ctrl(c)
		    feature: quit)]
	      feature: file)
	   MB(text: 'Thread'
	      menu:
		 [separator]
	      feature: 'thread')
	   MB(text: 'Options'
	      menu:
		 [CB(label:    'Step on all System Procedures'
		     variable: TkSystemProcedures
		     action:   Config # toggle(systemProcedures)
		     feature:  systemProcedures)
		  CB(label:   'Messages in Emulator buffer'
		     variable: TkVerbose
		     action:   Config # toggle(verbose)
		     feature:  verbose)]
	      feature: options)]
	  [MB(text: 'Help'
	      menu:
		 [separator]
	      feature: help)
	  ]}
      end
   end
end
