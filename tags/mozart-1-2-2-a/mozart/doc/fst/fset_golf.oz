declare
fun {Golf NbOfWeeks NbOfFourSomes}
   NbOfPlayers = 4*NbOfFourSomes
   
   fun {Flatten Ls}
      {FoldL Ls fun {$ L R}
		   if R==nil then L
		   else {Append L R} end
		end nil}
   end
   
   proc {DistrPlayers AllWeeks Player Weeks}
      choice
	 case Weeks
	 of FourSome|Rest then
	    dis {FS.include Player FourSome} then
	       {DistrPlayers AllWeeks Player Rest}
	    [] {FS.exclude Player FourSome} then
	       {DistrPlayers AllWeeks Player Rest}
	    end
	 else
	    if Player < NbOfPlayers then
	       {DistrPlayers AllWeeks Player+1 AllWeeks}
	    else skip end
	 end
      end
   end
in
   proc {$ Weeks}
      FlattenedWeeks
   in
      Weeks = {MakeList NbOfWeeks}
      
      {ForAll Weeks
       proc {$ Week}
	  Week =
	  {FS.var.list.upperBound
	   NbOfFourSomes [1#NbOfPlayers]}
	  {ForAll Week proc {$ FourSome}
			  {FS.card FourSome 4}
		       end}
	  {FS.partition Week
	   {FS.value.make [1#NbOfPlayers]}}
       end}
      
      {ForAllTail Weeks
       proc {$ WTails}
	  case WTails
	  of Week|RestWeeks then
	     {ForAll Week
	      proc {$ FourSome}
		 {ForAll {Flatten RestWeeks}
		  proc {$ RestFourSome}
		     {FS.cardRange 0 1
		      {FS.intersect
		       FourSome RestFourSome}}
		  end}
	      end}
	  else skip end
       end}
      
      FlattenedWeeks = {Flatten Weeks}
      {DistrPlayers FlattenedWeeks 1 FlattenedWeeks}
   end
end

