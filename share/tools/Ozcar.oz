%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

declare
   NewOzcar
in

proc
\ifdef NEWCOMPILER
   instantiate
\endif
   {NewOzcar
      Standard
      Compile Error Tk TkTools Browse ?Ozcar}
\insert 'Standard.env'
   = Standard

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



