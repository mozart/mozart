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

   functor MakeJobShop prop once

   import
      FD

      Schedule
      
      Explorer

      Tk

      TkTools

      Applet.{Argv   = args
	      toplevel spec}

      Search
      
   body
      Applet.spec = single(example(type:atom default:no)
			   title(type:string default:"Job Shop Scheduler"))
      
      \insert 'job-shop/main.oz'
      
   end

in
    
    MakeJobShop

end


 




