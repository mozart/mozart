local
   P I1 I2 I3
   Desc=placeholder(glue:nswe
		    td(handle:I1              % initial place created on the fly
		       label(text:"Hello"))
		    handle:P)
in
   {{QTk.build td(Desc)} show}
   {P set(td(handle:I2 glue:nw
	     label(text:"World")))}           % second place
   {P set(td(handle:I3 glue:se
	     button(text:"Close"
		    action:toplevel#close)))} % third place
   {P set(I1)}                                % places back then hello widget
   {Delay 2000}
   {P set(empty)}
   {Delay 1000}
   {P set(I2)}
   {Delay 2000}
   {P set(I3)}
end

   
