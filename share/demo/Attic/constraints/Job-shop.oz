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
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local

   functor MakeJobShop prop once

   import
      FD

      Explorer

      Tk

      TkTools

      Applet.{Argv   = args
              toplevel}

      Search

   body

      \insert 'job-shop/main.oz'

   end

in

   {Application.applet
    'job-shop.oza'

    MakeJobShop

    single(example(type:atom default:no)
           title(type:string default:"Job Shop Scheduler"))
   }

end
