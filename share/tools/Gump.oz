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

\ifdef LILO

functor $

import
   SP.{System  = 'System'
       Foreign = 'Foreign'}

   OP.{Open = 'Open'
       OS   = 'OS'}

export
   'Gump': Gump
   
body
   \insert gump/Main.oz
in
   Gump = gump(makeProductionTemplates: MakeProductionTemplates
	       transformScanner: TransformScanner
	       transformParser: TransformParser)
end

\else

fun instantiate {$ IMPORT}
   \insert 'SP.env'
   = IMPORT.'SP'
   \insert 'OP.env'
   = IMPORT.'OP'
in
   local
      \insert gump/Main.oz

      Gump = gump(makeProductionTemplates: MakeProductionTemplates
		  transformScanner: TransformScanner
		  transformParser: TransformParser)
   in
      \insert Gump.env
   end
end

\endif

