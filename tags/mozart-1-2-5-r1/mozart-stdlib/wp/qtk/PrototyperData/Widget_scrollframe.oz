local
   Frame=td(label(text:"Label" width:10 height:10)
	    button(text:"Hello" action:QTk.bell)
	    button(text:"Close" action:toplevel#close))
   Desc=scrollframe(glue:nswe
		    Frame
		    tdscrollbar:true
		    lrscrollbar:true)
in
   {{QTk.build td(Desc)} show}
end
