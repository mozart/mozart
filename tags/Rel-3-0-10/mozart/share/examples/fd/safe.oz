% Professor Smart has coded his safe combination as follows:

declare
proc {Safe C}
   {FD.tuple code 9 1#9 C}
   {FD.distinct C}
   {For 1 9 1 proc {$ I} C.I \=: I end}
   C.4 - C.6 =: C.7
   C.1 * C.2 * C.3 =: C.8 + C.9
   C.2 + C.3 + C.6 <: C.8
   C.9 <: C.8
   {FD.distribute ff C}
end

{ExploreAll Safe}
