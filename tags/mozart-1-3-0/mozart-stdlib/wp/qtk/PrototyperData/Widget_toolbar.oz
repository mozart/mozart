local
   Desc=td(
	   lr(glue:nwe
	      tbbutton(glue:w
		       text:"tbbutton" )
	      tdline(glue:wns)
	      tbcheckbutton(glue:w
			    text:"tbcheckbutton")
	      tdspace(glue:w)
	      tbradiobutton(glue:w
			    text:"tbradiobutton"
			    init:true
			    group:radio)
	      tbradiobutton(glue:w
			    text:"tbradiobutton"
			    group:radio)))
in
   {{QTk.build Desc} show}
end
