declare FD_PROP
local
   FD_PROP_O = {{New Module.manager init}
                link(url:
                {OS.getEnv 'HOME'}#'/mozart/doc/cpi/programs/ex_a.so{native}' $)}
in
   FD_PROP = fd(init: FD_PROP_O.init add: FD_PROP_O.add)
   {Browse FD_PROP}
   {FD_PROP.init}
end
