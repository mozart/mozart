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
   Error(messageToVirtualString)
   Open(file)
   Pickle(load save)
   Tk
   TkTools
   Listener('class')
   Browser(browse)
   Emacs(condSend)
export
   'class': CompilerPanel
require
   DefaultURL(pickleExt: PickleExt
	      homeUrl)
   URL(make resolve toAtom)
prepare
   BitmapUrl = {URL.toAtom {URL.resolve DefaultURL.homeUrl
			    {URL.make 'images/'}}}
define
   \insert 'compilerPanel/CompilerPanelClass.oz'
end
