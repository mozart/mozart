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

   \insert 'explorer/configure-static.oz'

   \insert 'explorer/misc.oz'

   \insert 'explorer/combine-nodes.oz'

in

   functor $ prop once

   import
      System.{show
              get
              printName}

      Error.{formatGeneric
             format
             dispatch}

      ErrorRegistry.{put}

      Tk

      TkTools

      Browser.{browse}

   export
      'ExplorerClass': ExplorerClass
      'Explorer':      Explorer
      'ExploreOne':    ExploreOne
      'ExploreAll':    ExploreAll
      'ExploreBest':   ExploreBest

      'class':  ExplorerClass
      'object': Explorer
      'one':    ExploreOne
      'all':    ExploreAll
      'best':   ExploreBest

   body

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

   end

end
