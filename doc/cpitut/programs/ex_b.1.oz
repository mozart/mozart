declare FD_PROP 
local 
   FD_PROP_O = {{New Module.manager init}
		link(url:'ex_b.so{native}' $)}
in
   FD_PROP = fd(init: FD_PROP_O.init
		add:  FD_PROP_O.add
		twice:FD_PROP_O.twice)
   {Browse FD_PROP}
   {FD_PROP.init}
end 

