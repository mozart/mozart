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

functor
import
   Open(file pipe text)
   OS(system getEnv)
   Property(get condGet)
   Debug at 'x-oz://boot/Debug'
   BisonModule(generate) at 'Bison.so{native}'
export
   makeProductionTemplates: MakeProductionTemplates
   transformScanner:        TransformScanner
   transformParser:         TransformParser
define
   \insert gump/Main.oz
end
