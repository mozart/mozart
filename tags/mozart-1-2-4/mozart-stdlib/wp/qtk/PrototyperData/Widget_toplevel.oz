local
   Desc=td(title:"Test window"
	   label(text:"Window" width:40))
   TopLevel
in
   TopLevel={QTk.build Desc}
   {TopLevel show}
   {Delay 2000}
   {TopLevel set(title:"TEST WINDOW")}
   {Delay 2000}
   {TopLevel set(geometry:geometry(x:200 y:200))}
end
   
