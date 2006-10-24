declare FD_PROP 
local 
   FD_PROP_O = {{New Module.manager init}
		link(url: 'ex_a.so{native}' $)}
in
   FD_PROP = fd(add: FD_PROP_O.add)
   {Browse FD_PROP}
end 
