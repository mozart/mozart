local
   Desc=td(button(text:"{Show 'Hello World'}"
		  action:proc{$} {Show 'Hello World'} end)
	   button(text:"Close"
		  action:toplevel#close))
in
   {{QTk.build Desc} show}
end
