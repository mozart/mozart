%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

declare
   PanelClass Panel
\ifdef SAVE
   NewPanel
\endif
in

\ifdef SAVE
fun {NewPanel Tk TkTools}
\endif
   
\insert panel/main.oz

   Panel = {New PanelClass init}

\ifdef SAVE
in
   panel('PanelClass': PanelClass
	 'Panel':      Panel)
end

panel('PanelClass': PanelClass
      'Panel':      Panel)
= {NewPanel Tk TkTools}
\endif



