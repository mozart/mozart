%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Contributor:
%%%   Christian Schulte <schulte@dfki.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1997
%%%   Christian Schulte, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%   http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor prop once
import
   Foreign.load
   Error.{formatGeneric format dispatch}
   ErrorRegistry.put
   Finalize.register
export
   'GumpScanner': GumpScanner
   'class':       GumpScanner
body
   \insert gump/GumpScannerClass
end
