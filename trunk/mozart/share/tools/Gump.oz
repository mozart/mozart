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
%%%   $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%   $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor $

import
   Open.{file}

   OS.{system}

   System.{get
	   valueToVirtualString}

   Error.{formatGeneric
	  format
	  dispatch}

   ErrorRegistry.{put}
   
   Foreign.{load}

export
   makeProductionTemplates: MakeProductionTemplates
   transformScanner:        TransformScanner
   transformParser:         TransformParser
   
body
   \insert gump/Main.oz
   \insert gump/Errors.oz
end
