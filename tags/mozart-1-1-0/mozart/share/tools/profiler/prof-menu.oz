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

   TkEmacs = {New Tk.variable tkInit(ConfigEmacs)}

   C  = command
   MB = menubutton
   CB = checkbutton

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
