%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
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

local

   \insert 'transport/country.oz'

   fun {MakeTransport IMPORT}
      \insert 'CP.env'
      = IMPORT.'CP'
      \insert 'WP.env'
      = IMPORT.'WP'
      \insert 'OP.env'
      = IMPORT.'OP'

      \insert 'transport/configure.oz'
      \insert 'transport/widgets.oz'
      \insert 'transport/randomizer.oz'
      \insert 'transport/agent.oz'
      \insert 'transport/frontend.oz'
      \insert 'transport/makeplan.oz'
      \insert 'transport/contract.oz'
      \insert 'transport/truck.oz'
      \insert 'transport/driver.oz'
      \insert 'transport/company.oz'
      \insert 'transport/broker.oz'

   in

      proc {$ Toplevel Argv}
         F = {New Frontend init(toplevel:Toplevel)}
      in
         case Argv.defaults orelse Argv.random then
            {F addDefaults}
         else skip
         end
         case Argv.random then
            {F random}
         else skip
         end
      end

   end

in

   {Application.applet
    'transport.oza'

    c('CP': eager
      'WP': eager
      'OP': eager)

    MakeTransport

    single(defaults(type:bool default:true)
           random(type:bool default:true)
           title(type:string default:"Transportation"))
   }

end
