%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

declare
\ifdef NEWSAVE
Profiler
\endif
\ifdef SAVE
NewProfiler
\endif
in

\ifdef SAVE
proc {NewProfiler
\ifdef NEWSAVE
      Standard
\endif
      Compile Tk TkTools Browse ?Profiler}
\ifdef NEWSAVE
\insert 'Standard.env'
   = Standard
\endif
\else
local
\endif   

   \insert 'profiler/prof-config'
   \insert 'profiler/prof-prelude'

   \insert 'profiler/prof-source'
   \insert 'profiler/prof-menu'
   \insert 'profiler/prof-dialog'
   \insert 'profiler/prof-help'
   \insert 'profiler/prof-gui'

in

   \insert 'profiler/profiler'

end
