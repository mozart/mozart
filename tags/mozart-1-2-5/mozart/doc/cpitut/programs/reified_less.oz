declare FD_PROP in

local
   FD_PROP_O = {{New Module.manager init}
		link(url: 'reified_less.so{native}' $)}
in
   FD_PROP = fd(init: FD_PROP_O.init
		lessEq: FD_PROP_O.lessEq
		greater: FD_PROP_O.greater
		reifiedLessEq: FD_PROP_O.reifiedLessEq)
   
   {FD_PROP.init}
end


/*

declare X Y R in {Browse [X Y R]}
[X Y] ::: 0#10
R :: 0#1
{FD_PROP.reifiedLessEq X Y R}

%%%%%%%%%%%%%%%%%%%%
R = 1
X >: 3
Y <: 6
X=4
%%%%%%%%%%%%%%%%%%%%
R = 0
X <: 6
Y >: 2
Y = 4
%%%%%%%%%%%%%%%%%%%%
X <: 2
Y >: 3
%%%%%%%%%%%%%%%%%%%%
X >: 2
Y <: 3
%%%%%%%%%%%%%%%%%%%%
X <: 6
Y >: 3
Y = 4
X = 2
%%%%%%%%%%%%%%%%%%%%
X <: 6
Y >: 6
*/

