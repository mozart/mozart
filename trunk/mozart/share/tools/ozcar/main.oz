%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>


%% turn the compiler/emulator into debug mode

\sw -optimize

{Debug.on}

declare

   Ozcar

local
   
   \insert pre   
   \insert config
   \insert emacs
   \insert thr
   \insert manager

in

   \insert ozcar

end
