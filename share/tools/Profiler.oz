%%%
%%% Authors:
%%%   Benjamin Lorenz (lorenz@ps.uni-sb.de)
%%%
%%% Contributor:
%%%   Christian Schulte
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
%%%   Christian Schulte, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor $

import
   Profile.{mode reset getInfo}
      from 'x-oz://boot/Profile'

   Property.{get}
   
   OS.{time}
   
   Tk
   
   TkTools
   
   Emacs.{getOPI
	  condSend}

export
   'Profiler': Profiler
   'object':   Profiler

body
   \insert 'profiler/prof-config'
   \insert 'profiler/prof-prelude'

   \insert 'profiler/prof-menu'
   \insert 'profiler/prof-dialog'
   \insert 'profiler/prof-help'
   \insert 'profiler/prof-gui'
   \insert 'profiler/profiler'
end
