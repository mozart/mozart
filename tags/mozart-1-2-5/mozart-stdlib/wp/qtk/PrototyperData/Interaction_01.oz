local
   L
   Desc=td(label(handle:L
		 feature:label
		 text:"Hello"
		 bg:white))
   Window={QTk.build Desc}
in
   {Window show}
   {Show L==Window.label}
   {L set(bg:blue)}
   {Show {L get(bg:$)}}
end
