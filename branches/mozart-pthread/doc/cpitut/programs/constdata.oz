declare FD_PROP TestProb in

local 
   FD_PROP_O = {{New Module.manager init}
		link(url: 'constdata.so{native}' $)}
in
   FD_PROP = fd(init: FD_PROP_O.init add: FD_PROP_O.add)
   {FD_PROP.init}
end 

proc {TestProb Root}
   X = {FD.int [1#1000]}
   Y = {FD.decl}
in
   Root = [X Y]
   {FD_PROP.add X 1 Y}
   {FD.distribute naive [X]}
end

{Browse {SearchAll TestProb}}
{Show a}




