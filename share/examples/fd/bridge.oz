declare

proc {Bridge TaskSpecs Constraints}
   TaskSpecs = [% task # duration # preceding tasks # resources
		pa # 0  # nil  # noResource
		a1 # 4  # [pa] # excavator
		a2 # 2  # [pa] # excavator
		a3 # 2  # [pa] # excavator
		a4 # 2  # [pa] # excavator
		a5 # 2  # [pa] # excavator
		a6 # 5  # [pa] # excavator
		p1 # 20 # [a3] # pileDriver
		p2 # 13 # [a4] # pileDriver
		ue # 10 # [pa] # noResource
		s1 # 8  # [a1] # carpentry
		s2 # 4  # [a2] # carpentry
		s3 # 4  # [p1] # carpentry
		s4 # 4  # [p2] # carpentry
		s5 # 4  # [a5] # carpentry
		s6 # 10 # [a6] # carpentry
		b1 # 1  # [s1] # concreteMixer
		b2 # 1  # [s2] # concreteMixer
		b3 # 1  # [s3] # concreteMixer
		b4 # 1  # [s4] # concreteMixer
		b5 # 1  # [s5] # concreteMixer
		b6 # 1  # [s6] # concreteMixer 
		ab1 # 1 # [b1] # noResource
		ab2 # 1 # [b2] # noResource
		ab3 # 1 # [b3] # noResource
		ab4 # 1 # [b4] # noResource
		ab5 # 1 # [b5] # noResource
		ab6 # 1 # [b6] # noResource
		m1 # 16 # [ab1]# bricklaying
		m2 # 8 # [ab2] # bricklaying
		m3 # 8 # [ab3] # bricklaying
		m4 # 8 # [ab4] # bricklaying
		m5 # 8 # [ab5] # bricklaying
		m6 # 20 # [ab6]# bricklaying
		l  # 2  # nil  # crane
		t1 # 12 # [m1 m2 l] # crane
		t2 # 12 # [m2 m3 l] # crane
		t3 # 12 # [m3 m4 l] # crane
		t4 # 12 # [m4 m5 l] # crane
		t5 # 12 # [m5 m6 l] # crane
		ua # 10 # nil # noResource
		v1 # 15 # [t1] # caterpillar
		v2 # 10 # [t5] # caterpillar
		pe # 0 # [t2 t3 t4 v1 v2 ua] # noResource
	       ]

   proc {Constraints Start Dur}
      {ForAll [s1#b1 s2#b2 s3#b3 s4#b4 s5#b5 s6#b6]
       proc {$ A#B}
	  (Start.B + Dur.B) - (Start.A + Dur.A) =<: 4
       end}
      
      {ForAll [a1#s1 a2#s2 a5#s5 a6#s6 p1#s3 p2#s4]
       proc{$ A#B}
	  Start.B - (Start.A + Dur.A) =<: 3
       end}
      
      {ForAll [s1 s2 s3 s4 s5 s6]
       proc{$ A}
	  Start. A >=: Start.ue + 6
       end}

      {ForAll [m1 m2 m3 m4 m5 m6]
       proc{$ A}
	  (Start.A + Dur.A) - 2 =<: Start.ua
       end}

      Start.l =: Start.pa + 30
      Start.pa = 0
   end
end

%%%%%%%%%%%%%%
%  Compiler  %
%%%%%%%%%%%%%%
	       
fun {Compile Specification}
   TaskSpecs
   Constraints
      {Specification TaskSpecs Constraints}
   MaxTime =
      {FoldR TaskSpecs fun {$ _#D#_#_ A} D+A end 0}
   Tasks =
      {FoldR TaskSpecs fun {$ T#_#_#_ A} T|A end nil}
   Dur =    % task --> duration 
      {MakeRecord dur Tasks}
      {ForAll TaskSpecs proc {$ T#D#_#_} Dur.T = D end}
   Resources =
       {FoldR TaskSpecs
	fun {$ _#_#_#Resource A}
	   case Resource==noResource orelse {Member Resource A}
	   then A else Resource|A end
	end
	nil}
   ExclusiveTasks =  % list of lists of exclusive tasks
      {Map Resources
       fun {$ Resource}
	  {FoldR TaskSpecs
	   fun {$ Task#_#_#ThisResource A}
	      case Resource==ThisResource then Task|A else A end
	   end
	   nil}
       end}
in
   proc {$ Start}
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
      % impose other constraints
      {Constraints Start Dur}
      % impose resource constraints
      {FD.schedule.serialized ExclusiveTasks Start Dur}
      % serialize and commit
      {FD.schedule.firstsLastsDist ExclusiveTasks Start Dur}
      choice
	 {Record.forAll Start proc {$ S} S = {FD.reflect.min S} end}
      end
   end
end



{ExploreBest {Compile Bridge}
 proc {$ Old New} Old.pe >: New.pe end}


/*
{SearchBest {Compile Bridge}
 proc {$ Old New} Old.pe >: New.pe end _}

{{Compile Bridge} _}
*/
