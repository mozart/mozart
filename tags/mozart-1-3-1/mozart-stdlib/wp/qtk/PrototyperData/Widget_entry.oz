local
   E R
   Desc=entry(init:"Type here"
	      handle:E
	      return:R
	      action:proc{$} {Show {String.toAtom {E get($)}}} end)
in
   {{QTk.build td(Desc)} show}
   {Wait R}
   {Show {String.toAtom R}}
end
