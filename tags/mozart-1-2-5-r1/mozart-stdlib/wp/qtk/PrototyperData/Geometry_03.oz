local
   Desc=lr(label(text:"1"
		 bg:blue)
	   label(text:"2"
		 bg:red
		 glue:ns)
	   label(text:"3"
		 bg:green
		 glue:we)
	   label(text:"4"
		 bg:yellow
		 glue:nswe))
in
   {{QTk.build Desc} show}
end

	   
