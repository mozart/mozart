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

   \insert 'transport/country.oz'

   ArgSpec = single(defaults(type:bool default:true)
		    random(type:bool default:true))

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
