%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$


declare

Explorer

ExploreOne ExploreAll ExploreBest

in

local

   \insert 'explorer/main.oz'

in

   Explorer = {New ExplorerClass noop}

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
