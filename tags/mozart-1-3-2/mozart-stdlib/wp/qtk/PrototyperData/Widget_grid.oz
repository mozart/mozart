local
   Grid
   Desc=grid(label(text:"1") label(text:"2") label(text:"3") newline
	     label(text:"4") empty           label(text:"6") newline
	     label(text:"7") label(text:"8") label(text:"9")
	     handle:Grid)
in
   {{QTk.build td(Desc)} show}
   {Grid configure(label(text:"5") column:2 row:2)}
   {Grid configure(label(text:"0" bg:white)
		   column:1 columnspan:3 row:4 sticky:we)}
end
