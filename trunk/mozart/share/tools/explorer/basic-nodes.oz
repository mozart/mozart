%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   fun {AddLast Xs Y}
      case Xs of nil then [Y]
      [] X|Xr then X|{AddLast Xr Y}
      end
   end

   fun {Replace Xs Y Z}
      case Xs of nil then nil
      [] X|Xr then case X==Y then Z|Xr else X|{Replace Xr Y Z} end
      end
   end
   
   class Node from UrObject
      feat
	 mom           % The mom of this node (False if topmost node) 
   end
   
   class Inner from Node
      attr
	 isDirty:    True  % No layout computed
	 kids:       nil   % The list of nodes below
	 toDo:       nil   % What is to be done (nil if nothing)
	 isSolBelow: False % Is there a solution below
	 choices:    1     % unfinished choices below?

      meth addKid(K)
	 kids <- {AddLast @kids K}
      end
      
      meth replaceKid(K KN)
	 kids <- {Replace @kids K KN}
      end
      
      meth getKids($)
	 @kids
      end
   end

in

   BasicNodes = c(inner: Inner
		  leaf:  Node)
		
end



