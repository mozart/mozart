declare
fun {Hamming Bits Distance NumSymbols}      
   proc {MinDist X Y}      
      Common1s = {FS.intersect X Y}
      Common0s = {FS.complIn
		  {FS.union X Y}
		  {FS.value.make [1#Bits]}}
   in
      Bits-{FS.card Common1s}-{FS.card Common0s}>=:Distance
   end
in
   proc {$ Xs}
      Xs = {FS.var.list.upperBound NumSymbols [1#Bits]}
      
      {ForAllTail Xs proc {$ X|Y}
			{ForAll Y proc {$ Z}
				     {MinDist X Z}
				  end}
		     end}
      
      {FS.distribute naive Xs}
   end
end
