%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%   Christian Schulte <schulte@dfki.de>
%%%
%%% Contributors:
%%%   Denys Duchier <duchier@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1997
%%%   Christian Schulte, 1998
%%%   Denys Duchier, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   Module(manager)
export
   full: Env
require
   DefaultURL(functorNames: Modules)
prepare
   ShortCuts = [%% Library
		'Pickle'('Load': [load]
			 'Save': [save])

		'Search'('SearchOne':  [base one]
			 'SearchAll':  [base all]
			 'SearchBest': [base best])

		'System'('Show':  [show]
			 'Print': [print])

		'Module'('Link':  [link]
			 'Apply': [apply])

		%% Tools
		'Browser'('Browse': [browse])

		'Explorer'('ExploreOne':  [one]
			   'ExploreAll':  [all]
			   'ExploreBest': [best])
\ifdef INSPECTOR
		'Inspector'('Inspect': [inspect])
\endif
	       ]

   fun {Dots M Fs}
      case Fs of nil then M
      [] F|Fr then {Dots M.F Fr}
      end
   end
define
   ModMan = {New Module.manager init}

   %% Get system modules
   SystemEnv = {List.toRecord env
		{Map Modules
		 fun {$ ModName}
		    ModName#{ModMan link(name:ModName $)}
		 end}}

   %% Provide shortcuts
   Env = {FoldL ShortCuts
	  fun {$ Env SC}
	     Module = {ModMan link(name:{Label SC} $)}
	     ExtraEnv = {Record.map SC
			 fun lazy {$ Fs}
			    {Dots Module Fs}
			 end}
	  in
	     {Adjoin Env ExtraEnv}
	  end SystemEnv}
end
