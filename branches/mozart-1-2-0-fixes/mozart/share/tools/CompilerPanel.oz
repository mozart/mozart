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
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
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
   Property(get)
   Tk
   TkTools
   Listener('class')
   Inspector(inspect)
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
