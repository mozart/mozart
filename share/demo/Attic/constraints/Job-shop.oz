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

   fun {MakeJobShop IMPORT}
      \insert 'CP.env'
      = IMPORT.'CP'
      \insert 'WP.env'
      = IMPORT.'WP'
      \insert 'Explorer.env'
      = IMPORT.'Explorer'
   in

      \insert 'job-shop/main.oz'

   end

in

   {Application.applet
    'job-shop.oza'

    c('CP':       eager
      'WP':       eager
      'Explorer': lazy)

    MakeJobShop

    single(example(type:atom default:no)
           title(type:string default:"Job Shop Scheduler"))
   }

end
