%%%
%%% Authors:
%%%   Benjamin Lorenz (lorenz@ps.uni-sb.de)
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
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

fun instantiate {$ IMPORT}
   \insert 'SP.env'
      = IMPORT.'SP'
   \insert 'OP.env'
      = IMPORT.'OP'
   \insert 'WP.env'
      = IMPORT.'WP'
   \insert 'Browser.env'
      = IMPORT.'Browser'
   \insert 'Emacs.env'
      = IMPORT.'Emacs'
   \insert 'Compiler.env'
      = IMPORT.'Compiler'

   \insert 'profiler/prof-config'
   \insert 'profiler/prof-prelude'

   \insert 'profiler/prof-menu'
   \insert 'profiler/prof-dialog'
   \insert 'profiler/prof-help'
   \insert 'profiler/prof-gui'

   \insert 'profiler/profiler'

in

   \insert 'Profiler.env'

end
