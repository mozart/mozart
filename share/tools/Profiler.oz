%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

declare
Profiler
\ifdef SAVE
NewProfiler
\endif
in

\ifdef SAVE
proc {NewProfiler Compile Tk TkTools Browse ?Profiler}
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

\ifdef SAVE
Profiler = {NewProfiler Compile Tk TkTools Browse}
\endif

