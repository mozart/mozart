local
   M R
   Desc=message(aspect:200
		init:"This is a message widget"
		handle:M
		return:R
	       )
in
   {{QTk.build td(Desc)} show}
   {M set("Long text for a message widget")}
   {Wait R} % R will be binded when the window is closed
   {Show {String.toAtom R}}
end
