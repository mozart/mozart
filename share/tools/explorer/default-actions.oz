%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

[add(information:
	proc {$ N S}
	   {Show N#{Space.merge S}}
	end
     label: 'Show')
 add(information:
	proc {$ N S}
	   {Browse N#{Space.merge S}}
	end
     label: 'Browse')
 add(information:separator)

 add(compare:
	proc {$ N1 S1 N2 S2}
	   {Show N1#N2#{Space.merge S1}#{Space.merge S2}}
	end
     label: 'Show')
 add(compare:
	proc {$ N1 S1 N2 S2}
	   {Browse N1#N2#{Space.merge S1}#{Space.merge S2}}
	end
     label: 'Browse')
 add(compare:separator)

 add(statistics:
	proc {$ N S}
	   {Show N#{Record.subtract S shape}}
	end
     label: 'Show')
 add(statistics:
	proc {$ N S}
	   {Browse N#{Record.subtract S shape}}
	end
     label: 'Browse')
 add(statistics:separator)]
