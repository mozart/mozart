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

ExplorerClass,add(information proc {$ N X} {Show N#X} end
		  label: 'Show')

ExplorerClass,add(information proc {$ N X} {Browse N#X} end
		  label: 'Browse')

ExplorerClass,add(compare proc {$ N1 X1 N2 X2} {Show N1#N2#X1#X2} end
		  label: 'Show')

ExplorerClass,add(compare proc {$ N1 X1 N2 X2} {Browse N1#N2#X1#X2} end
		  label: 'Browse')

ExplorerClass,add(statistics proc {$ N S}
				{Show N#{Record.subtract S shape}}
			     end
		  label: 'Show')

ExplorerClass,add(statistics proc {$ N S}
				{Browse N#{Record.subtract S shape}}
			     end
		  label: 'Browse')
