local
   Check Radio
   Menu=menu(command(text:"Command"
		     action:proc{$} {Show command} end)
	     separator
	     checkbutton(text:"Checkbutton"
			 action:proc{$} {Show checkbutton} end
			 init:false
			 return:Check)
	     radiobutton(text:"Radiobutton 1"
			 action:proc{$} {Show radiobutton} end
			 group:radiogroup
			 init:true
			 return:Radio)
	     radiobutton(text:"Radiobutton 2"
			 action:proc{$} {Show radiobutton} end
			 group:radiogroup
			 init:false)
	     cascade(text:"Cascade"
		     action:proc{$} {Show cascade} end
		     menu:menu(tearoff:false
			       radiobutton(text:"Radiobutton 3"
					   action:proc{$} {Show radiobutton} end
					   group:radiogroup))))
   Desc=td(menubutton(glue:nw
		      text:"File"
		      menu:Menu))
in
   {{QTk.build Desc} show}
end

		   
