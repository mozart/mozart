%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
%%%   Christian Schulte, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local

   \insert 'job-shop/configure.oz'

   \insert 'job-shop/examples.oz'

   ArgSpec = single(example(type:atom default:no))

in

   functor

   import
      FD
      Schedule
      Explorer
      Tk
      TkTools
      Search
      Application

   define

      Argv = {Application.getCmdArgs ArgSpec}

      \insert 'job-shop/main.oz'

   end

end
