%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

declare

   Ozcar

local
   
   \insert 'ozcar-prelude'
   \insert 'config'

   \insert 'tree'
   \insert 'thread'

   \insert 'source'

   \insert 'menu'
   \insert 'dialog'
   \insert 'gui'

in

   \insert 'ozcar'
   %% turn the emulator into debug mode
   %% compiler switches are set in oz.el
   {Debug.on}

end
