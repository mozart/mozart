%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Contributor:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
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
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   Open(file pipe text)
   OS(system getEnv tmpnam unlink)
   Property(get condGet)
   Error(printException)
   Debug at 'x-oz://boot/Debug'
   BisonModule(generate) at 'Bison.so{native}'
export
   makeProductionTemplates: MakeProductionTemplates
   transformScanner:        TransformScanner
   transformParser:         TransformParser
define
   \insert gump/Main.oz
end
