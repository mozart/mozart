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
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local

   \insert 'Transport/country.oz'

   ArgSpec = record(defaults(rightmost type:bool default:true)
                    random(rightmost type:bool default:true))

in

   functor

   import
      Tk
      TkTools
      Application
      OS
      Search(base)
      FD

   define

      Args = {Application.getCmdArgs ArgSpec}

      \insert 'Transport/configure.oz'
      \insert 'Transport/widgets.oz'
      \insert 'Transport/randomizer.oz'
      \insert 'Transport/agent.oz'
      \insert 'Transport/frontend.oz'
      \insert 'Transport/makeplan.oz'
      \insert 'Transport/contract.oz'
      \insert 'Transport/truck.oz'
      \insert 'Transport/driver.oz'
      \insert 'Transport/company.oz'
      \insert 'Transport/broker.oz'

      T = {New Tk.toplevel tkInit(title:  'Transportation'
                                  delete: Application.exit # 0)}

      F = {New Frontend init(toplevel:T)}

      if Args.defaults orelse Args.random then
         {F addDefaults}
      end

      if Args.random then
         {F random}
      end

   end

end
