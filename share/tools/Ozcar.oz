%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

declare
\ifndef NEWSAVE
Ozcar
\endif
\ifdef SAVE
NewOzcar
\endif
in

\ifdef SAVE
proc {NewOzcar
\ifdef NEWSAVE
      Standard
\endif
      Compile Error Tk TkTools Browse ?Ozcar}
\ifdef NEWSAVE
\insert 'Standard.env'
   = Standard
\endif
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
