%%%
%%% Authors:
%%%   Author's name (Author's email address)
%%%
%%% Contributors:
%%%   optional, Contributor's name (Contributor's email address)
%%%
%%% Copyright:
%%%   Organization or Person (Year(s))
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
%%%
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
