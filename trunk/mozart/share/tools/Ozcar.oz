%%%
%%% Authors:
%%%   Benjamin Lorenz (lorenz@ps.uni-sb.de)
%%%
%%% Contributor:
%%%   Christian Schulte
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
%%%   Christian Schulte, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
require
   DefaultURL(homeUrl)
   URL(make resolve toAtom)
prepare
   BitmapUrl = {URL.toAtom {URL.resolve DefaultURL.homeUrl
			    {URL.make 'images/ozcar/'}}}
import
   Browser(browse)
   Debug at 'x-oz://boot/Debug'
   Emacs(getOPI condSend)
   Error(exceptionToMessage extendedVSToVS printException)
   FD
   FS
   Property
   System
   Tk
   TkTools
   BaseEvalDialog at 'x-oz://system/EvalDialog'
   
export
   'object':     Ozcar
   'open':       OpenOzcar
   'close':      CloseOzcar
   'breakpoint': Breakpoint
define
   \insert 'ozcar/config'
   \insert 'ozcar/prelude'

   \insert 'ozcar/tree'
   \insert 'ozcar/thread'
   \insert 'ozcar/stack'

   \insert 'ozcar/source'

   \insert 'ozcar/menu'
   \insert 'ozcar/dialog'
   \insert 'ozcar/help'
   \insert 'ozcar/gui'

   \insert 'ozcar/ozcar'

   proc {OpenOzcar}
      {Ozcar on}
   end

   proc {CloseOzcar}
      {Ozcar off}
   end

   Breakpoint = Debug.breakpoint
end
