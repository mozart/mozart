%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
%%%   Christian Schulte, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local
   fun {SendSlaves Ss M}
      case Ss of nil then nil
      [] S|Sr then NewM={Adjoin M announce(answer:_ reply:_)} in
	 {S NewM}
	 NewM|{SendSlaves Sr M}
      end
   end

   fun {FindBest As BA}
      case As of nil then BA
      [] A|Ar then
	 RA#KA = if A.answer==reject then A#BA
		 elseif BA.answer==reject then BA#A
		 elseif A.answer>BA.answer then A#BA
		 else BA#A
		 end
      in
	 RA.reply=reject {FindBest Ar KA}
      end
   end
in
   
   class Contract from BaseObject     
      feat MySlaves

      meth init
	 self.MySlaves = {Dictionary.new}
      end      
      meth add(Name Slave)
	 {Dictionary.put self.MySlaves Name Slave}
      end
      meth remove(Name)
	 Slave = {Dictionary.get self.MySlaves Name}
      in
	 {Dictionary.remove self.MySlaves Name}
	 {Slave close}
      end
      meth announce(...) = M
	 case {SendSlaves {Map {Dictionary.entries self.MySlaves}
			   fun {$ E} E.2 end} M}
	 of nil then M.answer=reject
	 [] A|Ar then M={FindBest Ar A}
	 end
      end
      meth get(Name $)
	 {Dictionary.get self.MySlaves Name}
      end
      meth close
	 {ForAll {Dictionary.entries self.MySlaves}
	  proc {$ E}
	     {E.2 close}
	  end}
      end
   end

end
