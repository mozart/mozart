%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   class ChoiceNode

      meth mark
	 case @copy of !False then true elseof Copy then
	    case {Label Copy}
	    of transient then {self.canvas tk(itemconfigure
					      self.node o(fill:gray))}
	    [] persistent then {self.canvas tk(itemconfigure
					       self.node o(fill:black))}
	    end
	 end
	 {ForAll @kids proc {$ K} {K mark} end}
      end

   end

   class SolvedNode
      meth mark true end
   end
	 
   class FailedNode
      meth mark true end
   end
	 
   class UnstableNode
      meth mark true end
   end

in

   DebugNodes = nodes(choice: ChoiceNode
		      failed: FailedNode
		      unstable: UnstableNode
		      solved: SolvedNode)

end
