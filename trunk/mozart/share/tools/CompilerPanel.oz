%%%
%%% Authors:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   System(show)
   Error(formatLine msg)
   Open(file)
   Pickle(load save)
   Tk
   TkTools
   Listener('class')
   Browser(browse)
   Emacs(condSend)
export
   'class': CompilerPanel
define
   local
      UrlDefaults = \insert '../url-defaults.oz'
   in
      \insert compilerPanel/CompilerPanelClass
   end
end
