%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
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

   \insert 'explorer/configure-static.oz'

   \insert 'explorer/misc.oz'

   \insert 'explorer/combine-nodes.oz'

in

   functor

   import
      Property(get)

      System(show
	     printName)

      ErrorRegistry(put)

      Tk

      TkTools

      Browser(browse)

   export
      'class':  ExplorerClass
      'object': Explorer

      'one':    ExploreOne
      'all':    ExploreAll
      'best':   ExploreBest

      'close':  CloseExplorer

   define

      \insert 'explorer/errors.oz'
      \insert 'explorer/main.oz'

      Explorer = {New ExplorerClass init}

      proc {ExploreOne P}
	 {Explorer one(P)}
      end

      proc {ExploreAll P}
	 {Explorer all(P)}
      end

      proc {ExploreBest P O}
	 {Explorer all(P O)}
      end

      proc {CloseExplorer}
	 {Explorer close}
      end

   end

end
