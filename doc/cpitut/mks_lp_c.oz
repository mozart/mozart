declare
proc {DistributeKnapSackLP Vs ObjFn Constraints MaxProfit}
   choice
      DupVs = {DuplicateRIs Vs}
      DupMaxProfit V DupV
   in
      DupMaxProfit = {RI.var.bounds
                      {RI.getLowerBound MaxProfit}
                      {RI.getUpperBound MaxProfit}}

      {LP.solve DupVs ObjFn Constraints DupMaxProfit optimal}

      V#DupV = {SelectVar Vs#DupVs}

      case {IsDet V} then
         DupMaxProfit = MaxProfit
         DupVs        = Vs
      else
         dis {RI.lessEq {Ceil DupV} V} then
            {DistributeKnapSackLP Vs ObjFn Constraints MaxProfit}
         [] {RI.lessEq V {Floor DupV}} then
            {DistributeKnapSackLP Vs ObjFn Constraints MaxProfit}
         end
      end
   end
end
