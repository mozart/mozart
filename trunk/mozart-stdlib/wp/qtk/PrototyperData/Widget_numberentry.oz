local
   N R
   Desc=numberentry(min:0 max:100
		    init:10
		    handle:N
		    return:R
		    action:proc{$} {Show {N get($)}} end)
in
   {{QTk.build td(Desc)} show}
   {Wait R}
   {Show R}
end
