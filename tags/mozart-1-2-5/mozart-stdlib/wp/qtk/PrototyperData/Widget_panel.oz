local
   P
   Desc=panel(glue:nswe
	      td(title:"Panel 1"
		 label(text:"This is the first panel"))
	      td(title:"Panel 2"
		 label(text:"This is the second panel"))
	      handle:P)
in
   {{QTk.build td(Desc)} show}
   {P addPanel(td(title:"Panel 3"
		  label(text:"This is the third panel")))}
end
   
