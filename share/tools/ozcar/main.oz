%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

declare

   Ozcar

local
   
   \insert pre
   \insert string
   \insert config
   %\insert emacs
   \insert source
   \insert 'thread'
   \insert stack
   \insert menu
   \insert dialog
   \insert tkext
   \insert tree
   \insert frontend

in

   \insert ozcar

   %% turn the emulator into debug mode
   %% (compiler switches are set in oz.el)

   {Debug.on}

end
