%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   TkVerbose             = {New Tk.variable tkInit(ConfigVerbose)}

   TkStepDotBuiltin      = {New Tk.variable tkInit(ConfigStepDotBuiltin)}
   TkStepNewNameBuiltin  = {New Tk.variable tkInit(ConfigStepNewNameBuiltin)}

   TkEnvSystemVariables  = {New Tk.variable tkInit(ConfigEnvSystemVariables)}
   TkEnvPrintTypes       = {New Tk.variable tkInit(ConfigEnvPrintTypes)}

   TkUpdateEnv           = {New Tk.variable tkInit(ConfigUpdateEnv)}
   TkUseEmacsBar         = {New Tk.variable tkInit(ConfigUseEmacsBar)}

   C  = command
   MB = menubutton
   CB = checkbutton
   RB = radiobutton
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
		  C(label:   'Destroy Ozcar'
		    action:  Ozcar # reInit
		    key:     ctrl(d))
		  C(label:   'Suspend Debugging'
		    action:  self # off
		    key:     ctrl(x))])
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
		  C(label:  'Update Environment'
		    action: self # updateEnv
		    key:    v)
		  C(label:  'Browse'
		    action: self # action(StackAction)
		    key:    ctrl(b))
		  separator
		  C(label:  'Query...'
		    action: self # eval
		    key:    e)])
	   MB(text: 'Options'
	      menu:
		 [CC(label: 'Step on Builtin'
		     menu:
			[CB(label:    '\'.\''
			    variable: TkStepDotBuiltin
			    action:   Config # toggle(stepDotBuiltin))
			 CB(label:    '\'NewName\''
			    variable: TkStepNewNameBuiltin
			    action:   Config # toggle(stepNewNameBuiltin))])
		  CC(label:   'Value Printing'
		     menu:
			[RB(label:    'Types Only'
			    variable: TkEnvPrintTypes
			    value:    true
			    action:   Config # set(envPrintTypes true))
			 RB(label:    'Complete'
			    variable: TkEnvPrintTypes
			    value:    false
			    action:   Config # set(envPrintTypes false))])
		  separator
		  CB(label:    'Show System Variables'
		     variable: TkEnvSystemVariables
		     action:   Config # toggle(envSystemVariables))
		  CB(label:    'Auto Update of Environment'
		     variable: TkUpdateEnv
		     action:   self # toggleUpdateEnv
		     key:      ctrl(a))
		  CB(label:    'Use Emacs'
		     variable: TkUseEmacsBar
		     action:   self # toggleEmacs
		     key:      ctrl(e))
		  separator
		  CB(label:   'Debug Debugger'
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
