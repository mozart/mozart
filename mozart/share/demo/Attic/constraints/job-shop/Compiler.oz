%%%
%%% Authors:
%%%   Gert Smolka (smolka@dfki.de)
%%%   Joerg Wuertz (wuertz@dfki.de)
%%%
%%% Modified:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
%%%   Gert Smolka, 1997
%%%   Joerg Wuertz, 1997
%%%   Christian Schulte, 1997, 1998
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

functor

import
   FD
   Schedule

export
   Dumb Smart
   
define

   fun {MakeCompiler IsDumb}
      
      fun {$ TaskSpecs}
	 MaxTime =
	 {FoldL TaskSpecs fun {$ In _#D#_#_} D+In end 0}
	 
	 Tasks =
	 {Map TaskSpecs fun {$ T#_#_#_} T end}
	 
	 Dur =    % task --> duration 
	 {MakeRecord dur Tasks}
	 {ForAll TaskSpecs proc {$ T#D#_#_} Dur.T = D end}
	 
	 Resources =
	 {FoldL TaskSpecs
	  fun {$ In _#_#_#Resource}
	     if Resource==noResource orelse {Member Resource In} then In
	     else Resource|In
	     end
	  end nil}
	 
	 ExclusiveTasks =  % list of lists of exclusive tasks
	 {FoldR Resources
	  fun {$ Resource Xs}
	     {FoldR TaskSpecs
	      fun {$ Task#_#_#ThisResource In}
		 if Resource==ThisResource then Task|In else In end
	      end
	      nil} | Xs
	  end
	  nil}
	 
      in
	 
	 proc {$ Start}
	    Start =       % task --> start time
	    {FD.record start Tasks 0#MaxTime}
	    
	    %% impose precedences
	    
	    {ForAll TaskSpecs
	     proc {$ Task#_#Preds#_}
		{ForAll Preds
		 proc {$ Pred}
		    Start.Pred + Dur.Pred =<: Start.Task
		 end}
	     end}
	    
	    %% impose resource constraints
	    
	    {Schedule.serialized ExclusiveTasks Start Dur}
	    
	    %% distribute exclusion choices
	    
	    if IsDumb then
	       {Schedule.lastsDist ExclusiveTasks Start Dur}
	    else
	       {Schedule.firstsLastsDist ExclusiveTasks Start Dur}
	    end

	    %% fix all start points to minimum after distribution
	    {Record.forAll Start proc {$ S}
				    S={FD.reflect.min S}
				 end}
	    
	 end
	 
      end

   end

   Dumb  = {MakeCompiler true}

   Smart = {MakeCompiler false}
   
end





