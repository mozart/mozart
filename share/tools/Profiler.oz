%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

declare
   NewProfiler
in

proc
\ifdef NEWCOMPILER
   instantiate
\endif
   {NewProfiler
      Standard
      Compile Tk TkTools Browse ?Profiler}
\insert 'Standard.env'
   = Standard

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
