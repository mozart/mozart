%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

declare

   Ozcar

local
   
   \insert pre
   \insert config
   \insert emacs
   \insert thr
   \insert manager
   \insert dialog
   \insert tkext
   \insert frontend

in

   \insert ozcar

   %% turn the emulator into debug mode
   %% (compiler switches are set in oz.el)

   {Debug.on}

end
