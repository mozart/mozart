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
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
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
		 (C(label:   'About...'
		    action:  self # about)|
		  separator|
		  if {Property.get 'oz.standalone'} then
		     [C(label:   'Close'
			action:  self # off
			key:     ctrl(x))]
		  else
		     [C(label:   'Destroy'
			action:  Ozcar # reInit)
		      C(label:   'Suspend'
			action:  self # off
			key:     ctrl(x))]
		  end))
	   MB(text: 'Action'
	      menu:
		 [C(label:  'Step Into'
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
		  CC(label: 'Detach'
		     menu:
			[C(label:  'Current'
			   action: self # action(DetachButtonBitmap)
			   key:    d)
			 C(label:  'All But Current'
			   action: self # action(DetachAllButCurAction)
			   key:    ctrl(d))
			 C(label:  'All'
			   action: self # action(DetachAllAction)
			   key:    meta(d))
			 C(label:  'All Dead'
			   action: self # action(DetachAllDeadAction)
			   key:    meta(u))])
		  CC(label: 'Terminate'
		     menu:
			[C(label:  'Current'
			   action: self # action(TermButtonBitmap)
			   key:    t)
			 C(label:  'All But Current'
			   action: self # action(TermAllButCurAction)
			   key:    ctrl(t))
			 C(label:  'All'
			   action: self # action(TermAllAction)
			   key:    meta(t))])])
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
		  C(label:  'Update Env'
		    action: self # updateEnv
		    key:    v)
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
			    key:      'less'
			    action:   Config # set(envPrintTypes true))
			 RB(label:    'Complete'
			    variable: TkEnvPrintTypes
			    value:    false
			    key:      'greater'
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
		  C(label:    'Preferences...'
		    action:   self # settings
		    key:      ctrl(o))])]
	  [MB(text: 'Help'
	      menu:
		 [C(label:   'Help on Help'
		    action:  self # help(nil))
		  separator
		  C(label:   'Access to Values'
		    action:  self # help(ValuesHelp))
		  CC(label:  'Breakpoints'
		     menu:
			[C(label:  'Static'
			   action: self # help(BreakpointStaticHelp))
			 C(label:  'Dynamic'
			   action: self # help(BreakpointDynamicHelp))]
		     feature: breakpoints)])]}
      end
   end
end
