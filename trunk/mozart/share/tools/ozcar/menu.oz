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
	 {MyMenuBar self.toplevel self.toplevel
	  [MB(text: IconName
	      menu:
		 [C(label:   'About...'
		    action:  self # about
		    key:     ctrl(i))
		  separator
		  C(label:   'Remove All Threads'
		    action:  self # action(RemoveAllAction)
		    key:     ctrl(r))
		  C(label:   'Remove Dead Threads'
		    action:  self # action(RemoveAllDeadAction)
		    key:     ctrl(u))
		  separator
		  C(label:   'Suspend Debugging'
		    action:  self # off
		    key:     ctrl(x))
		  C(label:   'Destroy Ozcar'
		    action:  Ozcar # reInit
		    key:     ctrl(d))])
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
		  C(label:  'Step Into'
		    action: self # action(StepButtonBitmap)
		    key:    s)
		  C(label:  'Step Over'
		    action: self # action(NextButtonBitmap)
		    key:    n)
		  separator
		  C(label:  'Unleash'
		    action: self # action(UnleashButtonBitmap)
		    key:    c)
		  C(label:  'Stop'
		    action: self # action(StopButtonBitmap)
		    key:    z)
		  separator
		  C(label:  'Forget'
		    action: self # action(ForgetButtonBitmap)
		    key:    f)
		  C(label:  'Terminate'
		    action: self # action(TermButtonBitmap)
		    key:    t)
		  separator
		  C(label:   'Status'
		    action:  self # checkMe
		    key:     ctrl(s))])
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
		    action: self # action(StackAction)
		    key:    ctrl(b))
		  separator
		  C(label:  'Evaluate...'
		    action: self # eval
		    key:    ctrl(e))])
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
		     feature:  verbose)])]
	  [MB(text: 'Help'
	      menu:
		 [C(label:   'Help on Help'
		    action:  self # help(nil))
		  separator
		  CC(label:  'Breakpoints'
		     menu:
			[C(label:  'static'
			   action: self # help(BreakpointStaticHelp))
			 C(label:  'dynamic'
			   action: self # help(BreakpointDynamicHelp))]
		     feature: breakpoints)])]}
      end
   end
end
