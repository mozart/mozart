declare
fun {ChangeMoney BillAndCoins Amount}
   Available    = {Record.map BillAndCoins fun {$ A#D} A end}
   Denomination = {Record.map BillAndCoins fun {$ A#D} D end}
   NbDenoms     = {Width Denomination}
in
   proc {$ Change}
      {FD.tuple change NbDenoms 0#Amount Change}
      {For 1 NbDenoms 1 proc {$ I} Change.I =<: Available.I end}
      {FD.sumC Denomination Change '=:' Amount}
      {FD.distribute generic(order:naive value:max) Change}
   end
end

BillAndCoins = r(6#100  8#25  10#10  1#5  5#1)

{ExploreOne {ChangeMoney BillAndCoins 142}}
