%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

declare
   NewOzcar
in

fun
\ifdef NEWCOMPILER
   instantiate
\endif
   {NewOzcar IMPORT}
   \insert 'Standard.env'
      = IMPORT.'Standard'
   \insert 'WP.env'
      = IMPORT.'WP'
   \insert 'Browser.env'
      = IMPORT.'Browser'
   \insert 'Emacs.env'
      = IMPORT.'Emacs'

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

   \insert 'ozcar/ozcar'
in
   \insert 'Ozcar.env'
end



