%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

declare

fun
\ifdef NEWCOMPILER
   instantiate
\endif
   {NewPanel Standard Tk TkTools}
\insert 'Standard.env'
   = Standard
in
   local
      \insert 'panel/main.oz'
      Panel = {New PanelClass init}
   in
      \insert 'Panel.env'
   end
end
