%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

declare
   NewProfiler
in

fun
\ifdef NEWCOMPILER
   instantiate
\endif
   {NewProfiler IMPORT}
   \insert 'Standard.env'
      = IMPORT.'Standard'
   \insert 'WP.env'
      = IMPORT.'WP'
   \insert 'Browser.env'
      = IMPORT.'Browser'
   \insert 'Emacs.env'
      = IMPORT.'Emacs'

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
