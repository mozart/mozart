%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

[add(information proc {$ N X} {Show N#X} end
     label: 'Show')
 add(information proc {$ N X} {Browse N#X} end
     label: 'Browse')

 add(compare proc {$ N1 X1 N2 X2} {Show N1#N2#X1#X2} end
     label: 'Show')
 add(compare proc {$ N1 X1 N2 X2} {Browse N1#N2#X1#X2} end
     label: 'Browse')

 add(statistics proc {$ N S}
		   {Show N#{Record.subtract S shape}}
		end
     label: 'Show')
 add(statistics proc {$ N S}
		   {Browse N#{Record.subtract S shape}}
		end
     label: 'Browse')]
