declare FD_PROP in

local 
   FD_PROP_O = {{New Module.manager init}
		link(url:'ex_c.so{native}' $)}
in
   FD_PROP = fd(init: FD_PROP_O.init
		add: FD_PROP_O.add
		add_nestable: FD_PROP_O.add_nestable
		twice: FD_PROP_O.twice
		element: FD_PROP_O.element)

   {Browse FD_PROP}
   
   {FD_PROP.init}
end


