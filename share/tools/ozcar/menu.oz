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
	 {OzcarMenuBar self.toplevel self.toplevel
	  [MB(text: 'Ozcar'
	      menu:
		 [C(label:   'About...'
		    action:  self # about
		    key:     ctrl(i))
		  separator
		  C(label:   'Status'
		    action:  self # checkMe
		    key:     ctrl(s))
		  C(label:   'Reset'
		    action:  self # action(' reset')
		    key:     ctrl(r))
		  separator
		  C(label:   'Close'
		    action:  self # off
		    key:     ctrl(x))]
	      feature: ozcar)
	   MB(text: 'Thread'
	      menu:
		 [C(label:  'Previous'
		    action: self # previousThread
		    key:    'Left'
		    event:  '<Left>')
		  C(label:  'Next'
		    action: self # nextThread
		    key:    'Right'
		    event:  '<Right>')
		  separator
		  C(label:  'Step'
		    action: self # action(' step')
		    event:  s
		    key:    '  s')
		  C(label:  'Next'
		    action: self # action(' next')
		    event:  n
		    key:    '  n')
		  C(label:  'Continue'
		    action: self # action(' cont')
		    event:  c
		    key:    '  c')
		  C(label:  'Forget'
		    action: self # action(' forget')
		    event:  f
		    key:    '  f')
		  C(label:  'Terminate'
		    action: self # action(' term')
		    event:  t
		    key:    '  t')]
	      feature: thr)
	   MB(text: 'Stack'
	      menu:
		 [C(label:  'Previous Frame'
		    action: self # neighbourStackFrame(~1)
		    key:    'Up'
		    event:  '<Up>')
		  C(label:  'Next Frame'
		    action: self # neighbourStackFrame(1)
		    key:    'Down'
		    event:  '<Down>')
		  separator
		  C(label:  'Re-Calculate'
		    action: self # rebuildCurrentStack
		    key:    ctrl(l))
		  C(label:  'Browse'
		    action: self # action(' stack')
		    key:    ctrl(b))]
	      feature: stack)
	   MB(text: 'Options'
	      menu:
		 [CB(label:    'Step on All System Procedures'
		     variable: TkStepSystemProcedures
		     action:   Config # toggle(stepSystemProcedures)
		     feature:  stepSystemProcedures)
		  CC(label:    'Step on Builtin'
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
		 [C(label:   'Help on Help'
		    action:  self # help(nil))
		  separator
		   C(label:   'Thread Tree'
		    action:  self # help(TreeTitle))
		  C(label:   'Stack Control'
		    action:  self # help(StackTitle))
		  CC(label:  'Breakpoints'
		     menu:
			[C(label:  'static'
			   action: self # help(BreakpointStaticHelp))
			 C(label:  'dynamic'
			   action: self # help(BreakpointDynamicHelp))]
		     feature: breakpoints)]
	      feature: help)
	  ]}

      end
   end
end
