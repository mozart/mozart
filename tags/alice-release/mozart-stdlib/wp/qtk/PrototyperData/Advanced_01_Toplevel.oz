local
   Desc1=td(action:proc{$} skip end
	    title:"show"
	    button(text:"Close show" action:toplevel#close))
   Desc2=td(action:proc{$} skip end
	    title:"show(wait:true modal:true)"
	    button(text:"Close modal" action:toplevel#close))
   Desc3=td(action:proc{$} skip end
	    title:"show(wait:true)"
	    button(text:"Close wait" action:toplevel#close))
in
   {{QTk.build Desc1} show}   %% default : wait:false modal:false
   {{QTk.build Desc2} show(wait:true modal:true)}
   {{QTk.build Desc3} show(wait:true)}
end
