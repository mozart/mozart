declare
fun {KnapsackFD Problem}
   NumProducts = {Length Problem.profit}
   Resources   = Problem.resources
in
   proc {$ Sol}
      sol(maxprofit: MaxProfit = {FD.decl}
	  products: Products = {FD.list NumProducts 0#FD.sup})
      = Sol
   in
      MaxProfit = {FD.sumC Problem.profit Products '=:'}

      {ForAll {Arity Resources}
       proc {$ ResourceName}
	  Resource = Resources.ResourceName
       in
	  {FD.sumC Resource.npp Products '=<:' Resource.ta}
       end}

      {FD.distribute naive Products}
   end
end
