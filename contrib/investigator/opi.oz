\switch +dynamicvarnames
\switch +controlflowinfo

{Property.put 'internal.propLocation' true}

{Show {Property.get 'internal.propLocation'}}


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

declare [R] = {Module.link ['x-oz://contrib/Reflect.ozf']}
/*


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

{Browse {R.spaceReflect N11}}

declare
ReflectTables = {R.spaceReflect S}


{Browse ReflectTables}

% propagators failed during propagation
declare X Y Z in [X Y Z] ::: 1#10
X <: Y
Y <: Z
Z <: X

{InvestigateConstraints X}

% propagators failed during propagation
declare A B C D in
A :: 6#9
[B C] ::: 1#10
D :: 2#5
A <: B
C <: D
B <: C

{InvestigateConstraints A}


\insert ~/Programming/Oz/coins.oz
declare S in {Coins S}

declare
Tables = {R.spaceReflect S}

% propagators failed during propagation
declare A B C D in
[A B C  D] ::: 1#10
A <: B
C <: D
B <: C

{InvestigateConstraints A}

{Browse {R.spaceReflect A}}

*/
