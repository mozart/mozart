
\switch +dynamicvarnames
\switch +controlflowinfo

{Property.put 'internal.propLocation' true}


declare
local
   [V] = {Module.link ['x-oz://contrib/Investigator.ozf']}
in
   BrowserPluginActivate  = V.browserPluginActivate
   ExplorerPluginActivate = V.explorerPluginActivate
   InvestigateConstraints = V.investigateConstraints
end
{ExplorerPluginActivate}
{BrowserPluginActivate}

/*

declare [M] = {Module.link ['http://www.ps.uni-sb.de/~tmueller/mozart/refl_constr.so{native}']}
{Wait M}

thread raise foo end end


declare
proc {Problem Sol}
   Square
   [N11 N12 N13 N21 N22 N23 N31 N32 N33] = Square
   Sum = {FD.decl}
in
   Square
   = {FD.dom 1#9}
   = {FD.distinct}
   Sol=s(square:Square sum:Sum)
   N11 + N12 + N13 =: Sum
   N21 + N22 + N23 =: Sum
   N31 + N32 + N33 =: Sum
   N11 + N21 + N31 =: Sum
   N12 + N22 + N32 =: Sum
   N13 + N23 + N33 =: Sum
   N11 + N22 + N33 =: Sum
   N31 + N22 + N13 =: Sum
   {FD.distribute ff Square}
end
   Sol
   Square
   [N11 N12 N13 N21 N22 N23 N31 N32 N33] = Square
   Sum = {FD.decl}
in
   Sol=s(square:Square sum:Sum)
   Square ::: 1#9
   N11 + N12 + N13 =: Sum
   N21 + N22 + N23 =: Sum
   N31 + N32 + N33 =: Sum
   N11 + N21 + N31 =: Sum
   N12 + N22 + N32 =: Sum
   N13 + N23 + N33 =: Sum
   N11 + N22 + N33 =: Sum
   N31 + N22 + N13 =: Sum
   {FD.distinct Square}

{Explorer.object script(Problem)}

{InvestigateConstraints N11}
{Browse N11}


declare X = {FD.int 1#4} in thread cond Y = {FD.int 1#3} in Y = X {Show X} then {Show a} else {Show b} end end
{Show X}

declare [R] = {Module.link ['http://www.ps.uni-sb.de/~tmueller/mozart/ReflectConstraints.ozf']}
{Wait R}

*/
