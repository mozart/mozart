%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
%%%   Christian Schulte, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

fun
\ifdef NEWCOMPILER
   instantiate
\endif
   {$ IMPORT}
   \insert 'SP.env'
       = IMPORT.'SP'
   \insert 'OP.env'
       = IMPORT.'OP'
   \insert 'AP.env'
   = IMPORT.'AP'
   \insert 'WP.env'
       = IMPORT.'WP'
in
   local
      \insert DP/Site
   in
      \insert 'DP.env'
   end
end
