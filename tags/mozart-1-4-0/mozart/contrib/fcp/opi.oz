declare [FCP_FD] = {Module.link ['x-oz://contrib/modFDP.fcp.ozf']}
declare [FCP_FS] = {Module.link ['x-oz://contrib/modFSP.fcp.ozf']}
declare [FCP_RI] = {Module.link ['x-oz://contrib/modRI.fcp.ozf']}
declare [Propagator] = {Module.link ['x-oz://contrib/Propagator.ozf']}
declare [Reflect] = {Module.link ['x-oz://contrib/Reflect.ozf']}
declare [RI] = {Module.link ['x-oz://contrib/RI.ozf']}
declare [LP] = {Module.link ['x-oz://contrib/LP.ozf']}

{Wait FCP_FD}
{Wait FCP_FS}
{Wait FCP_RI}
{Wait Propagator}
{Wait Reflect}
{Wait RI}
{Wait LP}

{Show FCP_FD}
{Show FCP_FS}
{Show FCP_RI}
{Show Propagator}
{Show Reflect}
{Show LP}
{Show RI}


declare X Y P in [X Y]:::0#10
{FCP_FD.'FD.lessEqOff' X Y 5 P}

/*


cond X Y P in
   [X Y]:::0#10
   {FCP_FD.'FD.lessEqOff' X Y 5 P}
   {Propagator.discard P}
   {Show show(P)}
then {Show show(yes)}
else {Show show(no)}
end

*/
{Show show(P)}

/*

{Show show({Propagator.getName P})}
declare Ps = {Propagator.getParameter P}{Show show(Ps)}
{Show show({Propagator.isDiscarded P})}
{Show show({Propagator.is P})}
{Propagator.discard P}
{Show show({Propagator.isDiscarded P})}

declare
X Y Is
in
{FS.var.list.upperBound 2 [1 2 3]  [X Y]}
Is = {Propagator.identifyParameter [X Y 1 X 2 y Y]}
{Show show(Is)}
{Show show([X Y])}

declare
X Y Is
in
[X Y] ::: 1#100
Is = {Propagator.identifyParameter [X Y 1 X 2 y Y]}
{Show show(Is)}
{Show show([X Y])}

declare
X Y Is
in
[X Y] :::  0#1
Is = {Propagator.identifyParameter [X Y 1 X 2 y Y]}
{Show show(Is)}
{Show show([X Y])}

declare
X Y Is
in
{ForAll [X Y] proc {$ V} {RI.var.bounds ~1.1 1.1 V} end}
Is = {Propagator.identifyParameter [X Y 1 X 2 y Y]}
{Show show(Is)}
{Show show([X Y])}

{Browse Propagator}

declare X Y P B in [X Y]:::0#10 B::0#1
{FCP_FD.'FD.reified.sumC' [1 1] [X Y] '=<:' 5 B P}


{Show show({Propagator.getName P})}
B = 1
{Show show({Propagator.getName P})}

{Show show({Propagator.isActive P})}
{Show show({Propagator.isDiscarded P})}


{Propagator.activate P}
{Propagator.deactivate P}
{Show [X Y B P]}

{Browse FCP_FD}
{Browse Reflect}

declare A B C P in [A B] ::: 1#2 C :: 0#5
{Browse [A B C]}
{FCP_FD.'Plus.inactive' A B C P}

{Propagator.activate P}

{Browse FCP_FD}
{Browse FCP_FS}
{Browse FCP_RI}

*/
