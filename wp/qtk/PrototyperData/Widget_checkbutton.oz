local
   C R
   Desc=checkbutton(text:"Checkbutton"
		    init:true
		    handle:C
		    return:R
		    action:proc{$} {Show {C get($)}} end)
in
   {{QTk.build td(Desc)} show}
   {Delay 1000}
   {C set({C get($)}==false)}
   {Wait R}
   {Show R}
end
