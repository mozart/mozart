local
   Y
   Desc=lr(button(text:"Yes"
		  return:Y
		  action:toplevel#close)
	   button(text:"No"
		  action:toplevel#close))
in
   {{QTk.build Desc} show}
   if Y then {Show voted_yes} else {Show voted_no} end
end
