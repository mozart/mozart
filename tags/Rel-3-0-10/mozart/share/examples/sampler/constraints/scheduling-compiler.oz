%%%  Programming Systems Lab, DFKI Saarbruecken, 
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Gert Smolka, Joerg Wuertz
%%%  Email: {smolka,wuertz}@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

declare
	       
fun {Compile Specification}
   TaskSpecs   = Specification.tasks
   Constraints = Specification.constraints

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
	   case Resource==noResource orelse {Member Resource In}
	   then In else Resource|In end
	end
	nil}
   ExclusiveTasks =  % list of lists of exclusive tasks
      {FoldR Resources
       fun {$ Resource Xs}
	  {FoldR TaskSpecs
	   fun {$ Task#_#_#ThisResource In}
	      case Resource==ThisResource then Task|In else In end
	   end
	   nil} | Xs
       end
       nil}
   SortedExclusiveTasks =  % most requested resource first
      {Sort ExclusiveTasks
       fun {$ Xs Ys}
	  fun {Aux Xs}
	     {FoldL Xs fun {$ In X} In + Dur.X end 0}
	  end
       in
	  {Aux Xs} > {Aux Ys}
       end}
   ExclusionPairs =  
      {FoldR SortedExclusiveTasks
       fun {$ Xs Ps}
	  {FoldRTail Xs
	   fun {$ Y|Ys Pss}
	      {FoldR Ys fun {$ Z Pss} Y#Z|Pss end Pss}
	   end
	   Ps}
       end
       nil}
in
   proc {$ Sol}
      Choices Start
   in 
      Sol = Start
      Start =       % task --> start time
         {FD.record start Tasks 0#MaxTime}

      % impose precedences

      {ForAll TaskSpecs
       proc {$ Task#_#Preds#_}
	  {ForAll Preds
	   proc {$ Pred}
	      Start.Pred + Dur.Pred =<: Start.Task
	   end}
       end}

      % impose Constraints
      
      {Constraints Start Dur}

      % impose resource constraints

      {FoldR ExclusionPairs
       fun {$ A#B Cs}
	  {FD.disjointC Start.A Dur.A Start.B Dur.B} | Cs
       end
       nil
       Choices}


      % enumerate exclusion choices

      {FD.distribute naive Choices}

      % fix all start points to minimum after enumeration
      {Record.forAll Start proc {$ S} S = {FD.reflect.min S} end}

   end
end 


fun {SmartCompile Specification}
   TaskSpecs   = Specification.tasks
   Constraints = Specification.constraints

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
	   case Resource==noResource orelse {Member Resource In}
	   then In else Resource|In end
	end
	nil}
   ExclusiveTasks =  % list of lists of exclusive tasks
      {FoldR Resources
       fun {$ Resource Xs}
	  {FoldR TaskSpecs
	   fun {$ Task#_#_#ThisResource In}
	      case Resource==ThisResource then Task|In else In end
	   end
	   nil} | Xs
       end
       nil}
in
   proc {$ Sol}
      Choices Start
   in 
      Sol = Start
      Start =       % task --> start time
         {FD.record start Tasks 0#MaxTime}

      % impose precedences

      {ForAll TaskSpecs
       proc {$ Task#_#Preds#_}
	  {ForAll Preds
	   proc {$ Pred}
	      Start.Pred + Dur.Pred =<: Start.Task
	   end}
       end}

      % impose Constraints
      
      {Constraints Start Dur}

      % impose resource constraints

      {FD.schedule.serialized ExclusiveTasks Start Dur}

      % enumerate exclusion choices

      {FD.schedule.firstsLastsDist ExclusiveTasks Start Dur}

      % fix all start points to minimum after enumeration
      {Record.forAll Start proc {$ S} S = {FD.reflect.min S} end}

   end
end 

