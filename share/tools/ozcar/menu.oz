%%%
%%% Author:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.

local

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
	 {TkTools.menubar self.toplevel self.toplevel
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
		  C(label:  'Detach'
		    action: self # action(DetachButtonBitmap)
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
		 [CC(label:   'Value Printing'
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
		  CB(label:    'Use Emacs'
		     variable: TkUseEmacsBar
		     action:   self # toggleEmacs
		     key:      ctrl(e))
		  CB(label:    'Env Auto Update'
		     variable: TkUpdateEnv
		     action:   self # toggleUpdateEnv
		     key:      ctrl(a))
		  separator
		  C(label:    'Other Settings...'
		    action:   self # settings
		    key:      ctrl(o))])]
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
