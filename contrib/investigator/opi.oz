
\switch +dynamicvarnames
\switch +controlflowinfo
\switch +profile

{Property.put 'internal.propLocation' true}


declare
local
   [V] = {Module.link ['x-oz://contrib/Investigator.ozf']}
in
   BrowserPluginActivate  = V.browserPluginActivate
   ExplorerPluginActivate = V.explorerPluginActivate
   InvestigateConstraints = V.investigateConstraints
   InvestigateProcedureGraph = V.investigateProcedureGraph
end
{ExplorerPluginActivate}
{BrowserPluginActivate}

declare [R] = {Module.link ['x-oz://contrib/Reflect.ozf']}

{Wait FD}
{Wait Space}
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

{InvestigateConstraints [N11 N12] }

{Problem} = {InvestigateConstraints}

{Explorer.object script(Problem)}

{Browse N11}

{Browse {R.spaceReflect N11}.varsTable.1}
{Browse {R.spaceReflect N11}.propTable.1}
{Browse {R.spaceReflect N11}.procTable}

{Browse R} {Wait R}

declare
ReflectTables = {R.spaceReflect S}


{Browse ReflectTables}

% propagators failed during propagation
declare X Y Z in [X Y Z] ::: 1#10
X <: Y
Y <: Z
Z <: X

{Browse {R.spaceReflect X}}

{InvestigateConstraints X}

% propagators failed during propagation
declare A B C D in
A :: 6#9
[B C] ::: 1#10
D :: 2#5
A <: B
C <: D
B <: C

{Browse {R.spaceReflect X}}

{InvestigateConstraints A}


\insert ~/Programming/Oz/coins.oz
declare S in {Coins S}

declare
Tables = {R.spaceReflect S}

{Browse Tables.procTable}

{Browse Tables.propTable}

\insert ~/Programming/Oz/FirstClassPropagators/hamil.oz
% 2
declare H = [
	     arc(0 [1])
	     arc(1 [0]) 
	    ]

declare H = [
	     arc(0 [1])
	     arc(1 [0 2]) 
	     arc(2 [1 3 4 5]) 
	     arc(3 [2 4]) 
	     arc(4 [2 3 6]) 
	     arc(5 [2 6 7 8]) 
	     arc(6 [4 5 7]) 
	     arc(7 [5 6 8]) 
	     arc(8 [5 9 7]) 
	     arc(9 [8])
	    ]

declare S = {{Hamilton H}}

{InvestigateProcedureGraph S}


declare
Tables = {R.spaceReflect S}


{Browse Tables.procTable}




declare
Tables = {R.spaceReflect S}

{Browse Tables}

% propagators failed during propagation

declare A B C D in
[A B C D] ::: 1#10
A <: B
C <: D
B <: C
D <: A

{Browse [A B C D]}

{Browse {R.spaceReflect A}}

{InvestigateConstraints [A B C D]}


declare A B C D in
[A B C  D] ::: 1#10
A + C =: D
C \=: D
A + B =: C
{FD.distinct [A B C D]}

a(x^g, w^g, u^g)
b(a^h,w^h,v^h, u^g)
c(x^h,w^h)
d(w^h,v^h,u^h)

{Browse {R.spaceReflect [A B C D]}}

{InvestigateConstraints [A B C D]}

{Browse {R.spaceReflect A}}

declare
RS =reflect_space(
       failedProp:unit 
       propTable:reflect_proptable(
		    propagator(
		       connected_props:{FS.value.make 2#4} 
		       id:1 
		       location:unit
		       name:'   a   ' 
		       parameters:{FS.value.make [1 3#4]} 
		       reference:unit)
		    propagator(
		       connected_props:{FS.value.make [1 3#4]} 
		       id:2 
		       location:unit
		       name:'   b   '
		       parameters:{FS.value.make 1#4} 
		       reference:unit)
		    propagator(
		       connected_props:{FS.value.make [1#2 4]} 
		       id:3 
		       location:unit
		       name:'   c   '
		       parameters:{FS.value.make 3#4} 
		       reference:unit)
		    propagator(
		       connected_props:{FS.value.make 1#3} 
		       id:4 
		       location:unit
		       name:'   d   '
		       parameters:{FS.value.make 1#3} 
		       reference:unit)
		    )
       varsTable:reflect_vartable(
		    var(
		       connected_vars:{FS.value.make 2#4} 
		       id:1 
		       name:u
		       nameconstraint:u
		       propagators:{FS.value.make [1#2 4]} 
		       reference:unit
		       susplists:susplists(g:{FS.value.make [1 2 4]}
					   /*h:{FS.value.make 2}*/) 
		       type:fd) 
		    var(
		       connected_vars:{FS.value.make [1 3#4]} 
		       id:2 
		       name:v
		       nameconstraint:v
		       propagators:{FS.value.make [2 4]} 
		       reference:unit
		       susplists:susplists(g:{FS.value.make 4}
					   h:{FS.value.make 2}) 
		       type:fd) 
		    var(
		       connected_vars:{FS.value.make [1#2 4]} 
		       id:3 
		       name:w
		       nameconstraint:w
		       propagators:{FS.value.make 1#4} 
		       reference:unit
		       susplists:susplists(g:{FS.value.make [1 4]}
					   h:{FS.value.make 2#3}) 
		       type:fd) 
		    var(
		       connected_vars:{FS.value.make 1#3} 
		       id:4 
		       name:x
		       nameconstraint:x
		       propagators:{FS.value.make 1#3} 
		       reference:unit
		       susplists:susplists(g:{FS.value.make 1}
					   h:{FS.value.make 2#3}) 
		       type:fd)
		    )
       )
{InvestigateConstraints RS}

*/
/*
Denys Bug:

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

declare
[M] = {Module.link ['~duchier/Coli/DG/Tobias.ozf']}
proc {E L} {ExploreAll {M.solutionPredicate L}} end

declare L = {{M.solutionPredicate [zu lesen hat er mir das buch versprochen]}}
{Browse {R.spaceReflect L}}

{Show L}
{InvestigateConstraints L}




declare X = {FD.decl} Y = {FD.decl}
%X <: Y
{InvestigateConstraints X}

declare L [B1 B2 T1 T2] = L
L ::: 0#10
B1 =: (T1 + 2 =<: T2) % implemented by FD.reified.sumC
B2 =: (T2 + 3 =<: T1) % implemented by FD.reified.sumC
B1 + B2 =: 1          % implemented by FD.sumC

{InvestigateConstraints L}

{Browse L}
*/
