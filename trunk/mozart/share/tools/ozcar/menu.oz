%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local
   
   TkVerbose             = {New Tk.variable tkInit(ConfigVerbose)}
   
   TkStepSystemProcedures= {New Tk.variable tkInit(ConfigStepSystemProcedures)}
   TkStepRecordBuiltin   = {New Tk.variable tkInit(ConfigStepRecordBuiltin)}
   TkStepDotBuiltin      = {New Tk.variable tkInit(ConfigStepDotBuiltin)}
   TkStepWidthBuiltin    = {New Tk.variable tkInit(ConfigStepWidthBuiltin)}
   TkStepNewNameBuiltin  = {New Tk.variable tkInit(ConfigStepNewNameBuiltin)}
   TkStepSetSelfBuiltin  = {New Tk.variable tkInit(ConfigStepSetSelfBuiltin)}
   
   TkEnvSystemVariables  = {New Tk.variable tkInit(ConfigEnvSystemVariables)}
   TkEnvProcedures       = {New Tk.variable tkInit(ConfigEnvProcedures)}
   
   C  = command
   MB = menubutton
   CB = checkbutton
   CC = cascade
   
in
   
   class Menu
      meth init
	 self.menuBar = 
	 {TkTools.menubar self.toplevel self.toplevel
	  [MB(text: 'File'
	      menu:
		 [C(label:   'Quit'
		    action:  self # off
		    key:     ctrl(c)
		    feature: quit)]
	      feature: file)
	   /*
	   MB(text: 'Thread'
	      menu:
		 [separator]
	      feature: 'thread')
	   */
	   MB(text: 'Options'
	      menu:
		 [CB(label:    'Step on All System Procedures'
		     variable: TkStepSystemProcedures
		     action:   Config # toggle(stepSystemProcedures)
		     feature:  stepSystemProcedures)
		  CC(label:    'Step on Builtin...'
		     menu:
			[CB(label:    '\'record\''
			    variable: TkStepRecordBuiltin
			    action:   Config # toggle(stepRecordBuiltin)
			    feature:  stepRecordBuiltin)
			 CB(label:    '\'.\''
			    variable: TkStepDotBuiltin
			    action:   Config # toggle(stepDotBuiltin)
			    feature:  stepDotBuiltin)
			 CB(label:    '\'width\''
			    variable: TkStepWidthBuiltin
			    action:   Config # toggle(stepWidthBuiltin)
			    feature:  stepWidthBuiltin)
			 CB(label:    '\'NewName\''
			    variable: TkStepNewNameBuiltin
			    action:   Config # toggle(stepNewNameBuiltin)
			    feature:  stepNewNameBuiltin)
			 CB(label:    '\'setSelf\''
			    variable: TkStepSetSelfBuiltin
			    action:   Config # toggle(stepSetSelfBuiltin)
			    feature:  stepSetSelfBuiltin)]
		     feature:  stepOnBuiltin)
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
		  CB(label:   'Messages in Emulator Buffer'
		     variable: TkVerbose
		     action:   Config # toggle(verbose)
		     feature:  verbose)]
	      feature: options)]
	  [MB(text: 'Help'
	      menu:
		 [C(label:   'About...'
		    action:  self # about
		    feature: about)]
	      feature: help)
	  ]}
%	 {self.menuBar.help tk(conf tearoff:false)}
	 %{self.menuBar tk(conf borderwidth:2)}
      end
   end
end
