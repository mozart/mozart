declare [FCP_FD] = {Module.link ['x-oz://contrib/modFDP.fcp.ozf']}
declare [Propagator] = {Module.link ['x-oz://contrib/Propagator.ozf']}
declare [RI] = {Module.link ['x-oz://contrib/RI.ozf']}

{Wait FCP_FD}
%{Browse FCP_FD}

{Wait Propagator}
%{Browse Propagator}

declare X Y P in [X Y]:::0#10
{FCP_FD.'`LessEqOff`' X Y 5 P}

cond X Y P in
   [X Y]:::0#10
   {FCP_FD.'`LessEqOff`' X Y 5 P}
   {Propagator.discard P}
   {Show show(P)}
then {Show show(yes)}
else {Show show(no)}
end

{Show show(P)}

/*

{Show show({Propagator.getName P})}
declare Ps = {Propagator.getParameter P}{Show show(Ps)}
{Show show({Propagator.isDiscarded P})}
{Show show({Propagator.is P})}
{Propagator.discard P}

declare
X Y Is
in
{FS.var.list.upperBound 2 [1 2 3]  [X Y]}
Is = {Propagator.identifyParameter [X Y 1 X 2 y Y]}
{Show show(Is)}

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
*/
