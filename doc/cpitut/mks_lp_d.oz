declare
fun {KnapsackLP Problem}
   NumProducts = {Length Problem.profit}
   Resources   = Problem.resources
in
   proc {$ Sol}
      sol(maxprofit: MaxProfit = {RI.var.decl}
          products:  Products  = {MakeList NumProducts}) = Sol

      ObjFn Constraints
   in
      {ForAll Products proc {$ V} {RI.var.bounds 0.0 RI.sup V} end}

      ObjFn = objfn(row: {Map Problem.profit fun {$ I} {IntToFloat I} end}
                    opt: max)

      Constraints =
      {Map {Arity Resources}
        fun {$ ResourceName}
          Resource = Resources.ResourceName
       in
          constr(row: {Map Resource.npp fun {$ I} {IntToFloat I} end}
                 type: '=<'
                 rhs: {IntToFloat Resource.ta})
       end}

      {DistributeKnapSackLP Products ObjFn Constraints MaxProfit}
   end
end
