%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$


declare
   Explorer ExplorerClass
   ExploreOne ExploreAll ExploreBest
\ifdef SAVE
   NewExplorer
\endif
in

\ifdef SAVE
fun {NewExplorer Tk TkTools Browse}
\endif
   
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

\ifdef SAVE
in
   explorer('ExplorerClass': ExplorerClass
	    'Explorer':      Explorer
	    'ExploreOne':    ExploreOne
	    'ExploreAll':    ExploreAll
	    'ExploreBest':   ExploreBest)
end


explorer('ExplorerClass': ExplorerClass
	 'Explorer':      Explorer
	 'ExploreOne':    ExploreOne
	 'ExploreAll':    ExploreAll
	 'ExploreBest':   ExploreBest)
= {NewExplorer Tk TkTools Browse}
\endif

