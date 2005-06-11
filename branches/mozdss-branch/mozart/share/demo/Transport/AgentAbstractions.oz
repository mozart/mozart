%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
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
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor

export
   new:      NewAgent
   contract: Contract
      
prepare
   
   fun {NewAgent Class Mess}
      Stream
      ThisPort   = {NewPort Stream}
   in
      thread
	 ThisObject = {New Class Mess}
      in
	 try
	    {ForAll Stream ThisObject}
	 catch system(_ debug:_) then
	    %% The toplevel widget has been closed
	    {ThisObject close}
	 end
      end
      
      proc {$ Mess}
	 {Send ThisPort Mess}
      end
   end

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
      
      class Contract
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
	    case {SendSlaves {Dictionary.items self.MySlaves} M}
	    of nil then M.answer=reject
	    [] A|Ar then M={FindBest Ar A}
	    end
	 end
	 meth get(Name $)
	    {Dictionary.get self.MySlaves Name}
	 end
	 meth close
	    {ForAll {Dictionary.items self.MySlaves}
	     proc {$ E}
		{E close}
	     end}
	 end
      end
      
   end

end
