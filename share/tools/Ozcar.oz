%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

declare
Ozcar
\ifdef SAVE
NewOzcar
\endif
in

\ifdef SAVE
proc {NewOzcar Compile Error Tk TkTools Browse ?Ozcar}
\else
local
\endif   

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

in

   \insert 'ozcar/ozcar'

end

\ifdef SAVE   
Ozcar = {NewOzcar Compile Error Tk TkTools Browse}
\endif

