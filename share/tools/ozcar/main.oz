%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>


%% turn the compiler/emulator into debug mode

\sw -optimize +debuginfo

{Debug.on}

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

end
