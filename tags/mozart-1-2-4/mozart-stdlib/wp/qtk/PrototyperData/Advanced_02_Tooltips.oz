local
   Desc=td(button(text:"Button 1" tooltips:"You are over button 1"
		  action:toplevel#close)
	   button(text:"Button 2" tooltips:"You are over button 2"
		  action:toplevel#close))
in
   {{QTk.build Desc} show}
end
