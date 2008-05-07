local
   S R
   Desc=lrscale('from':0 to:100
		init:10
		handle:S
		action:proc{$} {Show {S get($)}} end
		return:R
	       )
in
   {{QTk.build td(Desc)} show}
   {Wait R}
   {Show R}
end
