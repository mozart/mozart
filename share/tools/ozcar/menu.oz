%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local
   
   TkVerbose             = {New Tk.variable tkInit(ConfigVerbose)}
   TkStepSystemProcedures= {New Tk.variable tkInit(ConfigStepSystemProcedures)}
   TkEnvSystemVariables  = {New Tk.variable tkInit(ConfigEnvSystemVariables)}
   TkEnvProcedures       = {New Tk.variable tkInit(ConfigEnvProcedures)}
   
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
		     variable: TkStepSystemProcedures
		     action:   Config # toggle(stepSystemProcedures)
		     feature:  stepSystemProcedures)
		  separator
		  CB(label:   'Filter System Variables'
		     variable: TkEnvSystemVariables
		     action:   Config # toggle(envSystemVariables)
		     feature:  envSystemVariables)
		  CB(label:   'Filter Procedures'
		     variable: TkEnvProcedures
		     action:   Config # toggle(envProcedures)
		     feature:  envProcedures)
		  separator
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
	 {self.menuBar tk(conf borderwidth:2)}
      end
   end
end
