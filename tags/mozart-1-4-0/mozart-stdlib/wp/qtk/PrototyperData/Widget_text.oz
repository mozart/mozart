local
   R E
   Desc=text(tdscrollbar:true
	     lrscrollbar:true
	     init:"Hello world"
	     handle:E
	     return:R)
in
   {{QTk.build td(Desc)} show}
   {Delay 2000}
   {E set("Hi guys")}  % changes the displayed text
   {Delay 2000}
   {E insert('end' "\nNew text")} % inserts new text
   {Wait R}
   {Show {VirtualString.toAtom R}}
end
