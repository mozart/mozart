%%%
%%% Authors:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Contributors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
%%%   Christian Schulte, 1998
%%%   Leif Kornstaedt, 2001
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
%%%

functor
require
   DefaultURL(homeUrl)
   URL(make resolve toAtom)
prepare
   BitmapUrl = {URL.toAtom {URL.resolve DefaultURL.homeUrl
			    {URL.make 'images/ozcar/'}}}
import
   BootName(newUnique: NewUniqueName) at 'x-oz://boot/Name'
   Debug(breakpoint getId getParentId) at 'x-oz://boot/Debug'
   Inspector(inspect: Inspect)
   Server at 'OzcarServer.ozf'
   Emacs(getOPI condSend)
   Error(exceptionToMessage extendedVSToVS printException registerFormatter)
   Property
   System
   Tk
   TkTools
   BaseEvalDialog(dialog) at 'x-oz://system/EvalDialog'
   OPIEnv(full)
export
   'object': Ozcar
   'open':   OpenOzcar
   'close':  CloseOzcar
   Breakpoint
   StartServer
define
   \insert 'ozcar/config'
   \insert 'ozcar/prelude'
   \insert 'ozcar/error'

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

   Breakpoint  = Debug.breakpoint
   StartServer = Server.start
end
