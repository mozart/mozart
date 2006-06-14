local
   C1 C2 C3 C4 C5
   Desc=td(radiobutton(text:"Choice 1 - group 1"
		       init:true
		       return:C1
		       group:radio1)
	   radiobutton(text:"Choice 2 - group 1"
		       return:C2
		       group:radio1)
	   radiobutton(text:"Choice 3 - group 1"
		       return:C3
		       group:radio1)
	   radiobutton(text:"Choice 1 - group 2"
		       return:C4
		       init:true
		       group:radio2)
	   radiobutton(text:"Choice 2 - group 2"
		       return:C5
		       group:radio2))
in
   {{QTk.build Desc} show}
   {Wait C5}
   {Show [C1 C2 C3 C4 C5]}
end
