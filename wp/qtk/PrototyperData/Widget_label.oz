local
   L R
   Desc=label(init:"This is a label widget"
	      handle:L
	      return:R
	     )
in
   {{QTk.build td(Desc)} show}
   {L set("Label widget")}
   {Wait R} % R will be binded when the window is closed
   {Show {String.toAtom R}}
end
